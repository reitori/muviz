#ifndef DATA_BASE_H
#define DATA_BASE_H

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
        ~Event() = default;
        
        void addEvent(const Event& event) {
            hits.insert(hits.end(), event.hits.begin(), event.hits.end());
            nHits += event.nHits;
        }

        void addHit(Hit hit) {
            hits.push_back(hit);
            nHits++;
        }

        void addHit(unsigned arg_row, unsigned arg_col, unsigned arg_timing) {
            hits.push_back(Hit{static_cast<uint16_t>(arg_col), static_cast<uint16_t>(arg_row), static_cast<uint16_t>(arg_timing)});
            nHits++;
        }

        uint16_t l1id;
        uint16_t bcid;
        uint32_t tag;
        uint16_t nHits;
        std::vector<Hit> hits;
};

class EventData {
    public:
        EventData() = default;
        ~EventData() = default;

        void newEvent(unsigned arg_tag, unsigned arg_l1id, unsigned arg_bcid) {
            events.emplace_back(Event(arg_tag, arg_l1id, arg_bcid));
            curEvent = &events.back();
        }

        void delete_back() {
            events.pop_back();
            curEvent = &events.back();
        }

        void addHit(Hit hit) {
            curEvent->addHit(hit);
        }

        void addHit(unsigned arg_row, unsigned arg_col, unsigned arg_timing) {
            curEvent->addHit(arg_row, arg_col, arg_timing);
        }

        size_t size() {
            return events.size();
        }

        Event* curEvent;
        std::vector<Event> events;
};

class DataLoader{
    public:
        DataLoader() = default;
        ~DataLoader() = default;
        virtual void init() = 0;
        virtual void configure(const json &arg_config) {}; // defined by default
        virtual void connect(ClipBoard<EventData> *arg_output)  {
            output = arg_output;
        }
        virtual void run() = 0;
        virtual void join() = 0;

    protected:
        ClipBoard<EventData> *output;
        std::unique_ptr<std::thread> thread_ptr;
};


#endif