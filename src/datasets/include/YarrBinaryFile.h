#ifndef YARR_BINARY_FILE_H
#define YARR_BINARY_FILE_H

#include "DataBase.h"
#include <fstream>
#include <iostream>
#include <thread>
#include <stdexcept>
#include <chrono>

class YarrBinaryFile : public DataLoader {
public:
    YarrBinaryFile();
    ~YarrBinaryFile();
    
    // interface
    void init() override;
    void configure(const json &arg_config) override;
    void run() override;
    void join() override;
    
private:
    // implementation
    void process();
    void processBatch();
    bool fromFile();

    void readHeader();
    void readHits();
    
    uint32_t this_tag;
    uint16_t this_l1id, this_bcid, this_t_hits;


    unsigned max_events_per_block, block_timeout; // configurable parameters
    
    unsigned total_events, batch_n, total_hits; // counters for reporting
    bool run_thread, header_read, hit_read;

    std::string name, filename;
    std::fstream fileHandle;
    std::streampos filePos;
    std::unique_ptr<EventData> curEvents;

};

#endif