#ifndef SHADERMANAGER_H
#define SHADERMANAGER_H

#include <string>
#include <unordered_map>

#include "util/include/util.hpp"

#include "OpenGL/Shader.h"

namespace viz{
class ShaderManager {
    public:
        static void loadShaders(); // Load all shared shaders
        static bool addShader(const std::shared_ptr<Shader>& shader);
        static std::shared_ptr<Shader> get(const std::string& name);
    private:
        static std::unordered_map<std::string, std::shared_ptr<Shader>> m_Shaders;
    };
}

#endif