#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include "core/header.h"
#include <glad/glad.h>
#include "OpenGL/Texture.h"

namespace viz
{

    class Framebuffer{
        public:
            Framebuffer() = delete;
            Framebuffer(int width, int height, bool setMultisample = false, bool useRenderbufferAttachment = true);
            Framebuffer(std::shared_ptr<Texture2D>& texture, bool setMultisample = false, bool useRenderbufferAttachment = true);

            void init(int width, int height);
            inline void bind() {glViewport(0, 0, m_width, m_height); glBindFramebuffer(GL_FRAMEBUFFER, m_id);}
            inline void unbind()  {glBindFramebuffer(GL_FRAMEBUFFER, 0);}

            void blitFramebuffer(const std::unique_ptr<Framebuffer>& toScreenFramebuffer);
            void changeTexture(std::shared_ptr<Texture2D>& texture);

            inline std::uint16_t getWidth() {return m_width;}
            inline std::uint16_t getHeight() {return m_height;}
            inline GLuint getTexID() {return m_targetTexture->getID();}
            inline std::shared_ptr<Texture2D> getTexture() {return m_targetTexture;}

            void resize(int width, int height);

            ~Framebuffer();
        private:
            std::shared_ptr<Texture2D> m_targetTexture;
            GLuint m_id, m_rbo;
            std::uint16_t m_width, m_height;
            bool m_init, m_multisample, m_useRenderbuffer;
    };
}


#endif