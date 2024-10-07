#ifndef YARR_BINARY_H
#define YARR_BINARY_H

#include "DataBase.h"
#include <fstream>
#include <iostream>
#include <thread>

class YarrBinary : public DataLoader {
public:
    void init() override;
    void configure(const json &arg_config) override;
    void run() override;
    void join() override;
private:
    void process();
    void fromFile(std::fstream &handle);
    
    uint32_t this_tag;
    uint16_t this_l1id, this_bcid;

    unsigned max_events_per_block;
    bool run_thread;

    std::unique_ptr<EventData> curEvents;
};

#endif