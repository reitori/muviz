#include "OpenGL/Renderer.h"

namespace viz{
    Renderer::Renderer(int width, int height){
        m_cameras.emplace(std::make_pair("Main", Camera())); m_currCam = "Main";
        m_framebuffer = std::make_unique<Framebuffer>(width, height);
        m_detector = std::make_unique<Detector>();

        m_framebuffer->bind();
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  
        m_framebuffer->unbind();
    }

    void Renderer::resize(int width, int height){
        m_framebuffer->resize(width, height);
        for(auto& pair : m_cameras)
            pair.second.resize(width, height);
    }

    void Renderer::render(){
        auto now = std::chrono::steady_clock::now();
        double currentTime = std::chrono::duration<double>(now.time_since_epoch()).count();
        delTime = currentTime - lastTime;
        lastTime = currentTime;

        m_framebuffer->bind();
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LESS);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  

            glClearColor(m_color[0], m_color[1], m_color[2], m_color[3]);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            
            auto mainShader = ShaderManager::get("main");
            auto it = m_cameras.find(m_currCam);
            mainShader->setMat4("uView", it->second.getView());
            mainShader->setMat4("uProj", it->second.getCamData().projection);

            m_detector->render(*mainShader);

            glDisable(GL_DEPTH_TEST);
            glDisable(GL_BLEND);
        m_framebuffer->unbind();
    }

    void Renderer::setCurrentCamera(std::string name){
        auto it = m_cameras.find(name);
        if(it != m_cameras.end())
            m_currCam = name;
    }

    void Renderer::attachCamera(std::string name, const Camera& camera){
        auto result = m_cameras.insert(std::make_pair(name, camera));
        if(!result.second)
            m_appLogger->error("Camera already exists!");
    }

    Camera* Renderer::getCamera(std::string name){
        auto it = m_cameras.find(name);
        if(it == m_cameras.end())
            return nullptr;

        return &it->second;
    }

    void Renderer::destroyCamera(std::string name){
        auto it = m_cameras.find(name);
        if(it != m_cameras.end())
            m_cameras.erase(it);
    }

    void Renderer::sortTransparentObjects(){
        auto it = m_cameras.find(m_currCam);
        m_detector->sortTransparent(it->second);
    }
}