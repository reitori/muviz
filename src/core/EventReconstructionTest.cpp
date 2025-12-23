#include <iostream>
#include <fstream>
#include <vector>
#include <json.hpp>
#include <cstdint>

#include "cli.h"
#include "AllDataLoaders.h"
#include "YarrBinaryFile.h"
#include "EventReconstructor.h"

using namespace viz;

std::vector<std::ofstream> YARRoutputFiles;
std::vector<std::unique_ptr<DataLoader>> dataLoaders;

void toMemoryBinary(std::vector<uint8_t> &handle, const Event& event){
    size_t data_size = sizeof(uint32_t) + (3 * sizeof(uint16_t)) + (event.hits.size() * sizeof(Hit));
    size_t offset = handle.size();
    handle.resize(offset + data_size);
    
    memcpy(handle.data() + offset, &event.tag, sizeof(uint32_t));   offset += sizeof(uint32_t);
    memcpy(handle.data() + offset, &event.l1id, sizeof(uint16_t));  offset += sizeof(uint16_t);
    memcpy(handle.data() + offset, &event.bcid, sizeof(uint16_t));  offset += sizeof(uint16_t);
    memcpy(handle.data() + offset, &event.nHits, sizeof(uint16_t)); offset += sizeof(uint16_t);
    memcpy(handle.data() + offset, event.hits.data(), event.hits.size() * sizeof(Hit));
}


void toFileBinary(std::ofstream &handle, const Event& event){
    std::vector<uint8_t> buffer;
    toMemoryBinary(buffer, event);
    handle.write(reinterpret_cast<const char*>(buffer.data()), buffer.size());
}

int main(int argc, char** argv){
    if(argc < 2){
        std::cout << "Please provide a configuration file path" << std::endl;
        return 0;
    }

    auto config = cli_helpers::openJsonFile(std::string(argv[1]));

    json globalConfig = nlohmann::json::object();
    json sourcesConfig = nlohmann::json::object();

    if(config.contains("global_source_config"))
        globalConfig = config["global_source_config"];
    else{
        std::cout << "Provide a global source parameter in config" << std::endl;
        return 0;
    }
    if(config.contains("sources"))
        sourcesConfig = config["sources"];
    else{
        std::cout << "Provide sources parameter in config" << std::endl;
        return 0;
    }

    int totalFEs = sourcesConfig.size();
    YARRoutputFiles.resize(totalFEs);
    
    EventReconstructor reconstructor(totalFEs);

    for(int i = 0; i < totalFEs; i++){
        auto source = sourcesConfig[i];
        FEBookie::addFE(i, source["name"]);

        source.merge_patch(globalConfig);
        dataLoaders.push_back(StdDict::getDataLoader(source["type"]));
        auto& currDataLoader = dataLoaders.back();
        currDataLoader->configure(source);
        currDataLoader->init();
        reconstructor.configure(currDataLoader.get(), i, globalConfig);
        currDataLoader->run();
    }

   
    for(int i = 0; i < YARRoutputFiles.size(); i++){
         std::cout << "path " << (std::string)sourcesConfig[i]["name"] + std::string(".raw") << std::endl;
        YARRoutputFiles[i].open((std::string(sourcesConfig[i]["name"]) + std::string(".raw")).c_str(), std::fstream::out | std::fstream::binary | std::fstream::trunc);
    }


    int retries = 0;
    int loop = 0;
    int totalRetries = 10;
    int target = 153137;
    std::vector<int> totalEvents(totalFEs);
    while (true) {
        auto fe_events = reconstructor.getEvents();
        bool anyWritten = false;
        for (int i = 0; i < totalFEs; ++i) {
            if (!fe_events[i].empty()) {
                anyWritten = true;
                for (auto &e : fe_events[i])
                    toFileBinary(YARRoutputFiles[i], e);
            }
        }

        if (!anyWritten) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            retries++;
        }
        bool allDrained = true;
        for (int i = 0; i < totalFEs; ++i) {
            if (!reconstructor.clipboards[i]->empty() 
                || !reconstructor.clipboards[i]->isDone()
                || !fe_events[i].empty()) {
                allDrained = false;
                break;
            }
        }
        if(allDrained){
            break;
        }
    }

    for (auto &dl : dataLoaders) dl->join();

    while (true) {
        auto fe_events = reconstructor.getEvents();
        bool gotOne = false;
        for (int i = 0; i < totalFEs; ++i) {
            if (!fe_events[i].empty()) {
                gotOne = true;
                for (auto &e : fe_events[i])
                    toFileBinary(YARRoutputFiles[i], e);
            }
        }

        if (!gotOne) break;
    }

    for(int i = 0; i < totalFEs; i++){
                std::cout << "Chip: " << FEBookie::getName(i) << " has " << reconstructor.leftovers[i].size() << " leftover events" << std::endl;
    }

    for(int i = 0; i < totalFEs; i++){
        if(reconstructor.clipboards[i]->size()){
            std::cout << "Clipboard associated with chip " << FEBookie::getName(i) << " still has data though!?" << std::endl;
        }
        std::cout << FEBookie::getName(i) << " has a total event count: " << reconstructor.totalEvents[i] << std::endl;
        std::cout << "       " <<            " has recieved event total " << totalEvents[i] << std::endl;
        std::cout << "       " <<            " has tot clipboard count: " << reconstructor.clipboards[i]->getNumDataIn() << std::endl;
        std::cout << "       " <<            " has total clipboard out: " << reconstructor.clipboards[i]->getNumDataOut() << "\n" << std::endl;
    }

    for(int i = 0; i < YARRoutputFiles.size(); i++){
        YARRoutputFiles[i].close();
    }

    return 0;
}   