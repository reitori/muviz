#include "DataBase.h"
#include "YarrBinary.h"
#include <iostream>

int main(int argc, char** argv) {
    std::cout << "test main start" << std::endl;
    YarrBinary yb;
    json j;

    std::unique_ptr<ClipBoard<EventData>> cb = std::make_unique<ClipBoard<EventData>>();

    yb.init();
    yb.connect(cb.get());
    yb.configure(j);
    yb.run();
    yb.join();

    ClipBoard<std::vector<Event>> events;
    std::cout << "test main end" << std::endl;

    return 0;
}