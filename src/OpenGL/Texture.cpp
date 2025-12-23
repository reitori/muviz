#include "OpenGL/Texture.h"
#include "logging.h"

namespace{
    auto logger = logging::make_log("Texture");
}

namespace viz{
        void Texture::fromPath(const std::string path, const std::string name){
            m_path = path;
            m_name = name;

            loadTexture();
        }
        
        void Texture2D::loadTexture(){
            if(!multisample){
                stbi_set_flip_vertically_on_load(true);
                data = stbi_load(m_path.c_str(), &m_width, &m_height, &m_nrChannel, 0);
                if (data) {
                    switch (m_nrChannel)
                    {
                    case 4:
                        m_internalFormat = GL_RGBA8;
                        m_dataFormat = GL_RGBA;
                        break;
                    case 3:
                        m_internalFormat = GL_RGB8;
                        m_dataFormat = GL_RGB;
                        break;
                    case 2:
                        m_internalFormat = GL_RG8;
                        m_dataFormat = GL_RG;
                        break;
                    case 1:
                        m_internalFormat = GL_R8;
                        m_dataFormat = GL_RED;
                        break;
                    }

                    glGenTextures(1, &textureID);
                    bindTexture();
                    setDefaultParams();
                    glTexImage2D(type, 0, m_internalFormat, m_width, m_height, 0, m_dataFormat, m_dataType, data);
                    if(generateMipmap) glGenerateMipmap(type);
                }

                else {
                    logger->warn("Texture2D ([0]) failed to load.", m_name.c_str());
                }

                stbi_image_free(data);
                data = nullptr;
                glBindTexture(type, 0);
            }
        }


        Texture2D::Texture2D(const std::string path, const std::string name, bool mipmap, bool setMultisample){
            m_path = path;
            m_name = name;
            generateMipmap = mipmap;
            multisample = setMultisample;

            if(setMultisample){
                type = GL_TEXTURE_2D_MULTISAMPLE;
            }else{
                type = GL_TEXTURE_2D;
            }

            fromPath(path, name);
        }

        Texture2D::Texture2D(uint16_t width, uint16_t height, std::string name, bool mipmap, bool setMultisample){
            m_name = name;
            generateMipmap = mipmap;
            multisample  = setMultisample;

            if(setMultisample){
                type = GL_TEXTURE_2D_MULTISAMPLE;
            }else{
                type = GL_TEXTURE_2D;
            }

            glGenTextures(1, &textureID);
            resize(width, height);
        }
        
        void Texture2D::bindTexture() {
            glBindTexture(type, textureID); 
        }

        void Texture2D::unbindTexture() {
            glBindTexture(type, 0);
        }

        void Texture2D::enableTexture(Shader& shader, std::string& name, int pos) {
            glActiveTexture(GL_TEXTURE0 + pos);
            shader.setInt(m_name.c_str(), pos);
        }

        void Texture2D::disableTexture() {
            glActiveTexture(GL_TEXTURE0 + m_TextureUnit);
            glDisable(type);
        }

        void Texture2D::resize(int width, int height) {
            m_width = width;
            m_height = height;

            bindTexture();
            setDefaultParams();
            if(multisample) glTexImage2DMultisample(type, samples, m_internalFormat, m_width, m_height, GL_TRUE);
            else{ glTexImage2D(type, 0, m_internalFormat, m_width, m_height, 0, m_dataFormat, m_dataType, data); }
            if(generateMipmap) glGenerateMipmap(type);
            unbindTexture();
        }

        //Note: Clears the current texture
        void Texture2D::changeFormat(GLenum internalFormat, GLenum dataFormat, GLenum dataType){
            m_internalFormat = internalFormat;
            m_dataFormat = dataFormat;
            m_dataType = dataType;

            bindTexture();
            if(multisample) glTexImage2DMultisample(type, samples, m_internalFormat, m_width, m_height, GL_TRUE);
            else{ glTexImage2D(type, 0, m_internalFormat, m_width, m_height, 0, m_dataFormat, m_dataType, data); }
            if(generateMipmap) glGenerateMipmap(type);
            unbindTexture();
        }

        void Texture2D::setDefaultParams() {
            glTexParameteri(type, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(type, GL_TEXTURE_WRAP_T, GL_REPEAT);
            if(generateMipmap){ 
                glTexParameteri(type, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                glTexParameteri(type, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            }
            else {
                glTexParameteri(type, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(type, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            }
        }
}