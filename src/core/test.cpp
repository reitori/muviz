#include "DataBase.h"
#include "YarrBinaryFile.h"
#include <iostream>
#include "cli.h"

#include "imgui.h"
#include "imgui_internal.h"
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
void dockspaceSetup();
void dockspaceInit();
void dockspaceFrame();
void frame(GLFWwindow* window);
static void GLFWErrorCallback(int err, const char* message);

void createFramebuffer();

float color[4] = {0.0f, 0.0f, 0.0f, 1.0f};

int main(int argc, char** argv) {

    // option parsing
    viz_cli::ScanOpts options;
    int ret = viz_cli::parseOptions(argc, argv, options);
    
    viz_cli::setupLoggers(false);

    if (ret != 1) {
        viz_cli::printHelp();
        return ret;
    }

    logging::banner(logger, "Visualizer CLI Program");

    YarrBinaryFile yb;
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

    dockspaceInit();
    glfwPollEvents();
    glfwSwapBuffers(window);
    glClear(GL_COLOR_BUFFER_BIT);

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
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

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

void dockspaceSetup(){
    static bool opt_fullscreen = true;
    static bool opt_padding = false;
    static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
        if (opt_fullscreen)
    {
        // If so, get the main viewport:
        const ImGuiViewport* viewport = ImGui::GetMainViewport();

        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

        if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
            window_flags |= ImGuiWindowFlags_NoBackground;


    }
    
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("DockSpace", nullptr, window_flags);
    ImGui::PopStyleVar();
    ImGui::PopStyleVar(2);
}

void dockspaceInit(){
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    dockspaceSetup();

    const ImGuiViewport* viewport = ImGui::GetMainViewport();

    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
    {
        ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);


        {
            ImGui::DockBuilderRemoveNode(dockspace_id); // clear any previous layout
            ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_None | ImGuiDockNodeFlags_DockSpace);
            ImGui::DockBuilderSetNodeSize(dockspace_id, viewport->Size);

            // split the dockspace into 2 nodes -- DockBuilderSplitNode takes in the following args in the following order
            //   window ID to split, direction, fraction (between 0 and 1), the final two setting let's us choose which id we want (which ever one we DON'T set as NULL, will be returned by the function)
            //                                                              out_id_at_dir is the id of the node in the direction we specified earlier, out_id_at_opposite_dir is in the opposite direction
            auto dock_id_right = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Right, 0.2f, nullptr, &dockspace_id);
            auto dock_id_down = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Down, 0.25f, nullptr, &dockspace_id);

            // we now dock our windows into the docking node we made above
            ImGui::DockBuilderDockWindow("Right", dock_id_right);
            ImGui::DockBuilderDockWindow("Down", dock_id_down);
            ImGui::DockBuilderFinish(dockspace_id);
        }
    }

    {
        ImGui::Begin("Right");
        ImGui::Text("Hello World");
        if(ImGui::ColorPicker4("Change Screen", color)){
            glClearColor(color[0], color[1], color[2], color[3]);
        }
        ImGui::End();
    }

    {
        ImGui::Begin("Down");
        ImGui::Text("This is a down window hopefully");
        ImGui::End();
    }

    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void dockspaceFrame(){
    dockspaceSetup();
    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
    {
        ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);
    }
    ImGui::End();
}

void frame(GLFWwindow* window){
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    dockspaceFrame();

    {
        ImGui::Begin("Right");
        ImGui::Text("Hello World");
        if(ImGui::ColorPicker4("Change Screen", color)){
            glClearColor(color[0], color[1], color[2], color[3]);
        }
        ImGui::End();
    }

    {
        ImGui::Begin("Down");
        ImGui::Text("This is a down window hopefully");
        ImGui::End();
    }
    ImGui::ShowDemoWindow();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwPollEvents();
    glfwSwapBuffers(window);
    glClear(GL_COLOR_BUFFER_BIT);
}

static void GLFWErrorCallback(int err, const char* message){
    logger->error("GLFW Code {0}: {1}", err, message);
}
