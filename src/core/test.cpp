#include "DataBase.h"
#include "YarrBinary.h"
#include <iostream>
#include "cli.h"

int main(int argc, char** argv) {
    std::cout << "test main start" << std::endl;

    // option parsing
    ScanOpts options;
    int ret = parseOptions(argc, argv, options);
    if (ret != 1) {
        printHelp();
        return ret;
    }

    YarrBinary yb;
    json j;

    std::unique_ptr<ClipBoard<EventData>> cb = std::make_unique<ClipBoard<EventData>>();

    yb.init();
    yb.connect(cb.get());
    yb.configure(j);
    yb.run();
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    yb.join();

    std::cout << "Data size: " << cb->getNumDataIn() << ", " << cb->size() << std::endl;
    std::cout << "test main end" << std::endl;

    return 0;
}