
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
#include "logging.h"
#include <fstream>

namespace viz_cli {
    struct ScanOpts {
        // std::string loggingPattern = "[%T:%e]%^[%=8l][%=15n][%t]:%$ %v";
        std::string configPath;
        std::string commandLineStr;
        std::string progName;
        bool verbose;
    };

    void setupLoggers(bool verbose);

    void printHelp(); 

    int parseOptions(int argc, char *argv[], ScanOpts &scanOpts);
    
    json openJsonFile(const std::string& filepath);
}

#endif