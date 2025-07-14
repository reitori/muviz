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
#include <glm/glm.hpp>

namespace viz{

    enum OrientationMode{
        QUAT, XYZ, ZYX, ZXZ
    };


    struct Hit {
        uint16_t col : 16;
        uint16_t row : 16;
        uint16_t tot : 16;
    };

    struct pixelHit {
        uint16_t row;
        uint16_t col;
    };

    struct Track{
        enum type{
            straight,
            multiplet,
            global
        };

        Track::type trackType;
        virtual ~Track() = default;
    }; 

    struct StraightLineTrack : Track{
            glm::vec3 point;
            glm::vec3 direction;
            glm::vec4 uncertainties; 
    };

    using TrackData = std::vector<std::unique_ptr<Track>>;

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
                events.emplace_back(Event(arg_tag, arg_l1id, arg_bcid));
                curEvent = &events.back();

                if(curEvent != nullptr && (arg_bcid != curEvent->bcid + 1 || arg_bcid != curEvent->bcid)){
                    has_desync_events = true;
                    desync_event_indices.push_back(events.size()); //corresponds to the index in which this new desynced event will be placed
                }
                if(curEvent->l1id == 666){
                    has_invalid_l1id = true;
                    invalid_l1id_indices.push_back(events.size());
                }
            }

            void addEvent(const Event& new_event){
                newEvent(new_event.tag, new_event.l1id, new_event.bcid);
                nHits += new_event.nHits;
            }
        
            void delete_back() {
                if(desync_event_indices.back() == events.size()){
                    desync_event_indices.pop_back();
                    if(desync_event_indices.empty()){
                        has_desync_events = false;
                    }
                }
                if(invalid_l1id_indices.back() == events.size()){
                    invalid_l1id_indices.pop_back();
                    if(invalid_l1id_indices.empty()){
                        has_invalid_l1id = false;
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

            bool has_invalid_l1id = false;
            std::vector<uint32_t> invalid_l1id_indices;

            bool has_desync_events = false;
            std::vector<uint32_t> desync_event_indices;
            
            std::string fromChip;
            Event* curEvent;
            std::vector<Event> events;
            uint16_t nHits = 0;
    };

    //Weird data structure but essentially a matrix (EventData is a wrapper for std::vector<Event>) where:
    //Rows are the total number of FEs
    //Each EventData per row refers to the total events for each FE
    //std::unique_ptr forces move semantics to prevent unnecessary copying
    //Used for event reconstruction
    using FEEvents = std::vector<std::unique_ptr<EventData>>;

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

            std::string name;
        protected:
            std::shared_ptr<ClipBoard<EventData>> output;
            std::unique_ptr<std::thread> thread_ptr;
    };

}

#endif