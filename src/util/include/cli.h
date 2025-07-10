
#ifndef _CLI_H_
#define _CLI_H_

// #################################################
// # Author: Luc Le Pottier, Koji Abel             #
// # Email: luclepot / kabel at lbl.gov            #
// # Project: YARR-event-visualizer                #
// # Description: CLI for data reading interfaces  #
// #################################################

#include <vector>
#include <iostream>
#include <algorithm>
#include <getopt.h>
#include <fstream>
#include <map>

#include "logging.h"
#include "AllDataLoaders.h"
#include "DataBase.h"
#include "util/include/FEBookie.h"
#include "datasets/include/EventReconstructor.h"
#include "datasets/include/CorryWrapper.h"

enum CLIstate {
    //No batching of events is done. User gets most recent events.
    //std::unique_ptr<Track> of the returned pair in getEventBatch() is always nullptr.
    FAST,
    
    //User gets ownership of the most recently batched events.
    //std::unique_ptr<Track> of the returned pair in getEventBatch() is always nullptr.
    BATCH,

    //User gets ownership of the most recently batched events.
    //std::unique_ptr<Track> of the returned pair in getEventBatch() corresponds to the reconstructed
    //                       of the batch of events.
    TRACKS
};


namespace cli_helpers {
    extern std::shared_ptr<spdlog::logger> logger;
    
    struct ScanOpts {
        // std::string loggingPattern = "[%T:%e]%^[%=8l][%=15n][%t]:%$ %v";
        std::string configPath;
        std::string commandLineStr;
        std::string progName;
        bool verbose;
    };

    json openJsonFile(const std::string& filepath);

    void setupLoggers(bool verbose);
}


class VisualizerCli {
    public:
        VisualizerCli();
        ~VisualizerCli();

        // Runtime usage
        int init(int argc, char** argv);
        int configure();
        int start();
        int stop();

        // Config passing
        void listFEs();
        const json& getConfig(int fe_id) const;
        const json& getConfig(std::string fe_id) const;
        const json& getMasterConfig() {return config;}
        bool setState(CLIstate state); //Initializes and sets up dataloaders for respective state

        std::pair<std::unique_ptr<FEEvents>, std::unique_ptr<TrackData>> getEventBatch();

        std::unique_ptr<std::vector<pixelHit>> getData(int fe_id, bool get_all=false) const;
        std::unique_ptr<std::vector<pixelHit>> getData(std::string fe_id, bool get_all=false) const;

        std::unique_ptr<std::vector<Event>> getEvents(int fe_id, bool get_all=false) const;
        std::unique_ptr<std::vector<Event>> getEvents(std::string fe_id, bool get_all=false) const;

        size_t  getTotalFEs() const {return clipboards.size();}
        bool isRunning() const { return started; }

    private:
        int parseOptions(int argc, char *argv[]);
        void printHelp();

        bool started = false;
        bool dataLoadersConnected = false;

        uint32_t nHits = 0;
        uint8_t totalFEs;

        std::unique_ptr<EventData> getRawData(int fe_id) const;
        std::unique_ptr<EventData> getRawData(std::string fe_id) const;

        std::unique_ptr<std::vector<Event>> loadEvents(int fe_id, bool get_all = false) const;
        std::unique_ptr<std::vector<Event>> loadEvents(std::string fe_id, bool get_all = false) const;
        
        cli_helpers::ScanOpts scanOpts;
        
        json config;
        std::vector<std::unique_ptr<DataLoader>> dataLoaders;
        std::vector<std::shared_ptr<ClipBoard<EventData>>> clipboards;
        std::unique_ptr<EventReconstructor> eventReconstructor;
        std::unique_ptr<CorryWrapper> trackReconstructor;
        std::map<std::string, int> feIdMap;
        std::vector<int> configIdMap;
        std::vector<std::string> names;

        CLIstate state;
};

#endif