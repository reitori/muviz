#include "OpenGL/Renderer.h"

namespace viz{
    Renderer::Renderer(int width, int height){
        m_cameras.emplace(std::make_pair("Main", Camera())); m_currCam = "Main";
        m_cameras.at(m_currCam).setPos(glm::vec3(0.0f, 0.0f, -50.0f));
        m_framebuffer = std::make_unique<Framebuffer>(width, height, true);
        m_screenFramebuffer = std::make_unique<Framebuffer>(width, height, false, false);
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
        m_screenFramebuffer->resize(width, height);
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
            glEnable(GL_MULTISAMPLE);
            glEnable(GL_LINE_SMOOTH);
            glLineWidth(5.0f);
            glDepthFunc(GL_LESS);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  

            glClearColor(m_color[0], m_color[1], m_color[2], m_color[3]);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            
            auto mainShader = ShaderManager::get("main");
            auto it = m_cameras.find(m_currCam);
            mainShader->use();
            mainShader->setMat4("uView", it->second.getView());
            mainShader->setMat4("uProj", it->second.getCamData().projection);

            mainShader->setBool("uUseGlobalColor", true);
            mainShader->setMat4("uModel", glm::mat4(1.0f));
            mainShader->setBool("uIsInstanced", false);
            for(auto& curve : m_curves){
                mainShader->setVec4("uGlobalColor", curve->color);
                curve->render();
            }
            mainShader->setBool("uUseGlobalColor", false);


            m_detector->render(*mainShader);

            glDisable(GL_DEPTH_TEST);
            glDisable(GL_MULTISAMPLE);
            glDisable(GL_BLEND);

        //Need to pass into MSAA
        m_framebuffer->blitFramebuffer(m_screenFramebuffer);
        m_framebuffer->unbind();
    }

    void Renderer::setCurrentCamera(std::string name){
        auto it = m_cameras.find(name);
        if(it != m_cameras.end())
            m_currCam = name;
    }

    void Renderer::addCurve(const std::shared_ptr<Curve>& curve){
        m_curves.push_back(curve);
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