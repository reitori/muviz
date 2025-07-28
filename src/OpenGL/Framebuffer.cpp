#include "OpenGL/Framebuffer.h"
#include "logging.h"

namespace{
    auto logger = logging::make_log("Framebuffer");
}

namespace viz{
    Framebuffer::Framebuffer(int width, int height, bool setMultisample, bool useRenderbufferAttachment){
        m_init = false;
        m_multisample = setMultisample;
        m_useRenderbuffer = useRenderbufferAttachment;

        // Create Texture2D for framebuffer color attachment
        m_targetTexture = std::make_shared<Texture2D>(width, height, "FramebufferTexture", false, m_multisample);
        m_targetTexture->changeFormat(GL_RGB, GL_RGB, GL_UNSIGNED_BYTE);

        init(width, height);
    }

    Framebuffer::Framebuffer(std::shared_ptr<Texture2D>& texture, bool setMultisample, bool useRenderbufferAttachment){
        m_init = false;
        m_multisample = setMultisample;
        m_useRenderbuffer = useRenderbufferAttachment;

        m_targetTexture = texture;
    }

   void Framebuffer::init(int width, int height) {
        if(m_init) {
            m_appLogger->info("Framebuffer already initialized");
            return;
        }

        glGenFramebuffers(1, &m_id);
        glBindFramebuffer(GL_FRAMEBUFFER, m_id);

        m_targetTexture->bindTexture();
        if(!m_multisample) glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_targetTexture->getID(), 0);
        else { glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, m_targetTexture->getID(), 0); }
        // Depth/Stencil buffer
        if(m_useRenderbuffer){
            glGenRenderbuffers(1, &m_rbo);
            glBindRenderbuffer(GL_RENDERBUFFER, m_rbo);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
            if(m_multisample) glRenderbufferStorageMultisample(GL_RENDERBUFFER, m_targetTexture->samples, GL_DEPTH24_STENCIL8, width, height);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_rbo);
        }

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

    void Framebuffer::changeTexture(std::shared_ptr<Texture2D>& texture){
        m_targetTexture = texture;
        resize(m_targetTexture->getWidth(), m_targetTexture->getHeight());
    }

    void Framebuffer::blitFramebuffer(const std::unique_ptr<Framebuffer>& toScreenFramebuffer){
        if(m_multisample){
            glBindFramebuffer(GL_READ_FRAMEBUFFER, m_id);
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, toScreenFramebuffer->m_id);
            glBlitFramebuffer(0, 0, m_width, m_height, 0, 0, toScreenFramebuffer->m_width, toScreenFramebuffer->m_height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
        }
    }   

    void Framebuffer::resize(int width, int height) {
        glBindFramebuffer(GL_FRAMEBUFFER, m_id);
            m_targetTexture->resize(width, height);

            if(!m_multisample) glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_targetTexture->getID(), 0);
            else { glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, m_targetTexture->getID(), 0); }

            if(m_useRenderbuffer){
                glBindRenderbuffer(GL_RENDERBUFFER, m_rbo);
                glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
                if(m_multisample) glRenderbufferStorageMultisample(GL_RENDERBUFFER, m_targetTexture->samples, GL_DEPTH24_STENCIL8, width, height);
                glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_rbo);
            }

        m_width = width;
        m_height = height;
    }
    

    Framebuffer::~Framebuffer(){
        glDeleteRenderbuffers(1, &m_rbo);
        glDeleteFramebuffers(1, &m_id);
    }
}