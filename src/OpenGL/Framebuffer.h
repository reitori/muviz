#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include "core/header.h"
#include <glad/glad.h>

namespace viz
{
    class Framebuffer{
        private:
            GLuint m_id, m_rbo, m_texid;
            int m_width, m_height;
            bool m_init;
        public:
            Framebuffer(int width, int height);

            void init(int width, int height);
            inline void bind() {glViewport(0, 0, m_width, m_height); glBindFramebuffer(GL_FRAMEBUFFER, m_id);}
            inline void unbind()  {glBindFramebuffer(GL_FRAMEBUFFER, 0);}
            void resize(int width, int height);

            ~Framebuffer();
    };
}


#endif