
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
#include <getopt.h>
#include <fstream>
#include <map>

#include "logging.h"
#include "AllDataLoaders.h"
#include "DataBase.h"

namespace cli_helpers {
    auto logger = logging::make_log("VizCLI");
    
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

struct pixelHit {
    uint16_t row;
    uint16_t col;
};

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
        const json& getConfig(int fe_id);
        const json& getConfig(std::string fe_id);
        const json& getMasterConfig() {return config;}

        std::unique_ptr<EventData> getRawData(int fe_id);
        std::unique_ptr<EventData> getRawData(std::string fe_id);

        std::unique_ptr<std::vector<pixelHit>> getData(int fe_id, bool get_all=false);
        std::unique_ptr<std::vector<pixelHit>> getData(std::string fe_id, bool get_all=false);

        // std::vector<std::vector<int>> getProcessedData(int fe_id); // row, column for all hits in the EventData object
        // row col
        // row col
        // row col
        // 
        // # batches
        // # events
        // getSingleBatch();

        size_t getSize() {return dataLoaders.size();}

    private:
        int parseOptions(int argc, char *argv[]);
        void printHelp();
        
        cli_helpers::ScanOpts scanOpts;
        
        json config;
        std::vector<std::unique_ptr<DataLoader>> dataLoaders;
        std::vector<std::shared_ptr<ClipBoard<EventData>>> clipboards;
        std::map<std::string, int> feIdMap;
        std::vector<int> configIdMap;
        std::vector<std::string> names;
};

#endif