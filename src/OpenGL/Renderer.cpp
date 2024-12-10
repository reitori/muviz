#include "OpenGL/Renderer.h"

namespace viz{
    Renderer::Renderer(int width, int height){
        m_Shaders[0] = new Shader(false, viz::vertexShaderSource , viz::fragmentShaderSource);
        m_Framebuffer = new Framebuffer(width, height);
    }

    Renderer::resize(int width, int height){
        m_Framebuffer->resize(width, height);
    }

    Renderer::~Renderer(){
        for(int i = 0; i < m_Shaders.size(); i++){
            delete m_Shaders[i];
        }
        delete m_Framebuffer;
    }
}