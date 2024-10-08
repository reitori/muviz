#ifndef _CLI_H_
#define _CLI_H_
#include <vector>
#include <iostream>
#include <getopt.h>
#include "logging.h"

struct ScanOpts {
    // std::string loggingPattern = "[%T:%e]%^[%=8l][%=15n][%t]:%$ %v";
    std::string configPath;
    std::string commandLineStr;
    std::string progName;
    // std::string outputDir = "./data/";
    // std::string ctrlCfgPath;
    // bool doPlots = false;
    // int target_charge{-1};
    // int target_tot{-1};
    // int mask_opt{-1};
    // bool dbUse = false;
    // bool doOutput = true;
    // std::string logCfgPath;
    // std::string dbCfgPath ;
    // std::string dbSiteCfgPath;
    // std::string dbUserCfgPath;
    // std::string dbTag;
    // bool setQCMode = false;
    // bool setInteractiveMode = false;
    // bool doResetBeforeScan = true;
    // bool makeGraph = false;
};

void setupLoggers() {
    json loggerConfig;
    loggerConfig["pattern"] = "[%T:%e]%^[%=8l][%=15n][%t]:%$ %v";
    loggerConfig["log_config"][0]["name"] = "all";
    loggerConfig["log_config"][0]["level"] = "info";
    loggerConfig["outputDir"] = "";

    logging::setupLoggers(loggerConfig);
}

void printHelp() {

    spdlog::info("Help:");
    spdlog::info(" -h: Shows this.");
    spdlog::info(" -c <config.json> Provide data reader configuration.");
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

int parseOptions(int argc, char *argv[], ScanOpts &scanOpts) {
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
    while (true) {
        int opt_index=0;
        c = getopt_long(argc, argv, "hc:", long_options, &opt_index);
        int count = 0;
        if(c == -1) break;
        switch (c) {
            case 'h':
                printHelp();
                return 0;
            case 'c':
                scanOpts.configPath = std::string(optarg);
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

#endif