#include "DataBase.h"
#include "YarrBinary.h"
#include <iostream>
#include "cli.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace
{
    auto logger = logging::make_log("VisualizerCLI");
}


int main(int argc, char** argv) {

    // option parsing
    ScanOpts options;
    int ret = parseOptions(argc, argv, options);
    
    setupLoggers();

    if (ret != 1) {
        printHelp();
        return ret;
    }

    logging::banner(logger, "Visualizer CLI Program");

    YarrBinary yb;
    json j;

    std::unique_ptr<ClipBoard<EventData>> cb = std::make_unique<ClipBoard<EventData>>();

    yb.init();
    yb.connect(cb.get());
    yb.configure(j);
    yb.run();
    // std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    yb.join();

    logger->info("Data size: {} / {}", cb->getNumDataIn(), cb->size());
    logger->info("test main end");

    glfwInit();
    GLFWwindow* window = glfwCreateWindow(640, 480, "My Title", NULL, NULL);
    if (window)
        std::this_thread::sleep_for(std::chrono::milliseconds(10000));
    else 
        logger->error("Window did not work");
    glfwDestroyWindow(window);

    return 0;
}