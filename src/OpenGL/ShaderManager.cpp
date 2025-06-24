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

        if(!addShader(mainShader))
            logger->error("Could not add Main Shader to ShaderManager -- ensure that assets folder is within the same directory as executable");
        if(!addShader(hitmapShader))
            logger->error("Could not add HitMap Shader to ShaderManager -- ensure that assets folder is within the same directory as executable");

        logger->info("Added Main Shader and HitMap Shader to the ShaderManager");
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
}