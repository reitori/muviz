#include "datasets/include/EventReconstructor.h"

#include <iostream>

namespace{
    auto logger = logging::make_log("EventReconstructor");
}

namespace viz{
    EventReconstructor::EventReconstructor(const uint8_t& total_fes){
        clipboards.resize(total_fes);
        for(int i = 0; i < total_fes; i++){
            clipboards[i] = std::make_shared<ClipBoard<EventData>>();
        }
        leftovers.resize(total_fes);
        refBCIDs.resize(total_fes, 0);
        refL1IDs.resize(total_fes, 0);

        totalFEs = total_fes;
        triggerMultiplier = 0;
    }

    void EventReconstructor::configure(const json& config){
        if(config.contains("trigger_multiplier") && config["trigger_multiplier"].is_number_unsigned()){
            triggerMultiplier = config["trigger_multiplier"];
            logger->info("trigger_multiplier is set to {0}. Event BCIDs and L1IDs will be overriden.", triggerMultiplier);
        } 
    }

    void EventReconstructor::connectDataLoader(DataLoader* dataloader, uint8_t fe_id){
        if(fe_id >= 0 && fe_id < totalFEs)
            dataloader->connect(clipboards[fe_id]);
    }

    //Ensure that vector of DataLoaders are listed in order of associated fe_id (first element in vector corresponds to fe_id of 0)
    void EventReconstructor::connectDataLoaders(std::vector<std::unique_ptr<DataLoader>>& dataloaders){
        for(int i = 0; i < dataloaders.size(); i++){
            connectDataLoader(dataloaders[i].get(), i);
        }
    }

    std::unique_ptr<FEEvents> EventReconstructor::getEvents(){
        FEEvents tempReturn;
        tempReturn.reserve(totalFEs);
        for (size_t i = 0; i < totalFEs; ++i) {
            tempReturn.emplace_back(std::make_unique<EventData>());
        }
        FEEvents data;
        std::vector<int> readyEventSizes;

        data.resize(totalFEs);
        readyEventSizes.reserve(totalFEs);
        for(int i = 0; i < totalFEs; i++){
            data[i] = std::move(clipboards[i]->popData());
            int dataSize = (bool)(data[i]) ? data[i]->size() : 0; //Handles case where popData() returns nullptr

            readyEventSizes.push_back(dataSize + leftovers[i].size());

            //Check some extraneous cases
            if(data[i]){
                if(data[i]->has_desync_events){
                    //Do something here for desync
                }
                if(triggerMultiplier == 0 && data[i]->has_invalid_l1id){
                    logger->info("Chip {0} has invalid l1id. Overwriting bcid from here on out.", FEBookie::getName(i));
                    triggerMultiplier = 1;
                    //Could use tag instead
                }
            }
        }

        unsigned int numToSend = UINT_MAX;
        for(int i = 0; i < readyEventSizes.size(); i++){
            if(readyEventSizes[i] < numToSend)
                numToSend = readyEventSizes[i];
        }
        //Load onto internal buffer before sending and rewrite bcid and l1ids
        for(int i = 0;  i < totalFEs; i++){
            std::vector<Event> currEvents;
            if(data[i]){
                currEvents = std::move(data[i]->events);
            }
            if(leftovers[i].size() > numToSend){
                tempReturn[i]->events.insert(tempReturn[i]->events.end(), std::make_move_iterator(leftovers[i].begin()), std::make_move_iterator(leftovers[i].begin() + numToSend));
                leftovers[i].erase(leftovers[i].begin(), leftovers[i].begin() + numToSend);
             
                for(int j = 0; j < currEvents.size(); j++){
                    overwriteEvent(currEvents[j], i);
                    leftovers[i].push_back(currEvents[j]);
                }
            }
            else if(leftovers[i].size() < numToSend){
                unsigned int relFinalIndex = numToSend - leftovers[i].size();
                tempReturn[i]->events = std::move(leftovers[i]); // Fill return with all leftovers
                //Overwrite event bcid and l1id
                for(int j = 0; j < relFinalIndex; j++){ //Fill up return with all events ready to be batched
                    overwriteEvent(currEvents[j], i);
                    tempReturn[i]->events.push_back(currEvents[j]);
                }
                for(int j = relFinalIndex; j < currEvents.size(); j++){ //Fill leftovers with everything else
                    overwriteEvent(currEvents[j], i);
                    leftovers[i].push_back(currEvents[j]);
                }
            }
            else{
                tempReturn[i]->events = std::move(leftovers[i]);
                for(int j = 0; j < currEvents.size(); j++){
                    overwriteEvent(currEvents[j], i);
                    leftovers[i].push_back(currEvents[j]);
                }
            }
        }

        if(numToSend == 0)
            return nullptr;

        return std::move(std::make_unique<FEEvents>(std::move(tempReturn)));
    }  

    //Synchronize based off of internal buffer (leftovers). Index refers to nth event that is resynchronized for all front ends.
    //Events in the internal buffer coming before the index are flushed and given as the return
    std::unique_ptr<FEEvents> EventReconstructor::synchronize(uint8_t index){
        // FEEvents temp; temp.resize(totalFEs);
        // for(int i = 0; i < totalFEs; i++){
        //     //Fill up internal buffer up to necessary index
        //     std::vector<Event>& fe_leftover = fe_leftover;
        //     while(index > fe_leftover.size()){
        //         auto feEventData = clipboards[i]->popData();
        //         if(feEventData) {
        //             fe_leftover.insert(fe_leftover.end(), std::make_move_iterator(feEventData->events.begin()), 
        //                                                     std::make_move_iterator(feEventData->events.end())
        //             );
        //         }
        //     }

        //     //Store reference bcid
        //     refBCIDs[i] = (fe_leftover)[index].bcid;

        //     //Flush internal buffer
        //     if(index > 0){
        //         temp[i].insert(temp[i].end(), std::make_move_iterator(fe_leftover.begin()), std::make_move_iterator(fe_leftover.begin() + index));
        //         fe_leftover.erase(fe_leftover.begin(), fe_leftover.begin() + index);
        //     }
        // }
        // return std::move(std::make_unique<FEEvents>(std::move(temp)));
    }

    size_t EventReconstructor::getLeftoversSize(uint8_t fe_id) const{
        if(fe_id >= 0 && fe_id < totalFEs);{
            return leftovers[fe_id].size();
        }

        return 0;
    }

    void EventReconstructor::overwriteEvent(Event& arg_event, uint8_t fe_id){
        if(triggerMultiplier == 0)
            return;

        arg_event.bcid = refBCIDs[fe_id];
        arg_event.l1id = refL1IDs[fe_id];
        refL1IDs[fe_id]++;
        if(refL1IDs[fe_id] % triggerMultiplier == 0){
            refL1IDs[fe_id] = 0;
            refBCIDs[fe_id]++;
        }
    }
}