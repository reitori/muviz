#include "cli.h"

namespace cli_helpers {

    std::shared_ptr<spdlog::logger> logger = logging::make_log("VizCLI");

    void setupLoggers(bool verbose) {
        json loggerConfig;
        loggerConfig["pattern"] = "[%T:%e]%^[%=8l][%=15n][%t]:%$ %v";
        loggerConfig["log_config"][0]["name"] = "all";
        loggerConfig["log_config"][0]["level"] = verbose ? "debug" : "info";
        loggerConfig["outputDir"] = "";

        logging::setupLoggers(loggerConfig);
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
        }
        file.close();
        // variant produces null for some parse errors
        if(j.is_null()) {
            throw std::runtime_error("Parsing json file produced null");
        }
        return j;
    }

}

using namespace cli_helpers;

void VisualizerCli::printHelp() {

    spdlog::info("Help:");
    spdlog::info(" -h: Shows this.");
    spdlog::info(" -c <config.json> Provide data reader configuration.");
    spdlog::info(" -v Toggles on debug output (default false).");
    // std::cout << " -o <dir> : Output directory. (Default ./data/)" << std::endl;
    // std::cout << " -k: Report known items (Scans, Hardware etc.)\n";
    // std::cout << " -W: Enable using Local DB." << std::endl;
    // std::cout << " -d <database.json> : Provide database configuration. (Default " << dbCfgPath << ")" << std::endl;
    // std::cout << " -i <site.json> : Provide site configuration. (Default " << dbSiteCfgPath << ")" << std::endl;
    // std::cout << " -u <user.json> : Provide user configuration. (Default " << dbUserCfgPath << ")" << std::endl;
    // std::cout << " -l <log_cfg.json> : Provide logger configuration." << std::endl;
    // std::cout << " -Q: Set QC scan mode." << std::endl;
    // std::cout << " -I: Set interactive mode." << std::endl;
    // std::cout << " --skip-reset: Disable sending global front-end reset command prior to running the scan." << std::endl;
}



int VisualizerCli::parseOptions(int argc, char *argv[]) {
    optind = 1; // this is a global libc variable to reset getopt

    for (int i=1;i<argc;i++)
        scanOpts.commandLineStr.append(std::string(argv[i]).append(" "));
    scanOpts.progName=argv[0];
    const struct option long_options[] =
    {
        // {"skip-reset", no_argument, 0, 'z'},
        {"help", no_argument, 0, 'h'},
        // {"version", no_argument, 0, 'v'},
        {0, 0, 0, 0}};
    int c;
    scanOpts.verbose = false;
    while (true) {
        int opt_index=0;
        c = getopt_long(argc, argv, "hc:v", long_options, &opt_index);
        int count = 0;
        if(c == -1) break;
        switch (c) {
            case 'h':
                printHelp();
                return 0;
            case 'c':
                scanOpts.configPath = std::string(optarg);
                break;
            case 'v':
                scanOpts.verbose = true;
                break;
            default:
                spdlog::critical("Error while parsing command line parameters!");
                return -1;
        }
    }

    if(scanOpts.configPath.empty()) {
        spdlog::critical("Configuration file required (-c)");
        return -1;
    }

    return 1;
}

VisualizerCli::VisualizerCli() {
    uncompletedReconEvents = std::make_unique<std::vector<ReconstructedBunch>>();
}

VisualizerCli::~VisualizerCli() {

}

int VisualizerCli::init(int argc, char** argv, CLIstate argstate) {

    state = argstate;

    int ret = parseOptions(argc, argv);
    
    cli_helpers::setupLoggers(scanOpts.verbose);

    if (ret != 1) {
        printHelp();
        return ret;
    }

    logging::banner(logger, "Visualizer CLI Program");

    // option parsing
    logger->info("Opening configuration file with path {}", scanOpts.configPath);
    config = cli_helpers::openJsonFile(scanOpts.configPath);

    for(int i = 0; i < dataLoaders.size(); i++) {
        dataLoaders[i].reset();
        clipboards[i].reset();
    }
    dataLoaders.clear();
    clipboards.clear();
    return 1;
}

int VisualizerCli::configure() {
    logger->info("Configuring data loaders...");

    uint8_t numSources = config["sources"].size();

    if(numSources == 0){
        logger->warn("No sources listed in config file");
    }
    
    names.reserve(numSources);
    configIdMap.reserve(numSources);
    clipboards.reserve(numSources);
    curr_fe_bcid.reserve(numSources);
    
    for(int i = 0, k=0; i < numSources; i++) {
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
            if(!source.contains("position")) {
                logger->error("Config for frontend {} with name '{}' does not contain position vector!", k, source["name"]);
                return -1; // throw std::invalid_argument("missing position vector in config");
            }
            if(!source.contains("angle")) {
                logger->error("Config for frontend {} with name '{}' does not contain angle vector!", k, source["name"]);
                return -1; // throw std::invalid_argument("missing angle vector in config");
            }
            if(!source.contains("size")){
                logger->error("Config for frontend {} with name '{}' does not contain chip size!", k, source["name"]);
                return -1; // throw std::invalid_argument("missing angle vector in config");
            }
            if(!source.contains("rowcol")){
                logger->error("Config for frontend {} with name '{}' does not contain maximum number of rows/columns!", k, source["name"]);
                return -1; // throw std::invalid_argument("missing angle vector in config");
            }

            
            dataLoaders.push_back(StdDict::getDataLoader(source["type"]));
            feIdMap[source["name"]] = k;
            names.push_back(source["name"]);
            curr_fe_bcid.push_back(0);
            configIdMap.push_back(i);
            clipboards.push_back(std::make_shared<ClipBoard<EventData>>());
            dataLoaders[k++]->configure(source);
        }
        else {
            logger->info("Skipping disabled FE with ID {}, name {}", i, source["name"]);
        }
    }

    logger->info("Initalizing and connecting data loaders...");
    for(int i = 0; i < dataLoaders.size(); i++) {
        dataLoaders[i]->init();
        dataLoaders[i]->connect(clipboards[i]);
    }
    return 0;
}

int VisualizerCli::start() {
    if(started){
        logger->info("Data loaders already are running");
        return 0;
    }

    logger->info("Starting data loaders for {} threads", dataLoaders.size());
    for(int i = 0; i < dataLoaders.size(); i++) {
        dataLoaders[i]->run();
    }

    started = true;
    return 0;
}

int VisualizerCli::stop() {
    for(int i = 0; i < dataLoaders.size(); i++) {
        dataLoaders[i]->join();
        logger->info("Clipboard for FE with ID {}: size {} / {}", i, clipboards[i]->getNumDataIn(), clipboards[i]->size());
        dataLoaders[i].reset();
        while(clipboards[i]->size() > 0) {
            auto raw = clipboards[i]->popData();
            raw.reset();
        }   
        clipboards[i].reset();
    }
    return 0;
}

void VisualizerCli::listFEs() {
    for(int i = 0; i < dataLoaders.size(); i++) {
        
        auto temp = getConfig(i);

        logger->info("[{}]: FE with ID {}", names[i], i);
        logger->info("[{}]:  - Clipboard I/O sizes {}/{}", names[i], clipboards[i]->getNumDataIn(), clipboards[i]->getNumDataOut());
        
        // std::vector<int> position = temp["position"].get<std::vector<int>>();
        // std::vector<int> angle = temp["angle"].get<std::vector<int>>();

        logger->info("[{}]:  - Position/angle vectors: {} / {}", names[i], 
            temp["position"].dump(), temp["angle"].dump()
        );
    }
}

const json& VisualizerCli::getConfig(int fe_id) const{
    if(!(fe_id >= 0 && fe_id < clipboards.size())) {
        logger->error("No frontend with index {} found in list! Returned config is empty", fe_id);
        throw std::invalid_argument("Invalid frontend config requested!");
    }
    return config["sources"][configIdMap[fe_id]];
}

const json& VisualizerCli::getConfig(std::string fe_id) const{
    if(feIdMap.find(fe_id) == feIdMap.end()) {
        logger->error("No frontend with name {} found in list! Returned config is empty", fe_id);
        throw std::invalid_argument("Invalid frontend config requested!");
    }
    return config["sources"][configIdMap[feIdMap.at(fe_id)]];
}

std::unique_ptr<EventData> VisualizerCli::getRawData(int fe_id) const{

    if(!(fe_id >= 0 && fe_id < clipboards.size())) {
        logger->error("No frontend with index {} found in list! Returned data is nullptr", fe_id);
        return nullptr;
    }
    return clipboards[fe_id]->popData();
}

std::unique_ptr<EventData> VisualizerCli::getRawData(std::string fe_id)const{
    if(feIdMap.find(fe_id) == feIdMap.end()) {
        logger->error("No frontend with name {} found in list! Returned data is nullptr", fe_id);
        return nullptr;
    }
    return getRawData(feIdMap.at(fe_id));
}

std::unique_ptr<std::vector<Event>> VisualizerCli::loadEvents(int fe_id, bool get_all) const{
    auto result = std::make_unique<std::vector<Event>>();
    // clipboards[fe_id].size();
    uint32_t procLoop = 1;
    std::unique_ptr<EventData> proc;
    do {
        std::cout << "procLoop: " <<  procLoop << std::endl;
        procLoop++;
        if(proc) proc.reset();
        proc = getRawData(fe_id);
        if(proc) {
            result = std::make_unique<std::vector<Event>>(std::move(proc->events));
        }
    } 
    while(proc && get_all);

    return result;
}

std::unique_ptr<std::vector<Event>> VisualizerCli::loadEvents(std::string fe_id, bool get_all) const{
    if(feIdMap.find(fe_id) == feIdMap.end()) {
        logger->error("No frontend with name {} found in list! Returned data is nullptr", fe_id);
        return nullptr;
    }
    return loadEvents(feIdMap.at(fe_id), get_all);
}

std::unique_ptr<std::vector<pixelHit>> VisualizerCli::getData(int fe_id, bool get_all) const{
    if(!(state == CLIstate::NORMAL))
        return nullptr;

    auto result = std::make_unique<std::vector<pixelHit>>();

    auto events = loadEvents(fe_id, get_all);
    for(int i = 0; i < events->size(); i++) {
        for(int j = 0; j < (*events)[i].hits.size(); j++) {
            result->push_back({(uint16_t)(*events)[i].hits[j].row, (uint16_t)(*events)[i].hits[j].col});
        }
    }

    return result;
}

std::unique_ptr<std::vector<pixelHit>> VisualizerCli::getData(std::string fe_id, bool get_all) const{
    if(!(state == CLIstate::NORMAL))
        return nullptr;

    if(feIdMap.find(fe_id) == feIdMap.end()) {
        logger->error("No frontend with name {} found in list! Returned data is nullptr", fe_id);
        return nullptr;
    }
    return getData(feIdMap.at(fe_id), get_all);
}

std::unique_ptr<std::vector<Event>> VisualizerCli::getEvents(int fe_id, bool get_all) const{
    if(!(state == CLIstate::NORMAL))
        return nullptr;

    return loadEvents(fe_id, get_all);
}

std::unique_ptr<std::vector<Event>> VisualizerCli::getEvents(std::string fe_id, bool get_all) const{
    if(!(state == CLIstate::NORMAL))
        return nullptr;

    if(feIdMap.find(fe_id) == feIdMap.end()) {
        logger->error("No frontend with name {} found in list! Returned data is nullptr", fe_id);
        return nullptr;
    }
    return getEvents(feIdMap.at(fe_id), get_all);
}

std::unique_ptr<std::vector<ReconstructedBunch>> VisualizerCli::getReconstructedBunch(){
    numloops++;

    uint8_t totalFEs = clipboards.size();
    uint32_t largest_bcid = 0;
    uint32_t smallestTail_bcid = UINT32_MAX;
    std::vector<std::unique_ptr<EventData>> loadersEventData(totalFEs);
    std::vector<std::vector<ReconstructedBunch>> loadersReconBunch(totalFEs);
    
    bool noEventsOccured = true;
    for(int i = 0; i < totalFEs; i++){
        loadersEventData[i] = getRawData(i);
        if(loadersEventData[i] && !loadersEventData[i]->empty()){

            uint32_t first_bcid = loadersEventData[i]->events.front().bcid;
            uint32_t last_bcid = loadersEventData[i]->events.back().bcid;

            largest_bcid = std::max(largest_bcid, last_bcid);
            smallestTail_bcid = std::min(smallestTail_bcid, last_bcid);

            for(uint32_t bcid = first_bcid; bcid <= last_bcid; bcid++){
                ReconstructedBunch tempBunch(bcid, totalFEs);
                tempBunch.addEventDataCI(*loadersEventData[i], i); //bcidChangeIndex should be defined in data loaders
                loadersReconBunch[i].push_back(std::move(tempBunch));
            }

            noEventsOccured = false;
        }
        // if(!temp[i]->empty()){
        //     std::cout << "ID: " << numloops << " // Loader " << i << " with // front_bcid: " <<  temp[i]->front().bcid << " and back_bcid: " << temp[i]->back().bcid  << "Size: " << temp[i]->size() << std::endl;
        //     std::cout << " |------------------------> front_hits: " << temp[i]->front().nHits << " and back_hits: " << temp[i]->back().nHits << std::endl;
        // }
    }
    if(noEventsOccured)
        return nullptr;

    std::cout << "ID: " << numloops << std::endl;

    // std::vector<std::unique_ptr<std::vector<Event>>> temp(totalFEs);
    // std::cout << "ID: " << numloops << " Here 1\n";
    // //Determine the overlapped events
    // for(int i = 0; i < temp.size(); i++){
    //     temp[i] = loadEvents(i);

    //     if(!temp[i]->empty()){
    //         largest_bcid = std::max(largest_bcid, temp[i]->back().bcid);
    //         smallestTail_bcid = std::min(smallestTail_bcid, temp[i]->back().bcid);
    //         std::cout << "Loader " << i << " with // front_bcid: " <<  temp[i]->front().bcid << " and back_bcid: " << temp[i]->back().bcid  << std::endl;
    //         noEventsOccured = false;
    //     }
    // }
    // if(noEventsOccured)
    //     return nullptr;


    std::cout << "-----largest_bcid: " << largest_bcid << std::endl;
    std::cout << "-----smallestTail_bcid:  " << smallestTail_bcid << std::endl; 
    std::cout << "-----firstuncompleted_bcid: " << firstuncompleted_bcid << std::endl;
    std::cout << "-----firstuntouched_bcid: " << firstuntouched_bcid << std::endl;
    
    std::cout << "ID: " << numloops << " Here 2\n";

    //Initialize new reconstructed events with appropriate bcid
    uncompletedReconEvents->reserve(smallestTail_bcid - firstuncompleted_bcid + 1);
    for(uint32_t bcid = firstuntouched_bcid; bcid <= smallestTail_bcid; bcid++){
        uncompletedReconEvents->push_back(ReconstructedBunch(bcid, totalFEs));
    }

    std::cout << "ID: " << numloops << " Here 3\n";

    //Fill in completed events to return. Completed evnts are the events with bcid less than the smallest tail bcid.
    for(int i = 0; i < totalFEs; i++){
        if(loadersEventData[i] && !loadersEventData[i]->empty()){
            uint32_t first_bcid = loadersReconBunch[i].front().bcid;
            size_t globalIndex = first_bcid - firstuncompleted_bcid;
            for(uint32_t bcid = first_bcid; bcid < smallestTail_bcid; bcid++){
                size_t index = bcid - first_bcid;
                (*uncompletedReconEvents)[globalIndex + index].addEventData(*((loadersReconBunch[i])[index].getEventDataFE(i)), i);
            }
        }
    }
    std::unique_ptr<std::vector<ReconstructedBunch>> reconstructedReturn = std::move(uncompletedReconEvents);

    std::cout << "ID: " << numloops << " Here 4\n";

    //Reset variables to load in the uncompleted events
    firstuncompleted_bcid = smallestTail_bcid;
    firstuntouched_bcid = largest_bcid + 1;

    uncompletedReconEvents = std::make_unique<std::vector<ReconstructedBunch>>();
    uncompletedReconEvents->empty();
    uncompletedReconEvents->reserve(firstuntouched_bcid - firstuncompleted_bcid + 1);
    for(uint32_t bcid = firstuncompleted_bcid; bcid < firstuntouched_bcid; bcid++){
        uncompletedReconEvents->push_back(ReconstructedBunch(bcid, totalFEs));
    }
    
    std::cout << "ID: " << numloops << " Here 5\n";

    std::cout << "Uncompleted Recon Events Size: " << uncompletedReconEvents->size() << std::endl;

    //Fill up uncompletedReconEvents with new uncompleted events
    for(int i = 0; i < loadersReconBunch.size(); i++){
        if(loadersEventData[i] && !loadersEventData[i]->empty()){
            size_t globalIndex = firstuncompleted_bcid - loadersReconBunch[i].front().bcid;
            std::cout << "global index " << globalIndex << std::endl;
            std::cout << "front bcid of the " << i << "th loader: " << loadersReconBunch[i].front().bcid << std::endl;
            std::cout << "back bcid of the " << i << "th loader: " << loadersReconBunch[i].back().bcid << std::endl;

            for(uint32_t bcid = firstuncompleted_bcid; bcid <= loadersReconBunch[i].back().bcid; bcid++){
                size_t index = bcid - firstuncompleted_bcid;

                std::cout << "Loaders Recon Bunch: " << i << " size: " << loadersReconBunch[i].size() << std::endl;
                std::cout << "index: " << index << std::endl;
                (*uncompletedReconEvents)[index].addEventData(*((loadersReconBunch[i])[globalIndex + index].getEventDataFE(i)), i);
            }
        }
    }

    std::cout << "ID: " << numloops << " Here 6\n";

    std::cout << "Actual nHits: " << nHits << std::endl;
    return std::move(reconstructedReturn);
}

