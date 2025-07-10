#include "Application.h"

std::shared_ptr<spdlog::logger> viz::m_appLogger = logging::make_log("Visualizer");

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
        ShaderManager::loadShaders();

        m_detector = std::make_unique<Detector>();
        m_renderer = std::make_unique<Renderer>(1,1); //arbitrary width/height, automatically resized upon load of application

        m_detector->init(m_cli);
        m_renderer->attachDetector(m_detector);

        m_GUIWindows.push_back(std::make_unique<Dockspace>("MainDock", ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking));
        m_GUIWindows.push_back(std::make_unique<SceneWindow>("Scene", m_renderer));
        m_GUIWindows.push_back(std::make_unique<ConsoleWindow>("Console"));
        m_GUIWindows.push_back(std::make_unique<ManagerWindow>("Manager", m_renderer));
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        for(int i = 0; i < m_GUIWindows.size(); i++)
            m_GUIWindows[i]->init();
        ImGui::EndFrame();

        m_detector->setEventCallback(std::bind(&Application::onEvent, this, std::placeholders::_1));
        m_appWin->setEventCallback(std::bind(&Application::onEvent, this, std::placeholders::_1));
    }

    void Application::run(){
        if(coreInit){
            float lastTime = 0.0f;
            float currTime;

            while(!m_appWin->windowShouldClose()){
                currTime = glfwGetTime();
                ConsoleWindow* console = dynamic_cast<ConsoleWindow*>(m_GUIWindows[2].get());
                float dTime = currTime - lastTime;
                console->fps = 1.0f/dTime;
                lastTime = currTime;

                ImGui_ImplOpenGL3_NewFrame();
                ImGui_ImplGlfw_NewFrame();
                ImGui::NewFrame();

                for(int i = 0; i < m_GUIWindows.size(); i++)
                    m_GUIWindows[i]->render();

                ImGui::Render();
                ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

                m_detector->update(*m_renderer->getCamera(), dTime);

                const ManagerWindow* manager = dynamic_cast<ManagerWindow*>(m_GUIWindows[3].get());
                m_renderer->setColor(glm::vec4(manager->color[0], manager->color[1], manager->color[2], manager->color[3]));
                if(manager->startCLI)
                    m_cli->start();
                
                m_detector->updateHitDurations(manager->hitDuration);

                m_renderer->render();
                m_appWin->render();
            }
        }
    }

    void Application::onEvent(system::event& e){
        if(e.getEventType() == system::eventType::particleHit){
            std::string str = e.toString();
            ConsoleWindow* console = dynamic_cast<ConsoleWindow*>(m_GUIWindows[2].get());
            console->AddLog(str.c_str());
        }

        for(int i = 0 ; i < m_GUIWindows.size(); i++){
            m_GUIWindows[i]->onEvent(e);
        }
    }

    void Application::cliInit(int& argc, char**& argv){
        int cli_status;

        m_cli = std::make_unique<VisualizerCli>();
        cli_status = m_cli->init(argc, argv); 
        cli_status = m_cli->configure(); 

        if(cli_status < 0){
            m_appLogger->error("CLI Error Code: {}", cli_status);
            coreInit = false;
        }
    }

    void Application::glInit(bool fullscreen, int width, int height){
        glfwSetErrorCallback(Application::GLFWErrorCallback);
        if(!glfwInit()){ 
            coreInit = false;
            m_appLogger->error("GLFW initialization failed!");
            glfwTerminate();
        }
        else{
            m_appLogger->info("GLFW Initialized!");
            if(fullscreen){
                width = glfwGetVideoMode(glfwGetPrimaryMonitor())->width;
                height = glfwGetVideoMode(glfwGetPrimaryMonitor())->height;
            }

            m_appWin = std::make_unique<glWindow>("Visualizer", width, height);
            if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
                coreInit = false;
                m_appLogger->error("Failed to initailize GLAD!");
                glfwTerminate();
            }

            glViewport(0, 0, width, height);
            glEnable(GL_DEPTH_TEST);
        }
    }

    void Application::imGuiInit(){
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
        m_GUIWindows.clear(); std::cout << "Detector pointer instances: " << m_detector.use_count() << std::endl;
        m_appWin.reset(); std::cout << "Renderer pointer instances: " << m_renderer.use_count() << std::endl;
        m_detector.reset();
        m_renderer.reset();

        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        glfwTerminate();
    }
} 