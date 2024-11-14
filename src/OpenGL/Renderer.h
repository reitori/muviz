#ifndef RENDERER_H
#define RENDERER_H

#include "OpenGL/Framebuffer.h"
#include "OpenGL/Shader.h"

namespace viz{
    class Renderer{
        private:
            struct SceneData{
                //Projection matrix
                //Objects
            };

            Framebuffer* m_scene;
            Shader* m_vshader;
            Shader* m_fshader;

        public:
            Renderer();

            void init();
            void render();
    };
}

#endif