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

    cli_status = cli.init(argc, argv);
    cli_status = cli.configure();
    cli_status = cli.start();

    signal(SIGINT, [](int signum){signaled = 1;});
    signal(SIGUSR1, [](int signum){signaled = 1;});

    while(signaled == 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    std::cout << "\r";
    logger->info("Caught interrupt, stopping threads");
    
    cli_status = cli.stop();
    logger->info("End run");
    return 0;
}
