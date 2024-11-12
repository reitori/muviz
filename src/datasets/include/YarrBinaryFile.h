#ifndef YARR_BINARY_FILE_H
#define YARR_BINARY_FILE_H

#include "DataBase.h"
#include <fstream>
#include <thread>
#include <stdexcept>

class YarrBinaryFile : public DataLoader {
public:
    void init() override;
    void configure(const json &arg_config) override;
    void run() override;
    void join() override;
private:
    void process();
    void processBatch();
    void fromFile();
    
    uint32_t this_tag;
    uint16_t this_l1id, this_bcid;

    unsigned max_events_per_block;
    bool run_thread;
    

    std::string name, filename;
    std::fstream fileHandle;
    json& config;
    std::unique_ptr<EventData> curEvents;
};

#endif