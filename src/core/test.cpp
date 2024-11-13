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
    auto logger = logging::make_log("MainLoop");
}

bool showdemowin = true;


struct FramebufferData{
    GLuint FBO, RBO, texture_id;
    float m_width, m_height;
};

//GLFW and GLAD things
void initGLFW();
GLFWwindow* createOpenGLContext(int width, int height);
static void GLFWErrorCallback(int err, const char* message);
FramebufferData createFramebuffer(float width, float height);
void resizeFramebuffer(FramebufferData& data, float width, float height);


//Dockspace functions
void initImGUI(GLFWwindow* window);
void shutdownImGUI();
void dockspaceSetup();
void dockspaceInit();
void dockspaceFrame();

//Frame
void frame(GLFWwindow* window);

//Scene View
FramebufferData sceneFramebuffer;
void createSceneView();
void renderSceneView();

float color[4] = {0.0f, 0.0f, 0.0f, 1.0f};

int main(int argc, char** argv) {

    VisualizerCli cli;
    // option parsing
    cli.init(argc, argv);

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
            ImGuiID dock_id_right, dock_id_down;
            
            ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Right, 0.2f, &dock_id_right, &dockspace_id);
            ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Down, 0.25f, &dock_id_down, &dockspace_id);

            // we now dock our windows into the docking node we made above
            ImGui::DockBuilderDockWindow("Right", dock_id_right);
            ImGui::DockBuilderDockWindow("Down", dock_id_down);
            ImGui::DockBuilderDockWindow("Scene", dockspace_id);
            ImGui::DockBuilderFinish(dockspace_id);
        }
    }

    {
        ImGui::Begin("Right");
        ImGui::Text("Hello World");
        ImGui::ColorPicker4("Change Screen", color);
        ImGui::End();
    }

    {
        ImGui::Begin("Down");
        ImGui::Text("This is a down window hopefully");
        ImGui::End();
    }

    {
        createSceneView();
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

    {   //Scene window
        renderSceneView();
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwPollEvents();
    glfwSwapBuffers(window);
    glClear(GL_COLOR_BUFFER_BIT);
}

static void GLFWErrorCallback(int err, const char* message){
    logger->error("GLFW Code {0}: {1}", err, message);
}


FramebufferData createFramebuffer(float width, float height){
    GLuint FBO, RBO, texture_id;
    glGenFramebuffers(1, &FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);

	glGenTextures(1, &texture_id);
	glBindTexture(GL_TEXTURE_2D, texture_id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture_id, 0);

	glGenRenderbuffers(1, &RBO);
	glBindRenderbuffer(GL_RENDERBUFFER, RBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!\n";

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

    return FramebufferData{FBO, RBO, texture_id, width, height};
}


//This binds the texture 
void resizeFramebuffer(FramebufferData& data, float width, float height){
    glBindTexture(GL_TEXTURE_2D, data.texture_id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, data.texture_id, 0);

	glBindRenderbuffer(GL_RENDERBUFFER, data.RBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, data.RBO);

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    data.m_width = width;
    data.m_height = height;
}

void createSceneView(){
    ImGui::Begin("Scene");
    sceneFramebuffer = createFramebuffer(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y);
    ImGui::End();
}

void renderSceneView(){
    GLint prevViewport[4];
    glGetIntegerv(GL_VIEWPORT, prevViewport);

    glBindFramebuffer(GL_FRAMEBUFFER, sceneFramebuffer.FBO);
        glClearColor(color[0], color[1], color[2], color[3]);
        glClear(GL_COLOR_BUFFER_BIT);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    ImGui::Begin("Scene");
        ImVec2 windim = ImGui::GetContentRegionAvail();
        if(windim.x != sceneFramebuffer.m_width || windim.y != sceneFramebuffer.m_height){
            resizeFramebuffer(sceneFramebuffer, windim.x, windim.y);
        }
        glViewport(0, 0, windim.x, windim.y);

        ImVec2 bottomRight = ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowSize().x, ImGui::GetWindowPos().y + ImGui::GetWindowSize().y);
        ImGui::GetWindowDrawList()->AddImage(sceneFramebuffer.texture_id, ImGui::GetCursorPos(), bottomRight, ImVec2(0, 1), ImVec2(1, 0));
    ImGui::End();

    glViewport(prevViewport[0], prevViewport[1], prevViewport[2], prevViewport[3]);
}