
#ifndef _CLI_H_
#define _CLI_H_

// #################################################
// # Author: Luc Le Pottier                        #
// # Email: luclepot@lbl.gov                       #
// # Project: YARR-event-visualizer                #
// # Description: CLI for data reading interfaces  #
// #################################################

#include <vector>
#include <iostream>
#include <getopt.h>
#include <fstream>

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

class VisualizerCli {
    public:
        VisualizerCli();
        ~VisualizerCli() = default;

        int init(int argc, char** argv);
        int configure();
        int start();
        int stop();

    private:
        int parseOptions(int argc, char *argv[]);
        void printHelp();
        
        cli_helpers::ScanOpts scanOpts;
        
        json config;
        std::vector<std::unique_ptr<DataLoader>> dataLoaders;
        std::vector<std::unique_ptr<ClipBoard<EventData>>> clipboards;
};

#endif