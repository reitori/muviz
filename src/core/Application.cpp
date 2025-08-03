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

        m_GUIWindows.push_back(std::make_shared<Dockspace>("MainDock", ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking));
        m_GUIWindows.push_back(std::make_shared<SceneWindow>("Scene"));
        std::shared_ptr<Renderer> renderer = dynamic_cast<SceneWindow*>(m_GUIWindows[1].get())->getRenderer();
        m_GUIWindows.push_back(std::make_shared<ConsoleWindow>("Console"));
        m_GUIWindows.push_back(std::make_unique<ManagerWindow>("Manager", renderer));
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        for(int i = 0; i < m_GUIWindows.size(); i++)
            m_GUIWindows[i]->init();
        ImGui::EndFrame();

        m_detector = std::make_shared<Detector>();
        m_detector->globalDetectorTransformation = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        m_detector->init(m_cli);
        
        auto scene = std::make_shared<SceneWindow>("Scene");
        renderer->attachDetector(m_detector);

        m_detector->setEventCallback(std::bind(&Application::onEvent, this, std::placeholders::_1));
        m_appWin->setEventCallback(std::bind(&Application::onEvent, this, std::placeholders::_1));

        Camera* cam = renderer->getCamera();
        float radius = 45.0f;

        //PATH TYPE1
        // std::shared_ptr<Curve> semiCircle1 = std::make_shared<Arc>(
        //     glm::vec3(-radius, 0.0f, 7.2f),
        //     glm::vec3(0.0f, radius, 7.2f),
        //     glm::vec3(radius, 0.0f, 7.2f)
        // );
        // std::shared_ptr<Curve> semiCircle2 = std::make_shared<Arc>(
        //     glm::vec3(radius, 0.0f, 7.2f),
        //     glm::vec3(0.0f, -radius, 7.2f),
        //     glm::vec3(-radius, 0.0f, 7.2f)
        // );

        // semiCircle1->initForRender(); 
        // semiCircle2->initForRender();

        // m_renderer->addCurve(semiCircle1);
        // m_renderer->addCurve(semiCircle2);

        // CamPath semiCirclePath1;
        // CamPath semiCirclePath2;

        // semiCirclePath1.position = semiCircle1;
        // semiCirclePath1.lookAtPoint = glm::vec3(0.0f, 0.0f, 7.2f);
        // semiCirclePath1.timeDuration = 10.0f;

        // semiCirclePath2.position = semiCircle2;
        // semiCirclePath2.lookAtPoint = glm::vec3(0.0f, 0.0f, 7.2f);
        // semiCirclePath2.timeDuration = 10.0f;

        // cam->pushCamPath(semiCirclePath1);
        // cam->pushCamPath(semiCirclePath2);

        //PATH TYPE2
        // std::vector<glm::vec3> linePoints = {
        //     glm::vec3(0.0f, 0.0f, 0.0f),
        //     glm::vec3(0.0f, 0.0f, 7.2f),
        //     glm::vec3(0.0f, 0.0f, 14.478f)
        // };

        // std::shared_ptr<Curve> lineUp = std::make_shared<Bezier>(linePoints, true);
        // lineUp->color = glm::vec4(0.0f, 0.5f, 0.5f, 1.0f);

        // std::shared_ptr<Curve> helix = std::make_shared<Helix>(
        //     glm::vec3(-radius, 0.0f, 0.0f),
        //     glm::vec3(0.0f, radius, 7.2f),
        //     glm::vec3(-radius, 0.0f, 14.478f)
        // );
        // helix->color = glm::vec4(0.0f, 0.5f, 0.5f, 1.0f);

        // lineUp->initForRender();
        // helix->initForRender();

        // m_renderer->addCurve(lineUp);
        // m_renderer->addCurve(helix);

        // CamPath helixPath;
        // helixPath.position = helix;
        // helixPath.lookAtCurve = lineUp;
        // helixPath.timeDuration = 20.0f;
        // helixPath.fixedUpVector = glm::vec3(0.0f, 0.0f, 1.0f);

        // std::function<float(float)> parametricBlend = [](float t){
        //     float sqr = t * t;
        //     return sqr / (2.0f * (sqr - t) + 1.0f);
        // };

        // cam->pushCamPath(helixPath);
        // cam->globalTimeFunc = parametricBlend;

        //PATH TYPE3
        std::vector<glm::vec3> bsplinePoints = {
            glm::vec3( 60.0f,   0.0f,   0.0f),
            glm::vec3( 50.0f,   5.0f,  25.0f),
            glm::vec3( 30.0f,  12.0f,  40.0f),
            glm::vec3(  0.0f,  18.0f,  50.0f),
            glm::vec3(-30.0f,  25.0f,  40.0f),
            glm::vec3(-50.0f,  30.0f,  15.0f),
            glm::vec3(-45.0f,  20.0f, -20.0f),
            glm::vec3(-20.0f,  10.0f, -45.0f),
            glm::vec3( 10.0f,   0.0f, -50.0f)
        };

        std::shared_ptr<Curve> bspline = std::make_shared<BSpline>(bsplinePoints, 3);
        bspline->color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);

        bspline->initForRender();
        renderer->addCurve(bspline);

        CamPath camPath;
        camPath.position = bspline;
        camPath.lookAtPoint = glm::vec3(0.0f, 7.2f, 0.0f);
        camPath.timeDuration = 20.0f;
        camPath.useWorldUp = true;

        std::function<float(float)> parametricBlend = [](float t) {
            float sqr = t * t;
            return sqr / (2.0f * (sqr - t) + 1.0f);
        };

        cam->pushCamPath(camPath);
        cam->globalTimeFunc = parametricBlend;
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

                std::shared_ptr<Renderer> renderer = dynamic_cast<SceneWindow*>(m_GUIWindows[1].get())->getRenderer();
                //Detector Frame
                m_detector->update(*renderer->getCamera(), dTime);

                //Updates to detector
                const ManagerWindow* manager = dynamic_cast<ManagerWindow*>(m_GUIWindows[3].get());
                renderer->setColor(glm::vec4(manager->color[0], manager->color[1], manager->color[2], manager->color[3]));
                if(manager->startCLI)
                    m_cli->start();
                
                m_detector->updateHitDurations(manager->hitDuration);
                m_detector->trackIsImmortal = manager->hitDurIsIndefinite;

                //GUI Frame
                ImGui_ImplOpenGL3_NewFrame();
                ImGui_ImplGlfw_NewFrame();
                ImGui::NewFrame();

                for(int i = 0; i < m_GUIWindows.size(); i++)
                    m_GUIWindows[i]->render();

                ImGui::Render();
                ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

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
        m_GUIWindows.clear(); 
        m_appWin.reset();
        m_detector.reset();

        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        glfwTerminate();
    }
} 