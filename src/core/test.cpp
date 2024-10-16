#include "DataBase.h"
#include "YarrBinary.h"
#include <iostream>
#include "cli.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace
{
    auto logger = logging::make_log("VisualizerCLI");
}

void initGLFW();
GLFWwindow* createOpenGLContext(int width, int height);
void frame(GLFWwindow* window);
static void GLFWErrorCallback(int err, const char* message);

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

    initGLFW();
    GLFWwindow* window = createOpenGLContext(800, 800);
    while(!glfwWindowShouldClose(window)){
        frame(window);
        logger->debug("This is a frame");
    }
    glfwTerminate();
    logger->info("Terminated window");

    return 0;
}

void initGLFW(){
    if(!glfwInit()){ 
        logger->error("GLFW initialization failed");
        return;
    }
    logger->info("GLFW Initialized");
}

GLFWwindow* createOpenGLContext(int width, int height){
    GLFWwindow* window = glfwCreateWindow(width, height, "Visualizer", NULL, NULL);
    if(window == NULL){
        logger->error("Failed to create GLFW window");
        glfwTerminate();
        return NULL;
    }
    glfwMakeContextCurrent(window);

    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
        logger->error("Failed to initailize GLAD");
        return NULL;
    }
    glViewport(0, 0, width, height);

    return window;
}

void frame(GLFWwindow* window){
    float t = glfwGetTime();
    glClearColor(sin(t), cos(t), 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glfwSwapBuffers(window);
    glfwPollEvents();
}

static void GLFWErrorCallback(int err, const char* message){
    logger->error("GLFW Code {0}: {1}", err, message);
}