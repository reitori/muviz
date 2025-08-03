#ifndef SKYBOX_H
#define SKYBOX_H

#include "OpenGL/Cubemap.h"
#include "OpenGL/Shader.h"
#include <glad/glad.h>
#include <vector>

namespace viz {
    class Skybox {
        public:
            Skybox(const std::vector<std::string>& faces, const std::string& name);
            void render();
            Cubemap& getCubemap();

            bool successfulLoad() { return m_cubemap.success(); }

            ~Skybox();
        private:
            void setupCube();

            Cubemap m_cubemap;
            GLuint m_VAO, m_VBO;
    };
}

#endif