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

bool showdemowin = true;

void initGLFW();
void initImGUI(GLFWwindow* window);
void shutdownImGUI();
GLFWwindow* createOpenGLContext(int width, int height);
void frame(GLFWwindow* window);
static void GLFWErrorCallback(int err, const char* message);

void createFramebuffer();

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
    GLFWwindow* window = createOpenGLContext(glfwGetVideoMode(glfwGetPrimaryMonitor())->width, glfwGetVideoMode(glfwGetPrimaryMonitor())->height);
    initImGUI(window);

    
    while(!glfwWindowShouldClose(window)){
        frame(window);
        logger->debug("This is a frame");
    }
    shutdownImGUI();
    glfwTerminate();
    logger->info("Terminated window");


    return 0;
}

void initGLFW(){
    glfwSetErrorCallback(GLFWErrorCallback);
    if(!glfwInit()){ 
        logger->error("GLFW initialization failed");
        return;
    }
    logger->info("GLFW Initialized");
}

void initImGUI(GLFWwindow* window){
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init();

    ImGui::StyleColorsDark();
}

void shutdownImGUI(){
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
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
    //glfwSwapInterval(1); //Enable vsync

    return window;
}


void frame(GLFWwindow* window){
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();

    glClear(GL_COLOR_BUFFER_BIT);
    
    ImGui::NewFrame();
    {
        ImGui::Begin("New Window");
        ImGui::Text("Hello World");
        ImGui::End();
    }
    ImGui::ShowDemoWindow();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    
    glfwPollEvents();
    glfwSwapBuffers(window);
}

static void GLFWErrorCallback(int err, const char* message){
    logger->error("GLFW Code {0}: {1}", err, message);
}
