#include "YarrBinaryFile.h"
#include "AllDataLoaders.h"
#include "logging.h"

namespace
{
    auto logger = logging::make_log("YarrBinaryFile");
    bool YarrBinaryFileRegistered =
      StdDict::registerDataLoader("YarrBinaryFile",
                                []() { return std::unique_ptr<DataLoader>(new YarrBinaryFile());});

}
// void YarrBinaryFile::init() override;
// void YarrBinaryFile::configure(const json &arg_config) override;

void YarrBinaryFile::run() {
    run_thread = true;
    thread_ptr.reset(new std::thread(&YarrBinaryFile::process, this));
}

void YarrBinaryFile::join() {
    run_thread = false;
    thread_ptr->join();
}

void YarrBinaryFile::init() {
    this_tag = 0;
    this_l1id = 0;
    this_bcid = 0;
    max_events_per_block = (unsigned)(-1);
    run_thread = false;
}

void YarrBinaryFile::configure(const json &arg_config) {
    config = arg_config;
    if(config.contains("name")) {
        name = config["name"]; 
    }
    else {
        logger->error("No name provided! Please add name to all sources. Assuming name = None");
        throw(std::invalid_argument("No name provided!"));
    }

    bool auto_path = false;
    if(config.contains("config")) {
        if(config["config"].contains("auto")) {
            auto_path = (bool)config["config"]["auto"];
        }
    }

    filename = "";
    if(auto_path)
        filename = std::to_string(config["path"]) + "/" + name + "_data.raw";
    else
        filename = name;
    
    fileHandle.open(filename.c_str(), std::fstream::out | std::fstream::binary | std::fstream::trunc);
    if(!fileHandle.good()) 
        throw(std::invalid_argument("Path " + filename + " could not be opened!"));
}

void YarrBinaryFile::processBatch() {
    int sleep_step = 0;
    while(fileHandle && run_thread && (curEvents != nullptr)) { // basic case of "block lives"
        fromFile();
    }
}

void YarrBinaryFile::process() {
    int batch = 0;
    while(run_thread) {
        // Make new block of events
        curEvents = std::make_unique<EventData>();
        processBatch();
        output->pushData(std::move(curEvents));
        batch++;
    }
}

void YarrBinaryFile::fromFile() {
    uint16_t t_hits = 0;
    fileHandle.read((char*)&this_tag, sizeof(uint32_t));
    fileHandle.read((char*)&this_l1id, sizeof(uint16_t));
    fileHandle.read((char*)&this_bcid, sizeof(uint16_t));
    fileHandle.read((char*)&t_hits, sizeof(uint16_t));

    curEvents->newEvent(this_tag, this_l1id, this_bcid);
    for (unsigned ii = 0; ii < t_hits; ii++) {
        Hit hit;
        fileHandle.read((char*)&hit, sizeof(Hit));
        curEvents->curEvent->addHit(hit);
    } // ii
}


// #include "EventData.h"

// // std/stl
// #include <iostream>
// #include <fstream>

// void YarrBinaryFile::toFile(std::fstream &handle) const {
//     handle.write((char*)&tag, sizeof(uint32_t));
//     handle.write((char*)&l1id, sizeof(uint16_t));
//     handle.write((char*)&bcid, sizeof(uint16_t));
//     handle.write((char*)&nHits, sizeof(uint16_t));
//     for (auto hit : hits) {
//         handle.write((char*)&hit, sizeof(FrontEndHit));
//     } // h
// }

// void YarrBinaryFile::fromFile(std::fstream &handle) {
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
