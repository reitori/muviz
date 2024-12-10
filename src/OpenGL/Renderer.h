#ifndef RENDERER_H
#define RENDERER_H

#include "OpenGL/Framebuffer.h"
#include "OpenGL/Shader.h"
#include "OpenGL/Scene/Scene.h"
#include "OpenGL/Scene/Entity.h"

#include <glm/glm.hpp>

namespace viz{
    const char* vertexShaderSource="#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "layout (location = 1) in vec4 aFrag;"

    "layout (location = 2) in aInstanceFrag;"
    "layout (location = 3) in mat4 aInstanceModel;"

    "uniform mat4 uModel;\n"
    "uniform mat4 uView;\n"
    "uniform mat4 uProj;\n"

    "uniform bool uIsInstanced\n"
    
    "out vec4 fragOut;"

    "void main()\n"
    "{\n"
    "   mat4 modelTransform = uIsInstanced ? aInstanceModel : uModel;"
    "   gl_Position = proj * view * modelTransform * vec4(aPos, 1.0);\n"
    "   fragOut = usIsInstaced ? mix(aFrag, aInstanceFrag, 0.5) : aFrag;\n"
    "}\0";

    const char* fragmentShaderSource="#version 330 core\n"
        "in vec4 fragOut;"
        "out vec4 FragColor;\n"

        "void main()\n"
        "{\n"
        "    FragColor = fragOut;\n"
        "}\0";

    class Renderer{
        public:
            Renderer(int width, int height);

            void init();
            void resize(int width, int height);

            void addScene(const Scene& scene);
            void addEntity(const Entity& entity);
            
            void render();
        private:
            Framebuffer* m_Framebuffer;

            std::vector<Detectors> m_Detectors;
            std::vector<Shader*> m_Shaders;
            std::vector<Scene> m_Scenes;
    };
}

#endif