#include "Application.h"

std::shared_ptr<spdlog::logger> viz::m_appLogger = logging::make_log("Visualizer CLI");

namespace viz
{
    const char* vertTest="#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"

    "uniform mat4 model;"
    "uniform mat4 view;"
    "uniform mat4 proj;"

    "void main()\n"
    "{\n"
    "   gl_Position = proj * view * model * vec4(aPos, 1.0);\n"
    "}\0";

const char* fragTest="#version 330 core\n"
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

            m_renderer->m_framebuffer->bind();
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
            testShader = new viz::Shader(false, "test", vertTest, fragTest);
            m_renderer->m_framebuffer->unbind();

            while(true){
                currTime = glfwGetTime();
                m_appLogger->info("FPS: {0}", (1.0f/(currTime - lastTime)));
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


                m_renderer->m_framebuffer->bind();
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
                m_renderer->m_framebuffer->unbind();



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