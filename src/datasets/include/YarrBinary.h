#ifndef YARR_BINARY_H
#define YARR_BINARY_H

#include "DataBase.h"
#include <fstream>

class YarrBinary : public DataLoader {
public:
    // void init() override;
    // void configure(const json &arg_config) override;
    void run() override;
    void join() override;
private:
    void fromFile(std::fstream &handle);
    
    uint32_t this_tag;
    uint16_t this_l1id, this_bcid;

    std::unique_ptr<EventData> curEvents;
};

#endif