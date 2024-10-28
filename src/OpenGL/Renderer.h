#ifndef RENDERER_H
#define RENDERER_H

#include "Framebuffer.h"
#include "Shader.h"

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