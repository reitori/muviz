#include <iostream>
#include <string>
#include <csignal>

#include "cli.h"
#include "AllDataLoaders.h"
#include "DataBase.h"
#include "YarrBinaryFile.h"
namespace
{
    auto logger = logging::make_log("VisualizerCLI");
}


sig_atomic_t signaled = 0;
int main(int argc, char** argv) {

    // option parsing
    viz_cli::ScanOpts options;
    int ret = viz_cli::parseOptions(argc, argv, options);
    
    viz_cli::setupLoggers(options.verbose);

    if (ret != 1) {
        viz_cli::printHelp();
        return ret;
    }

    logging::banner(logger, "Visualizer CLI Program");

    json config = viz_cli::openJsonFile(options.configPath);
    
    std::vector<std::unique_ptr<DataLoader>> dataLoaders;
    std::vector<std::unique_ptr<ClipBoard<EventData>>> clipboards;

    for(int i = 0, k=0; i < config["sources"].size(); i++) {
        auto source = config["sources"][i];
        if(!source.contains("name")) {
            source["name"] = "anon_" + std::to_string(i);
        }
        if(config.contains("global_source_config")) {
            source.merge_patch(config["global_source_config"]);
        }
        
        int enable = 1;
        if(source.contains("enable")) {
            enable = (int)source["enable"];
        }

        if(enable) {
            dataLoaders.push_back(StdDict::getDataLoader(source["type"]));
            dataLoaders[k++]->configure(source);
            clipboards.push_back(std::make_unique<ClipBoard<EventData>>());
        }
        else {
            logger->info("Skipping disabled FE with ID {}, name {}", i, source["name"]);
        }
    }

    for(int i = 0; i < dataLoaders.size(); i++) {
        dataLoaders[i]->init();
        dataLoaders[i]->connect(clipboards[i].get());
        dataLoaders[i]->run();
    }

    signal(SIGINT, [](int signum){signaled = 1;});
    signal(SIGUSR1, [](int signum){signaled = 1;});

    while(signaled == 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    std::cout << "\r";
    logger->info("Caught interrupt, stopping threads");
    
    for(int i = 0; i < dataLoaders.size(); i++) {
        dataLoaders[i]->join();
        logger->info("FE with ID {}: size {} / {}", i, clipboards[i]->getNumDataIn(), clipboards[i]->size());
    }

    logger->info("test main end");
    return 0;
}
