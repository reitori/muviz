#include "OpenGL/Framebuffer.h"
#include "logging.h"

namespace{
    auto logger = logging::make_log("Framebuffer");
}

namespace viz{

    Framebuffer::Framebuffer(int width, int height){
        m_init = false;
        init(width, height);
    }

   void Framebuffer::init(int width, int height) {
        if(m_init) {
            m_appLogger->info("Framebuffer already initialized");
            return;
        }

        glGenFramebuffers(1, &m_id);
        glBindFramebuffer(GL_FRAMEBUFFER, m_id);

        // Create Texture2D for framebuffer color attachment
        m_targetTexture = std::make_shared<Texture2D>(width, height, "FramebufferTexture", false);
        m_targetTexture->changeFormat(GL_RGB, GL_RGB, GL_UNSIGNED_BYTE);
        m_targetTexture->bindTexture();
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_targetTexture->getID(), 0);

        // Depth/Stencil buffer
        glGenRenderbuffers(1, &m_rbo);
        glBindRenderbuffer(GL_RENDERBUFFER, m_rbo);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_rbo);

        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (status != GL_FRAMEBUFFER_COMPLETE) {
            logger->warn("FBO incomplete: 0x{:X}", status);
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
        m_targetTexture->unbindTexture();

        m_width = width;
        m_height = height;
        m_init = true;
    }

    void Framebuffer::resize(int width, int height) {
        GLint drawFBOid = 0, readFBOid = 0;
        glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &drawFBOid);
        glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &readFBOid);

        glBindFramebuffer(GL_FRAMEBUFFER, m_id);
            m_targetTexture->resize(width, height);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_targetTexture->getID(), 0);

            glBindRenderbuffer(GL_RENDERBUFFER, m_rbo);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_rbo);

        glBindTexture(GL_TEXTURE_2D, 0);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, readFBOid);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, drawFBOid);

        m_width = width;
        m_height = height;
    }
    

    Framebuffer::~Framebuffer(){
        glDeleteRenderbuffers(1, &m_rbo);
        glDeleteFramebuffers(1, &m_id);
    }
}