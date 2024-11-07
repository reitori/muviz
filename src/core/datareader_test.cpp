#include "DataBase.h"
#include "YarrBinaryFile.h"
#include <iostream>
#include "cli.h"
#include "AllDataLoaders.h"

namespace
{
    auto logger = logging::make_log("VisualizerCLI");
}

json openJsonFile(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file) {
        throw std::runtime_error("could not open file");
    }
    json j;
    try {
        j = json::parse(file);
    } catch (json::parse_error &e) {
        throw std::runtime_error(e.what());
        throw std::runtime_error(e.what());
    }
    file.close();
    // variant produces null for some parse errors
    if(j.is_null()) {
        throw std::runtime_error("Parsing json file produced null");
    }
    return j;
}

int main(int argc, char** argv) {

    // option parsing
    ScanOpts options;
    int ret = parseOptions(argc, argv, options);
    
    setupLoggers();

    if (ret != 1) {
        printHelp();
        return ret;
    }

    logging::banner(logger, "Visualizer CLI Program");

    json config = openJsonFile(options.configPath);
    
    std::vector<std::unique_ptr<DataLoader>> dataLoaders;
    std::vector<std::unique_ptr<ClipBoard<EventData>>> clipboards;

    for(int i = 0; i < config["sources"].size(); i++) {
        auto source = config["sources"][i];
        if(config.contains("path"))
            source["path"] = config["path"];

        dataLoaders.push_back(StdDict::getDataLoader(source["type"]));
        dataLoaders[i]->configure(source);
        clipboards.push_back(std::make_unique<ClipBoard<EventData>>());
    }

    for(int i = 0; i < dataLoaders.size(); i++) {
        dataLoaders[i]->init();
        dataLoaders[i]->connect(clipboards[i].get());
        dataLoaders[i]->run();
    }

    for(int i = 0; i < dataLoaders.size(); i++) {
        dataLoaders[i]->join();

        logger->info("FE with ID {}: size {} / {}", i, clipboards[i]->getNumDataIn(), clipboards[i]->size());
    }

    logger->info("test main end");
    return 0;
}
