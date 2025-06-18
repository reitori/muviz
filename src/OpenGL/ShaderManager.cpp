#include "OpenGL/ShaderManager.h"
#include "logging.h"

namespace{
    auto logger = logging::make_log("ShaderManager");
}

const char* vertexShaderSource="#version 330 core\n"
        "layout (location = 0) in vec3 aPos;\n"
        "layout (location = 1) in vec4 aFrag;\n"

        "layout (location = 2) in vec4 aInstanceFrag;\n"
        "layout (location = 3) in mat4 aInstanceModel;\n"

        "uniform mat4 uModel;\n"
        "uniform mat4 uView;\n"
        "uniform mat4 uProj;\n"

        "uniform bool uIsInstanced;\n"
        
        "out vec4 fragOut;\n"

        "void main()\n"
        "{\n"
        "   mat4 modelTransform = uIsInstanced ? aInstanceModel : uModel;\n"
        "   gl_Position = uProj * uView * modelTransform * vec4(aPos, 1.0);\n"
        "   fragOut = uIsInstanced ? aInstanceFrag : aFrag;\n"
        "}\0";

    const char* geometryShaderSource="version 330 compatibility\n"
        "layout (triangles_adjacency) in;\n"
        "layout (line_strip) out\n;"
        
        "void main();\n"
        "{\n"
        "   "
        "}\n";


    const char* fragmentShaderSource="#version 330 core\n"
        "in vec4 fragOut;\n"
        "out vec4 FragColor;\n"

        "void main()\n"
        "{\n"
        "    FragColor = fragOut;\n"
        "}\0";

namespace viz{
    std::unordered_map<std::string, std::shared_ptr<Shader>> ShaderManager::m_Shaders;

    void ShaderManager::loadShaders(){
        auto execPath = (std::string)getExecutableDir();

        auto mainShader = std::make_shared<Shader>(false, "main", vertexShaderSource, fragmentShaderSource);
        //auto mainShader = std::make_shared<Shader>(true, "main", execPath + "/assets/shaders/MainVertex.glsl", execPath + "/assets/shaders/MainFragment.glsl");
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