#include "datasets/include/EventReconstructor.h"

#include <iostream>

namespace{
    auto logger = logging::make_log("EventReconstructor");
}

namespace viz{
    EventReconstructor::EventReconstructor(const uint8_t& total_fes){
        clipboards.resize(total_fes, std::make_shared<ClipBoard<EventData>>());
        leftovers.resize(total_fes);
        refBCIDs.resize(total_fes, 0);
        refL1IDs.resize(total_fes, 0);

        totalFEs = total_fes;
        triggerMultiplier = 0;
    }

    void EventReconstructor::configure(DataLoader* dataloader, uint8_t fe_id, const json& config){
        if(fe_id >= 0 && fe_id < totalFEs)
            dataloader->connect(clipboards[fe_id]);

        if(config.contains("trigger_multiplier") && config["trigger_multiplier"].is_number_unsigned()){
            triggerMultiplier = config["trigger_multiplier"];
            logger->info("trigger_multiplier is set to {0}. Event BCIDs and L1IDs will be overriden.", triggerMultiplier);
        } 
    }

    std::vector<std::vector<Event>> EventReconstructor::getEvents(){
        std::vector<std::vector<Event>> tempReturn; tempReturn.resize(totalFEs);
        std::vector<std::unique_ptr<EventData>> data;
        std::vector<std::pair<int, int>> index_size;

        data.resize(totalFEs);
        index_size.reserve(totalFEs);
        for(int i = 0; i < totalFEs; i++){
            data[i] = std::move(clipboards[i]->popData());
            int dataSize = (bool)(data[i]) ? data[i]->size() : 0; //Handles case where popData() returns nullptr
            index_size.push_back(std::make_pair(i, dataSize + leftovers[i].size()));

            //Check some extraneous cases
            if(data[i]){
                if(data[i]->has_desync_events){
                    //Do something here for desync
                }
                if(triggerMultiplier == 0 && data[i]->has_invalid_l1id){
                    logger->info("Chip {0} has invalid l1id. Overwriting bcid from here on out.", FEBookie::getName(i));
                    triggerMultiplier = 1;
                }
            }
        }
        std::sort(index_size.begin(), index_size.end(), 
                  [](const std::pair<int, int>& left, const std::pair<int, int>& right){
                        return left.second < right.second;
                }); //Sort based on smallest number of events in leftovers plus incoming data

        unsigned int numToSend = index_size.front().second;
        if(triggerMultiplier > 0){ //Trigger multiplier mode
            //Load onto internal buffer before sending and rewrite bcid and l1ids
            for(int i = 0;  i < totalFEs; i++){
                if(!data[i])
                    continue;

                std::vector<Event>& currEvents = data[i]->events;
                if(leftovers[i].size() > numToSend){
                    if(numToSend > 0){
                        tempReturn[i].insert(tempReturn[i].end(), std::make_move_iterator(leftovers[i].begin()), std::make_move_iterator(leftovers[i].begin() + numToSend));
                        leftovers[i].erase(leftovers[i].begin(), leftovers[i].begin() + numToSend - 1);
                    }

                    for(int j = 0; j < currEvents.size(); j++){
                        overwriteEvent(currEvents[j], i);
                        leftovers[i].push_back(currEvents[j]);
                    }
                }
                else if(leftovers[i].size() <= numToSend){
                    unsigned int relFinalIndex = numToSend - leftovers[i].size();
                    tempReturn[i] = std::move(leftovers[i]); // Fill return with all leftovers

                    //Overwrite event bcid and l1id
                    for(int j = 0; j < relFinalIndex; j++){ //Fill up return with all events ready to be batched
                        overwriteEvent(currEvents[j], i);
                        tempReturn[i].push_back(currEvents[j]);
                    }
                    for(int j = relFinalIndex; j < currEvents.size(); j++){ //Fill leftovers with everything else
                        overwriteEvent(currEvents[j], i);
                        leftovers[i].push_back(currEvents[j]);
                    }
                }
                else{
                    tempReturn[i] = std::move(leftovers[i]);
                }
            }
        }else{
            //TODO: Mode relying on bcid?
        }

        return std::move(tempReturn);
    }  

    //Synchronize based off of internal buffer (leftovers). Index refers to nth event that is resynchronized for all front ends.
    //Events in the internal buffer coming before the index are flushed and given as the return
    std::vector<std::vector<Event>> EventReconstructor::synchronize(uint16_t index){
        std::vector<std::vector<Event>> temp;
        for(int i = 0; i < totalFEs; i++){
            //Fill up internal buffer up to necessary index
            std::vector<Event>& fe_leftover = fe_leftover;
            while(index > fe_leftover.size()){
                auto feEventData = clipboards[i]->popData();
                if(feEventData) {
                    fe_leftover.insert(fe_leftover.end(), std::make_move_iterator(feEventData->events.begin()), 
                                                            std::make_move_iterator(feEventData->events.end())
                    );
                }
            }

            //Store reference bcid
            refBCIDs[i] = (fe_leftover)[index].bcid;

            //Flush internal buffer
            if(index > 0){
                temp[i].insert(temp[i].end(), std::make_move_iterator(fe_leftover.begin()), std::make_move_iterator(fe_leftover.begin() + index));
                fe_leftover.erase(fe_leftover.begin(), fe_leftover.begin() + index);
            }
        }
        return std::move(temp);
    }

    void EventReconstructor::overwriteEvent(Event& arg_event, uint8_t fe_id){
        arg_event.bcid = refBCIDs[fe_id];
        arg_event.l1id = refL1IDs[fe_id];
        refL1IDs[fe_id]++;
        if(refL1IDs[fe_id] % triggerMultiplier == 0){
            refL1IDs[fe_id] = 0;
            refBCIDs[fe_id]++;
        }
    }
}