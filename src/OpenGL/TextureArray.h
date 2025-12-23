#ifndef TEXTUREARRAY_H
#define TEXTUREARRAY_H

#include "stb_image.h"

#include <string>
#include <vector>

#include "OpenGL/Shader.h"

namespace viz{
    class Texture2DArray{
        public:
            Texture2DArray(const Texture2DArray& textureArray) = delete;
            Texture2DArray& operator=(const Texture2DArray&) = delete;

            Texture2DArray(const std::vector<std::string>& paths, std::string name, bool mipmap = true);
            Texture2DArray(uint16_t width, uint16_t height, uint8_t layer, std::string name, bool mipmap = true);

            bool loadFromPath(const std::vector<std::string>& paths);

            void bind();
            void unbind();

            void enable(Shader& shader, const std::string& uniformName, int pos);
            void disable();
        private:
            void setDefaultParams();
            void setFormatFromChannels(int channels);

			unsigned char* data = nullptr;
			int m_width, m_height, m_layers, m_nrChannel;
			bool generateMipmap;
			GLenum m_internalFormat = GL_RGBA8;
            GLenum m_dataFormat = GL_RGBA;
			GLenum m_dataType = GL_UNSIGNED_BYTE;

            GLuint textureID;
            std::vector<std::string> m_paths;
            std::string m_name;
    };
}

#endif