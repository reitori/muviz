#ifndef CUBEMAP_H
#define CUBEMAP_H

#include "OpenGL/Texture.h"

namespace viz{
    class Cubemap{
        public:
            Cubemap(const Cubemap& textureArray) = delete;
            Cubemap& operator=(const Cubemap&) = delete;

            Cubemap(const std::vector<std::string>& paths, std::string name);
            Cubemap(uint16_t width, uint16_t height, std::string name);

            bool loadFromPath(const std::vector<std::string>& paths);
            bool success() { return loadSuccessful; } 

            void bind();
            void unbind();

            void enable(Shader& shader, const std::string& uniformName, int pos);

            ~Cubemap();
        private:
            void setDefaultParams();
            void setFormatFromChannels(int channels);

            bool loadSuccessful = true;
			unsigned char* data = nullptr;
			int m_width, m_height, m_nrChannel;
			GLenum m_internalFormat = GL_RGBA8;
            GLenum m_dataFormat = GL_RGBA;
			GLenum m_dataType = GL_UNSIGNED_BYTE;

            GLuint textureID;
            std::vector<std::string> m_paths;
            std::string m_name;
    };
}

#endif