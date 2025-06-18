
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

enum CLIstate {
    //User gets ownership of any chip events
    //No reconstruction of events is done by the CLI.
    //Gives ownership of any data off any chip to user when getData or getEvents is called.
    //getReconstructedData and getReconstructedEvents returns nullptr.
    NORMAL,
    
    //User gets ownership of a batch of th emost recent completed reconstructed event
    //CLI reconstructs events based off of BCID (events with same BCID are placed in the same ReconstructedEvent type)
    //Gives ownership of only reconstructed events when getReconstructedData or getReconstructedEvents is called.
    //getData and getEvents returns nullptr
    RECONSTRUCT
};

class VisualizerCli {
    public:
        VisualizerCli();
        ~VisualizerCli();

        CLIstate state = CLIstate::NORMAL;

        // Runtime usage
        int init(int argc, char** argv, CLIstate argstate = CLIstate::NORMAL);
        int configure();
        int start();
        int stop();

        // Config passing
        void listFEs();
        const json& getConfig(int fe_id) const;
        const json& getConfig(std::string fe_id) const;
        const json& getMasterConfig() {return config;}

        std::unique_ptr<std::vector<pixelHit>> getData(int fe_id, bool get_all=false) const;
        std::unique_ptr<std::vector<pixelHit>> getData(std::string fe_id, bool get_all=false) const;

        std::unique_ptr<std::vector<Event>> getEvents(int fe_id, bool get_all=false) const;
        std::unique_ptr<std::vector<Event>> getEvents(std::string fe_id, bool get_all=false) const;

        std::unique_ptr<std::vector<ReconstructedBunch>> getReconstructedBunch();

        // std::vector<std::vector<int>> getProcessedData(int fe_id); // row, column for all hits in the EventData object
        // row col
        // row col
        // row col
        // 
        // # batches
        // # events
        // getSingleBatch();

        size_t  getTotalFEs() const {return clipboards.size();}
        bool isRunning() const { return started; }

    private:
        int parseOptions(int argc, char *argv[]);
        void printHelp();

        bool started = false;
        bool firstTime = true;

        uint32_t nHits = 0;

        std::unique_ptr<EventData> getRawData(int fe_id) const;
        std::unique_ptr<EventData> getRawData(std::string fe_id) const;

        std::unique_ptr<std::vector<Event>> loadEvents(int fe_id, bool get_all = false) const;
        std::unique_ptr<std::vector<Event>> loadEvents(std::string fe_id, bool get_all = false) const;
        
        cli_helpers::ScanOpts scanOpts;
        
        json config;
        std::vector<std::unique_ptr<DataLoader>> dataLoaders;
        std::vector<std::shared_ptr<ClipBoard<EventData>>> clipboards;
        std::map<std::string, int> feIdMap;
        std::vector<int> configIdMap;
        std::vector<std::string> names;


        std::unique_ptr<std::vector<ReconstructedBunch>> uncompletedReconEvents; //Buffer containing uncompleted reconstructed events from the last getReconstructedEvents() call
        std::vector<uint32_t> curr_fe_bcid;
        uint32_t firstuncompleted_bcid = 0,  firstuntouched_bcid = 0; //Smallest bcid of reconstructed event that is in the process of reconstruction, smallest bcid of first reconstructed event that has


        uint16_t numloops = 0;
};

#endif