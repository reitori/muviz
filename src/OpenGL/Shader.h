#ifndef SHADER_H
#define SHADER_H

#include "header.h"

#include <string>
#include <fstream>
#include <sstream>

#include "glad/glad.h"

namespace{
    auto logger = logging::makelog("Shaders");
}

namespace viz{
    class Shader{
        private:
            uint16_t m_id;

        public:
            Shader() = delete;
            Shader(const char* vertexPath, const char* fragmentPath, const char* geometryPath = nullptr);

            inline void use() {glUseProgram(m_id);}
    };
}

#endif