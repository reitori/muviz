#include <iostream>
#include <fstream>
#include <vector>
#include <json.hpp>
#include <cstdint>

#include "cli.h"
#include "AllDataLoaders.h"
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

    auto config = cli_helpers::openJsonFile(argv[1]);

    auto globalConfig = json();
    auto sourcesConfig = json();

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
        dataLoaders.push_back(StdDict::getDataLoader(source["type"]));
        auto& currDataLoader = dataLoaders.back();
        currDataLoader->init();
        reconstructor.configure(currDataLoader.get(), i, globalConfig);
        currDataLoader->run();
    }



    int retries = 0;
    while(retries < 10){
        auto fe_events = reconstructor.getEvents();
        
        //Not important just checks
        bool hasEvent = false;
        for(int i = 0; i < fe_events.size(); i++){
            if(fe_events[i].size() > 0)
                hasEvent = true;
        }
        if(hasEvent){
            retries++;
            continue;
        }
        //

        for(int i = 0; i < fe_events.size(); i++){
            YARRoutputFiles[i].open((std::string(sourcesConfig[i]["name"]) + std::string(".raw")).c_str(), std::fstream::out | std::fstream::binary | std::fstream::trunc);
            for(int j = 0; j < fe_events.size(); j++){
                toFileBinary(YARRoutputFiles[i], fe_events[i][j]);
            }
        }
    }

    return 0;
}   