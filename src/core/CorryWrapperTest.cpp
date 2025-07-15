#include <iostream>
#include <fstream>
#include <vector>
#include <json.hpp>
#include <cstdint>

#include "cli.h"
#include "AllDataLoaders.h"
#include "YarrBinaryFile.h"
#include "EventReconstructor.h"


#include "TApplication.h"
#include "TROOT.h"
#include "TSystem.h"
#include "TH1.h"

using namespace viz;

std::vector<std::ofstream> YARRoutputFiles;
std::vector<std::unique_ptr<DataLoader>> dataLoaders;

std::unique_ptr<VisualizerCli> cli;

//For running tests
std::vector<std::thread> threads;
std::vector<std::pair<std::unique_ptr<FEEvents>, std::unique_ptr<TrackData>>> hsi;

auto logger = logging::make_log("CLITest");

void testThread(int i){
    while(true){
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

int main(int argc, char** argv){
    ROOT::EnableThreadSafety();
    ROOT::EnableImplicitMT();

    if(argc < 3){
        std::cout << "Please provide a configuration file path" << std::endl;
        return 0;
    }

    auto config = cli_helpers::openJsonFile(std::string(argv[2]));

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

    for(int i = 0; i < 10; i++){
        threads.push_back(std::thread(&testThread, i));
    }
    
    cli = std::make_unique<VisualizerCli>();
    int cli_status = cli->init(argc, argv); 
    cli_status = cli->configure(); 

    if(cli_status < 0){
        logger->error("CLI Error Code: {}", cli_status);
        return -1;
    }

    cli->start();

    while(true){
        auto eventBatch = cli->getEventBatch();
        if(eventBatch.second){
            TrackData* tracks = eventBatch.second.get();
            for(auto& track : *tracks){
                if(track->trackType == Track::type::straight){
                    StraightLineTrack* straight = static_cast<StraightLineTrack*>(track.get());
                    // std::cout << "StraightLineTrack " << "(" << straight->point.x << "," << straight->point.y << "," << straight->point.z << "), "
                    //                                   << "(" << straight->direction.x << "," << straight->direction.y << "," << straight->direction.z << "), "
                    //                                   << straight->uncertainties.x << ", " << straight->uncertainties.y << ", " << straight->uncertainties.z << ", "
                    //                                   << straight->uncertainties.a << "\n";
                }
            }
        }
    }

    

    return 0;
}   