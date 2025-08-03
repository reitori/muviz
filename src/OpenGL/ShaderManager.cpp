#include "OpenGL/ShaderManager.h"
#include "logging.h"

namespace{
    auto logger = logging::make_log("ShaderManager");
}

namespace viz{
    std::unordered_map<std::string, std::shared_ptr<Shader>> ShaderManager::m_Shaders;

    void ShaderManager::loadShaders(){
        auto execPath = (std::string)getExecutableDir();

        auto mainShader = std::make_shared<Shader>(true, "main", execPath + "/assets/shaders/MainVertex.glsl", execPath + "/assets/shaders/MainFragment.glsl");
        auto hitmapShader = std::make_shared<Shader>(true, "hitmap", execPath + "/assets/shaders/HitMapVertex.glsl", execPath + "/assets/shaders/HitMapFragment.glsl");
        auto lineShader = std::make_shared<Shader>(true, "line", execPath + "/assets/shaders/LineVertex.glsl", execPath + "/assets/shaders/LineFragment.glsl");
        auto skyShader = std::make_shared<Shader>(true, "skybox", execPath + "/assets/shaders/SkyboxVertex.glsl", execPath + "/assets/shaders/SkyboxFragment.glsl");

        tryShader(mainShader);
        tryShader(hitmapShader);
        tryShader(lineShader);
        tryShader(skyShader);
    }

    bool ShaderManager::addShader(const std::shared_ptr<Shader>& shader){
        if(m_Shaders.find(shader->m_name) == m_Shaders.end()){
            m_Shaders.insert(std::make_pair(shader->m_name, shader));
            return true;
        }
        
        return false;
    }

    std::shared_ptr<Shader> ShaderManager::get(const std::string& name){
        auto shader = m_Shaders.find(name);
        if(shader != m_Shaders.end()){
            return shader->second;
        }
        else{
            return nullptr;
        }
    }

    void ShaderManager::tryShader(const std::shared_ptr<Shader>& shader){
        if(!addShader(shader)){
            logger->error("Could not add {} Shader to ShaderManager -- ensure that assets folder is within the same directory as executable", shader->m_name);
        }else{
            logger->info("Added {} Shader to the shader manager", shader->m_name);
        }
    }
}