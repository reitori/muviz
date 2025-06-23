#ifndef DATA_BASE_H
#define DATA_BASE_H

// #################################################
// # Author: Luc Le Pottier, Koji Abel             #
// # Email: luclepot / kabel at lbl.gov            #
// # Project: YARR-event-visualizer                #
// # Description: CLI for data reading interfaces  #
// #################################################

#include "ClipBoard.h"
#include "util.hpp"

#include <memory>
#include <thread>
#include <vector>
#include <csignal>

struct Hit {
    uint16_t col : 16;
    uint16_t row : 16;
    uint16_t tot : 16;
};

struct pixelHit {
    uint16_t row;
    uint16_t col;
};

class Event {
    public:
        Event() {
            tag = 0;
            l1id = 0;
            bcid = 0;
            nHits = 0;
        }
        Event(unsigned arg_tag, unsigned arg_l1id, unsigned arg_bcid) {
            tag = arg_tag;
            l1id = arg_l1id;
            bcid = arg_bcid;
            nHits = 0;
        }

        void addHit(Hit hit) {
            hits.push_back(hit);
            nHits++;
        }

        void addHit(unsigned arg_row, unsigned arg_col, unsigned arg_timing) {
            hits.push_back(Hit{static_cast<uint16_t>(arg_col), static_cast<uint16_t>(arg_row), static_cast<uint16_t>(arg_timing)});
            nHits++;
        }

        uint32_t l1id, bcid, tag;
        uint16_t nHits = 0;
        std::vector<Hit> hits;
};


//Vector-like storage container for Events. Identifies if desynchronization occurs when adding new Events.
//Desynchronization when vector of events no longer has monotonically increasing bcid. 
class EventData {
    public:
        EventData() = default;
        ~EventData() = default;

        void newEvent(unsigned arg_tag, unsigned arg_l1id, unsigned arg_bcid) {
            if(curEvent != nullptr && (arg_bcid != curEvent->bcid + 1 || arg_bcid != curEvent->bcid)){
                has_desync_events = true;
                desync_indices.push_back(events.size()); //corresponds to the index in which this new desynced event will be placed
            }

            events.emplace_back(Event(arg_tag, arg_l1id, arg_bcid));
            curEvent = &events.back();
        }

        void addEvent(const Event& new_event){
            newEvent(new_event.tag, new_event.l1id, new_event.bcid);
            nHits += new_event.nHits;
        }
    
        void delete_back() {
            if(events.size() == desync_indices.back()){
                desync_indices.pop_back();
                if(desync_indices.empty()){
                    has_desync_events = false;
                }
            }

            nHits -= events.back().nHits;
            events.pop_back();
            curEvent = &events.back();
        }

        void addHit(Hit hit) {
            curEvent->addHit(hit);
            nHits++;
        }

        void addHit(unsigned arg_row, unsigned arg_col, unsigned arg_timing) {
            curEvent->addHit(arg_row, arg_col, arg_timing);
            nHits++;
        }

        bool empty() const{
            return events.empty();
        }

        size_t size() {
            return events.size();
        }

        bool has_desync_events = false;
        std::vector<uint32_t> desync_indices;
        
        Event* curEvent;
        std::vector<Event> events;
        uint16_t nHits = 0;
};

// class ReconstructedBunch{
//     public:
//         ReconstructedBunch(){
//             bcid = 0;
//             totalFEs = 0;
//             nHits = 0;
//         }
//         ReconstructedBunch(uint32_t arg_bcid, size_t arg_totalFEs) {
//             bcid = arg_bcid;
//             totalFEs = arg_totalFEs;
//             nHits = 0;

//             fe_events.resize(arg_totalFEs);
//         }
//         void addEvent(const Event& newEvent, uint16_t fe_id){
//             if(this->bcid != newEvent.bcid || fe_id > totalFEs)
//                 return;

//             nHits += newEvent.nHits;
//             fe_events[fe_id].addEvent(newEvent);
//         }

//         void addEventData(const EventData& newEventData, uint16_t fe_id){
//             for(int i = 0; i < newEventData.events.size(); i++){
//                 if(newEventData.events[i].bcid == bcid){
//                     fe_events[fe_id].addEvent(newEventData.events[i]);
//                     nHits += newEventData.nHits;
//                 }
//             }
//         }
    
//         //Adds EventData using the bcidChangeIndex (Assumes that the bcidChangeIndex indexes events correctly)
//         void addEventDataCI(const EventData& newEventData, uint16_t fe_id){
//             std::vector<size_t> bcidChangeIndex = newEventData.bcidChangeIndex;
//             if(bcidChangeIndex.size()!=0){
//                 for(int i = 0; i < bcidChangeIndex.size(); i++){
//                     if(newEventData.events[bcidChangeIndex[i]].bcid == bcid){
//                         size_t endIndex = ( i == (bcidChangeIndex.size() - 1)) ? bcidChangeIndex.size() : bcidChangeIndex[i+1];
//                         for(size_t j = bcidChangeIndex[i]; j < endIndex; j++){
//                             fe_events[fe_id].addEvent(newEventData.events[j]);
//                             nHits += newEventData.events[j].nHits;
//                         }
//                     }
//                 }
//             }
//             else if(bcidChangeIndex.size() == 0 && !newEventData.events.empty() && bcid == newEventData.events[0].bcid){
//                 fe_events[fe_id].addEventData(newEventData);
//             }
//         }
//         void addReconstructedBunch(const ReconstructedBunch& newBunch){
//             if(this->bcid != newBunch.bcid || this->bcid != newBunch.totalFEs)
//                 return; 

//             for(unsigned int i = 0; i < newBunch.totalFEs; i++){
//                 fe_events[i].addEventData(newBunch.fe_events[i]);
//             }
//         }
//         //Gives the memory to user
//         std::unique_ptr<std::vector<EventData>> getEventData(){
//             return std::make_unique<std::vector<EventData>>(std::move(fe_events));
//         }
//         std::unique_ptr<EventData> getEventDataFE(uint16_t fe_id){
//             return std::make_unique<EventData>(std::move(fe_events[fe_id]));
//         }
//         uint16_t totalFEs;
//         uint32_t nHits, bcid;
//     private:
//         std::vector<EventData> fe_events; //Event with associated fe_id. ie, access events of fe with id fe_id fe_events[fe_id]
// };


class DataLoader{
    public:
        DataLoader() = default;
        virtual ~DataLoader() = default;
        virtual void init() = 0;
        virtual void configure(const json &arg_config) {}; // defined by default
        virtual void connect(std::shared_ptr<ClipBoard<EventData>> arg_output)  {
            output = arg_output;
        }
        virtual void run() = 0;
        virtual void join() = 0;

    protected:
        std::shared_ptr<ClipBoard<EventData>> output;
        std::unique_ptr<std::thread> thread_ptr;
};


#endif