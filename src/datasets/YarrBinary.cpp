#include "YarrBinary.h"
#include "logging.h"

namespace
{
    auto logger = logging::make_log("YarrBinaryReader");
}
// void YarrBinary::init() override;
// void YarrBinary::configure(const json &arg_config) override;

void YarrBinary::run() {
    run_thread = true;
    thread_ptr.reset(new std::thread(&YarrBinary::process, this));
}

void YarrBinary::join() {
    run_thread = false;
    thread_ptr->join();
}

void YarrBinary::init() {
    this_tag = 0;
    this_l1id = 0;
    this_bcid = 0;
    max_events_per_block = (unsigned)(-1);
    run_thread = false;
}

void YarrBinary::configure(const json &arg_config) {
    
}

void YarrBinary::process() {
    int i = 0;
    while(run_thread) {
        // Make new block of events
        curEvents = std::make_unique<EventData>();
        while(run_thread && (curEvents->size() < max_events_per_block)) {
            logger->info("sleep step {}", i++);
            std::this_thread::sleep_for(std::chrono::milliseconds(250));
        }
        output->pushData(std::move(curEvents));
    }
}

void YarrBinary::fromFile(std::fstream &handle) {
    uint16_t t_hits = 0;
    handle.read((char*)&this_tag, sizeof(uint32_t));
    handle.read((char*)&this_l1id, sizeof(uint16_t));
    handle.read((char*)&this_bcid, sizeof(uint16_t));
    handle.read((char*)&t_hits, sizeof(uint16_t));

    curEvents->newEvent(this_tag, this_l1id, this_bcid);
    for (unsigned ii = 0; ii < t_hits; ii++) {
        Hit hit;
        handle.read((char*)&hit, sizeof(Hit));
        curEvents->curEvent->addHit(hit);
    } // ii
}


// #include "EventData.h"

// // std/stl
// #include <iostream>
// #include <fstream>

// void YarrBinary::toFile(std::fstream &handle) const {
//     handle.write((char*)&tag, sizeof(uint32_t));
//     handle.write((char*)&l1id, sizeof(uint16_t));
//     handle.write((char*)&bcid, sizeof(uint16_t));
//     handle.write((char*)&nHits, sizeof(uint16_t));
//     for (auto hit : hits) {
//         handle.write((char*)&hit, sizeof(FrontEndHit));
//     } // h
// }

// void YarrBinary::fromFile(std::fstream &handle) {
//     uint16_t t_hits = 0;
//     handle.read((char*)&tag, sizeof(uint32_t));
//     handle.read((char*)&l1id, sizeof(uint16_t));
//     handle.read((char*)&bcid, sizeof(uint16_t));
//     handle.read((char*)&t_hits, sizeof(uint16_t));
//     for (unsigned ii = 0; ii < t_hits; ii++) {
//         FrontEndHit hit;
//         handle.read((char*)&hit, sizeof(FrontEndHit));
//         this->addHit(hit);
//     } // ii
// }
