#include <iostream>
#include <string>
#include <csignal>

#include "cli.h"

namespace
{
    auto logger = logging::make_log("MainLoop");
}

sig_atomic_t signaled = 0;
int main(int argc, char** argv) {

    int cli_status;
    VisualizerCli cli;

    cli_status = cli.init(argc, argv); if(cli_status <= 0) return cli_status;
    cli_status = cli.configure(); if(cli_status < 0) return cli_status;

    // Printout of FE statuses
    cli.listFEs();

    // Start CLI fes
    cli_status = cli.start(); if(cli_status < 0) return cli_status;

    signal(SIGINT, [](int signum){signaled = 1;});
    signal(SIGTERM, [](int signum){signaled = 1;});
    signal(SIGSTOP, [](int signum){signaled = 1;});

    // while we don't see CTRL-C or SIGUSR1 
    while(signaled == 0) {

        // MAIN EVENT LOOP HERE
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        // collect and count data
        long int size = 0;
        int nfe= 0;
        for(int i = 0; i < cli.getSize(); i++) {
            auto data = cli.getData(i);
            if(data){
                size += data->size();
                data.reset();
                nfe++;
            }
        }
        if(size > 0)
            logger->debug("Total hit event size for this frame, with {} FEs: {}", nfe, size);
    }
    
    std::cout << "\r";
    logger->info("Caught interrupt, stopping threads");
    
    cli_status = cli.stop(); if(cli_status < 0) return cli_status;
    logger->info("End run");
    return 0;
}
