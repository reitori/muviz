#include "Application.h"

std::shared_ptr<spdlog::logger> viz::m_appLogger = logging::make_log("Visualizer");

std::chrono::steady_clock::time_point ApplicationStartTimePoint;

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

        m_detector = std::make_shared<Detector>();
        m_detector->globalDetectorTransformation = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        m_detector->init(m_cli);

        m_GUIWindows.push_back(std::make_shared<Dockspace>("MainDock", ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking));
        m_GUIWindows.push_back(std::make_shared<SceneWindow>("Scene"));
        std::shared_ptr<Renderer> renderer = dynamic_cast<SceneWindow*>(m_GUIWindows[1].get())->getRenderer();
        m_GUIWindows.push_back(std::make_shared<ConsoleWindow>("Console"));
        m_GUIWindows.push_back(std::make_unique<ManagerWindow>("Manager", renderer));
        m_GUIWindows.push_back(std::make_unique<ScrubberWindow>("Scrubber Window", m_detector));
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        for(int i = 0; i < m_GUIWindows.size(); i++)
            m_GUIWindows[i]->init();
        ImGui::EndFrame();
        
        auto scene = std::make_shared<SceneWindow>("Scene");
        renderer->attachDetector(m_detector);

        m_detector->setEventCallback(std::bind(&Application::onEvent, this, std::placeholders::_1));
        m_appWin->setEventCallback(std::bind(&Application::onEvent, this, std::placeholders::_1));

        Camera* cam = renderer->getCamera();
        float radius = 250.0f;

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
        //     glm::vec3(0.0f, -390.535f, 0.0f),
        //     glm::vec3(0.0f, -348.548f, 0.0f),
        //     glm::vec3(0.0f, -308.69f, 0.0f)
        // };

        // std::shared_ptr<Curve> lineUp = std::make_shared<Bezier>(linePoints, true);
        // lineUp->color = glm::vec4(0.0f, 0.5f, 0.5f, 1.0f);

        // std::vector<glm::vec3> helixPoints = {
        //     glm::vec3( radius, -390.535f,   0.0f),   // start (x = +r, z = 0)
        //     glm::vec3( 0.0f,   -370.0f,     radius), // quarter turn
        //     glm::vec3(-radius, -350.0f,     0.0f),   // half turn
        //     glm::vec3( 0.0f,   -330.0f,    -radius), // three-quarter turn
        //     glm::vec3( radius, -310.0f,     0.0f),   // full turn
        //     glm::vec3( 0.0f,   -300.0f,     radius), // quarter into 2nd turn
        //     glm::vec3(-radius, -295.0f,     0.0f),   // halfway through 2nd turn
        //     glm::vec3( 0.0f,   -300.0f,    -radius), // 3/4 turn in 2nd
        //     glm::vec3( radius, -308.679f,   0.0f)    // end at top, wrap around
        // };

        // std::shared_ptr<Curve> helix = std::make_shared<BSpline>(helixPoints, 3);
        // helix->color = glm::vec4(0.0f, 0.5f, 0.5f, 1.0f);

        // lineUp->initForRender();
        // helix->initForRender();

        // renderer->addCurve(lineUp);
        // renderer->addCurve(helix);

        // CamPath helixPath;
        // helixPath.position = helix;
        // helixPath.lookAtCurve = lineUp;
        // helixPath.timeDuration = 45.0f;
        // helixPath.fixedUpVector = glm::vec3(0.0f, 1.0f, 0.0f);

        // std::function<float(float)> parametricBlend = [](float t){
        //     float sqr = t * t;
        //     return sqr / (2.0f * (sqr - t) + 1.0f);
        // };

        // cam->pushCamPath(helixPath);
        // cam->globalTimeFunc = parametricBlend;

        //PATH TYPE3
        std::vector<glm::vec3> bsplinePoints = {
            glm::vec3( radius, -390.535f,   0.0f),   // start (x = +r, z = 0)
            glm::vec3( 0.0f,   -370.0f,     radius), // quarter turn
            glm::vec3(-radius, -350.0f,     0.0f),   // half turn
            glm::vec3( 0.0f,   -330.0f,    -radius), // three-quarter turn
            glm::vec3( radius, -310.0f,     0.0f),   // full turn
            glm::vec3( 0.0f,   -300.0f,     radius), // quarter into 2nd turn
            glm::vec3(-radius, -295.0f,     0.0f),   // halfway through 2nd turn
            glm::vec3( 0.0f,   -300.0f,    -radius), // 3/4 turn in 2nd
            glm::vec3( radius, -308.679f,   0.0f)    // end at top, wrap around
        };

        std::shared_ptr<Curve> bspline = std::make_shared<BSpline>(bsplinePoints, 3);
        bspline->color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);

        // bspline->initForRender();
        // renderer->addCurve(bspline);

        CamPath camPath;
        camPath.position = bspline;
        camPath.lookAtPoint = glm::vec3(0.0f, -349.607f, 0.0f);
        camPath.timeDuration = 60.0f;
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
            ApplicationStartTimePoint = std::chrono::steady_clock::now();
            std::chrono::steady_clock::time_point lastTime = std::chrono::steady_clock::now();
            std::chrono::steady_clock::time_point currTime;

            while(!m_appWin->windowShouldClose()){
                currTime = std::chrono::steady_clock::now();
                ConsoleWindow* console = dynamic_cast<ConsoleWindow*>(m_GUIWindows[2].get());
                std::chrono::duration<double> delTime = currTime - lastTime;
                double dTime = delTime.count();
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
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
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