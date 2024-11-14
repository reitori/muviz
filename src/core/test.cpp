#include "DataBase.h"
#include "YarrBinary.h"
#include <iostream>
#include "cli.h"

#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "core/header.h"
#include "OpenGL/Shader.h"

#include <csignal>

std::shared_ptr<spdlog::logger> viz::m_appLogger = logging::make_log("VisualizerCLI");

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

//Global ImGUI window variables to change scene
float color[4] = {0.0f, 0.0f, 0.0f, 1.0f};

//-----------------------------------------------//
//                    SHADERS                    //
//-----------------------------------------------//
const char* vertexShaderSource="#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"

    "uniform mat4 model;"
    "uniform mat4 view;"
    "uniform mat4 proj;"

    "void main()\n"
    "{\n"
    "   gl_Position = proj * view * model * vec4(aPos, 1.0);\n"
    "}\0";

const char* fragmentShaderSource="#version 330 core\n"
    "out vec4 FragColor;\n"

    "void main()\n"
    "{\n"
    "    FragColor = vec4(0.3921f, 0.3921f, 0.3921f, 1.0f);\n"
    "}\0";

 GLfloat cube_vertices[] = {
        // front
        -1.0, -1.0,  1.0,
         1.0, -1.0,  1.0,
         1.0,  1.0,  1.0,
        -1.0,  1.0,  1.0,
        // back
        -1.0, -1.0, -1.0,
         1.0, -1.0, -1.0,
         1.0,  1.0, -1.0,
        -1.0,  1.0, -1.0
 };

GLuint cube_elements[] = {
        // front
        0, 1, 2,
        2, 3, 0,
        // right
        1, 5, 6,
        6, 2, 1,
        // back
        7, 6, 5,
        5, 4, 7,
        // left
        4, 0, 3,
        3, 7, 4,
        // bottom
        4, 5, 1,
        1, 0, 4,
        // top
        3, 2, 6,
        6, 7, 3
};

unsigned int VAO, VBO, EBO;
viz::Shader* testShader;

glm::mat4 proj = glm::mat4(1.0f);

//-----------------------------------------------//
//                      END                      //
//-----------------------------------------------//

int main(int argc, char** argv) {

    // option parsing
    ScanOpts options;
    int ret = parseOptions(argc, argv, options);
    
    setupLoggers();

    if (ret != 1) {
        printHelp();
        return ret;
    }

    logging::banner(viz::m_appLogger, "Visualizer CLI Program");

    YarrBinary yb;
    json j;

    std::unique_ptr<ClipBoard<EventData>> cb = std::make_unique<ClipBoard<EventData>>();

    yb.init();
    yb.connect(cb.get());
    yb.configure(j);
    yb.run();
    // std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    yb.join();

    viz::m_appLogger->info("Data size: {} / {}", cb->getNumDataIn(), cb->size());
    viz::m_appLogger->info("test main end");

    //OpenGL Initialization BEGIN
    initGLFW();
    GLFWwindow* window = createOpenGLContext(glfwGetVideoMode(glfwGetPrimaryMonitor())->width, glfwGetVideoMode(glfwGetPrimaryMonitor())->height);
    initImGUI(window);
    //OpenGL Initializiation END

    //Dockspace initialization
    dockspaceInit();
    glfwPollEvents();
    glfwSwapBuffers(window);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //Dockspace initialization END

    //Initialization of OpenGL Objects
    glBindFramebuffer(GL_FRAMEBUFFER, sceneFramebuffer.FBO);
    
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices, GL_STATIC_DRAW);

    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_elements), cube_elements, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(GL_FLOAT), (void*)0);
    glEnableVertexAttribArray(0);
    
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    
    //Initializer Shaders
    testShader = new viz::Shader(false, vertexShaderSource, fragmentShaderSource);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    while(!glfwWindowShouldClose(window)){
        frame(window);
    }
    shutdownImGUI();
    glfwTerminate();
    viz::m_appLogger->info("Terminated window");


    return 0;
}

void initGLFW(){
    glfwSetErrorCallback(GLFWErrorCallback);
    if(!glfwInit()){ 
        viz::m_appLogger->error("GLFW initialization failed");
        return;
    }
    viz::m_appLogger->info("GLFW Initialized");

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
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
        viz::m_appLogger->error("Failed to create GLFW window");
        glfwTerminate();
        return NULL;
    }
    glfwMakeContextCurrent(window);

    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
        viz::m_appLogger->error("Failed to initailize GLAD");
        return NULL;
    }
    glViewport(0, 0, width, height);
    glEnable(GL_DEPTH_TEST);
    glfwSwapInterval(1); //Enable vsync

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
    viz::m_appLogger->error("GLFW Code {0}: {1}", err, message);
}


FramebufferData createFramebuffer(float width, float height){
    GLuint FBO, RBO, texture_id;
    glGenFramebuffers(1, &FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    glEnable(GL_DEPTH_TEST);

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
		viz::m_appLogger->error("ERROR::FRAMEBUFFER:: Framebuffer is not complete!");

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
    ImGui::Begin("Scene");
        ImVec2 windim = ImGui::GetContentRegionAvail();
        ImVec2 winpos = ImGui::GetWindowPos();
        ImVec2 winsize = ImGui::GetWindowSize();

        glViewport(0, 0, windim.x, windim.y);
        if(windim.x != sceneFramebuffer.m_width || windim.y != sceneFramebuffer.m_height){
            resizeFramebuffer(sceneFramebuffer, windim.x, windim.y);

            testShader->use();
            proj = glm::perspective(glm::radians(45.0f), (float)windim.x / (float)windim.y, 0.1f, 100.0f);
            testShader->setMat4("proj", proj);
            glUseProgram(0);
        }

        ImVec2 topRight = ImVec2(winpos.x + winsize.x, winpos.y + winsize.y);
        ImGui::GetWindowDrawList()->AddImage(sceneFramebuffer.texture_id, winpos, topRight, ImVec2(0, 1), ImVec2(1, 0));
    ImGui::End();

    glBindFramebuffer(GL_FRAMEBUFFER, sceneFramebuffer.FBO);
        glClearColor(color[0], color[1], color[2], color[3]);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        testShader->use();
            glm::mat4 model = glm::mat4(1.0f);
            glm::mat4 view  = glm::mat4(1.0f);

            model = glm::rotate(model, float(glfwGetTime()), glm::vec3(0.5f, 1.0f, 0.0f));
            view = glm::translate(view, glm::vec3( 0.0f, 0.0f, -10.0f));

            testShader->setMat4("model", model);
            testShader->setMat4("view", view);

            glBindVertexArray(VAO);
            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);
        glUseProgram(0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}