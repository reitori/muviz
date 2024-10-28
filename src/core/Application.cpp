#include "Application.h"

namespace viz
{
    Application::Application(){
        logging::banner(logger, "Visualizer CLI Program");
        coreInit = true; //If any initialization fails, set to false

        glfwSetErrorCallback(Application::GLFWErrorCallback);
        if(!glfwInit()){ 
            coreInit = false;
            logger->error("GLFW initialization failed");
            glfwTerminate();
        }
        else{
            logger->info("GLFW Initialized");

            appWin = new glWindow(glfwGetVideoMode(glfwGetPrimaryMonitor())->width, glfwGetVideoMode(glfwGetPrimaryMonitor())->height, "Visualizer");
            appWin->setAsContext();

            if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
                coreInit = false;
                logger->error("Failed to initailize GLAD");
                glfwTerminate();
            }
        }

        if(coreInit){
            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            ImGuiIO& io = ImGui::GetIO(); (void)io;
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
            
            ImGui_ImplGlfw_InitForOpenGL(appWin->m_window, true);
            ImGui_ImplOpenGL3_Init();

            ImGui::StyleColorsDark();
        }
    }

    Application::Application(int width, int height){
        logging::banner(logger, "Visualizer CLI Program");

        glfwSetErrorCallback(Application::GLFWErrorCallback);
        if(!glfwInit()){ 
            logger->error("GLFW initialization failed");
            glfwTerminate();
        }
        else{
            logger->info("GLFW Initialized");

            appWin = new glWindow(width, height, "Visualizer");
            appWin->setAsContext();

            if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
                coreInit = false;
                logger->error("Failed to initailize GLAD");
                glfwTerminate();
            }
        }
        
        if(coreInit){
            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            ImGuiIO& io = ImGui::GetIO(); (void)io;
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
            io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
            io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; 
            
            ImGui_ImplGlfw_InitForOpenGL(appWin->m_window, true);
            ImGui_ImplOpenGL3_Init();

            ImGui::StyleColorsDark();
        }
    }  

    void Application::run(){
        if(coreInit){
            while(true){

            }
        }
    }
} 