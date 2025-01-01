#include "Application.h"

std::shared_ptr<spdlog::logger> viz::m_appLogger = logging::make_log("Visualizer CLI");

namespace viz
{
    Application::Application(int argc, char** argv){
        coreInit = true;
        cliInit(argc, argv);
        glInit(true);
        imGuiInit();

        if(coreInit)
            init();
    }

    Application::Application(int argc, char** argv, int width, int height){
        coreInit = true;
        cliInit(argc, argv);
        glInit(false, width, height);
        imGuiInit();

        if(coreInit)
            init();
    }  

    void Application::init(){
        m_detector = std::make_unique<Detector>();
        m_renderer = std::make_unique<Renderer>(0,0);

        m_detector->init(m_cli);
        m_renderer->attachDetector(m_detector);

        m_GUIWindows.push_back(std::make_unique<Dockspace>("MainDock", ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking));
        m_GUIWindows.push_back(std::make_unique<SceneWindow>("Scene", m_renderer));
        m_GUIWindows.push_back(std::make_unique<ConsoleWindow>("Console"));
        m_GUIWindows.push_back(std::make_unique<ManagerWindow>("Manager"));

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        for(int i = 0; i < m_GUIWindows.size(); i++)
            m_GUIWindows[i]->init();
        ImGui::EndFrame();
    }

    void Application::run(){
        if(coreInit){
            float lastTime = 0.0f;
            float currTime;

            while(true){
                currTime = glfwGetTime();
                ConsoleWindow* console = dynamic_cast<ConsoleWindow*>(m_GUIWindows[2].get());
                console->fps = 1.0f/(currTime - lastTime);
                lastTime = currTime;

                ImGui_ImplOpenGL3_NewFrame();
                ImGui_ImplGlfw_NewFrame();
                ImGui::NewFrame();

                for(int i = 0; i < m_GUIWindows.size(); i++)
                    m_GUIWindows[i]->render();

                ImGui::Render();
                ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
                m_detector->update();

                const ManagerWindow* manager = dynamic_cast<ManagerWindow*>(m_GUIWindows[3].get());
                m_renderer->m_Cameras["Main"].setPos(glm::vec3(manager->x, manager->y, manager->z));
                m_renderer->setColor(glm::vec4(manager->color[0], manager->color[1], manager->color[2], manager->color[3]));

                m_renderer->render();
                m_appWin->render();
            }
        }
    }

    void Application::cliInit(int& argc, char**& argv){
        int cli_status;

        cli_status = m_cli.init(argc, argv); 
        cli_status = m_cli.configure(); 

        if(cli_status < 0){
            m_appLogger->error("CLI Error Code: {}", cli_status);
            coreInit = false;
        }
    }

    void Application::glInit(bool fullscreen, int width, int height){
        glfwSetErrorCallback(Application::GLFWErrorCallback);
        if(!glfwInit()){ 
            coreInit = false;
            m_appLogger->error("GLFW initialization failed");
            glfwTerminate();
        }
        else{
            m_appLogger->info("GLFW Initialized");
            if(fullscreen){
                width = glfwGetVideoMode(glfwGetPrimaryMonitor())->width;
                height = glfwGetVideoMode(glfwGetPrimaryMonitor())->height;
            }

            m_appWin = std::make_unique<glWindow>("Visualizer", width, height);
            if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
                coreInit = false;
                m_appLogger->error("Failed to initailize GLAD");
                glfwTerminate();
            }

            glViewport(0, 0, width, height);
            glEnable(GL_DEPTH_TEST);
        }
    }

    void Application::imGuiInit(){
        m_appLogger->info("{0} IS what core init is set to ", coreInit);
        if(coreInit){
            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            ImGuiIO& io = ImGui::GetIO(); (void)io;
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard | ImGuiConfigFlags_DockingEnable;
            
            ImGui_ImplGlfw_InitForOpenGL(m_appWin->m_window, true);
            ImGui_ImplOpenGL3_Init();
            if(ImGui::GetCurrentContext != nullptr)
                m_appLogger->info("ImGui initialized!");

            ImGui::StyleColorsDark();
        }
    }


    Application::~Application(){
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        glfwTerminate();
    }
} 