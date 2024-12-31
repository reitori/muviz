#include "OpenGL/Renderer.h"

namespace viz{
    // const char* vertexShaderSource="#version 330 core\n"
    //     "layout (location = 0) in vec3 aPos;\n"
    //     "layout (location = 1) in vec4 aFrag;\n"

    //     "layout (location = 2) in aInstanceFrag;\n"
    //     "layout (location = 3) in mat4 aInstanceModel;\n"

    //     "uniform mat4 uModel;\n"
    //     "uniform mat4 uView;\n"
    //     "uniform mat4 uProj;\n"

    //     "uniform bool uIsInstanced;\n"
        
    //     "out vec4 fragOut;\n"

    //     "void main()\n"
    //     "{\n"
    //     "   mat4 modelTransform = uIsInstanced ? aInstanceModel : uModel;\n"
    //     "   gl_Position = proj * view * modelTransform * vec4(aPos, 1.0);\n"
    //     "   fragOut = usIsInstanced ? mix(aFrag, aInstanceFrag, 0.5) : aFrag;\n"
    //     "}\0";

    const char* vertexShaderSource = R"(
        #version 330 core
        layout (location = 0) in vec3 aPos;

        void main()
        {
            gl_Position = vec4(aPos, 1.0);
        }
    )";

    const char* fragmentShaderSource="#version 330 core\n"
        "in vec4 fragOut;\n"
        "out vec4 FragColor;\n"

        "void main()\n"
        "{\n"
        "    FragColor = fragOut;\n"
        "}\0";

    Renderer::Renderer(int width, int height){
        m_Shaders.emplace_back(false, "default", viz::vertexShaderSource, viz::fragmentShaderSource);
        m_Cameras.emplace(std::make_pair("Main", Camera())); m_currCam = "Main";
        m_framebuffer = std::make_unique<Framebuffer>(width, height);
        m_detector = std::make_unique<Detector>();

        m_activeShaderNum = 0;
    }

    void Renderer::resize(int width, int height){
        m_framebuffer->resize(width, height);
        for(auto& pair : m_Cameras)
            pair.second.resize(width, height);
    }

    void Renderer::render(){
        m_framebuffer->bind();
            glClearColor(m_color[0], m_color[1], m_color[2], m_color[3]);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            
            m_detector->render(m_Shaders[0]);

            auto it = m_Cameras.find(m_currCam);
            m_Shaders[0].setMat4("uView", it->second.getView());
            m_Shaders[0].setMat4("uProj", it->second.data.projection);

        m_framebuffer->unbind();
    }

    void Renderer::setCurrentCamera(std::string name){
        auto it = m_Cameras.find(name);
        if(it != m_Cameras.end())
            m_currCam = name;
    }

    void Renderer::attachCamera(std::string name, const Camera& camera){
        auto result = m_Cameras.insert(std::make_pair(name, camera));
        if(!result.second)
            m_appLogger->error("Camera already exists!");
    }

    Camera* Renderer::getCamera(std::string name){
        auto it = m_Cameras.find(name);
        if(it == m_Cameras.end())
            return nullptr;

        return &it->second;
    }

    void Renderer::destroyCamera(std::string name){
        auto it = m_Cameras.find(name);
        if(it != m_Cameras.end())
            m_Cameras.erase(it);
    }
}