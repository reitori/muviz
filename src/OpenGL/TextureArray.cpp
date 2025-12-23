#include "OpenGL/TextureArray.h"

namespace{
    auto logger = logging::make_log("Texture2DArray");
}

namespace viz{ 
    Texture2DArray::Texture2DArray(const std::vector<std::string>& paths, std::string name, bool mipmap){
        m_paths = paths;
        m_name = name;
        generateMipmap = mipmap;

        glGenTextures(1, &textureID);
        loadFromPath(paths);
    }

    Texture2DArray::Texture2DArray(uint16_t width, uint16_t height, uint8_t layers, std::string name, bool mipmap){
        m_width = width;
        m_height = height;
        m_layers = layers;
        m_name = name;
        generateMipmap = mipmap;
        
        glGenTextures(1, &textureID);

        bind();
        setDefaultParams();
        glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, m_internalFormat, m_width, m_height, m_layers, 0, m_dataFormat, m_dataType, nullptr);
        if(generateMipmap) glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
        unbind();
    }

    bool Texture2DArray::loadFromPath(const std::vector<std::string>& paths){
        if (paths.empty()) {
                logger->error("Texture2DArray '{}' load failed: No paths provided", m_name);
                return false;
            }

            bind();
            stbi_set_flip_vertically_on_load(true);

            int w, h, channels;
            unsigned char* firstData = stbi_load(paths[0].c_str(), &w, &h, &channels, 0);
            if (!firstData) {
                logger->error("Texture2DArray '{}' failed to load layer 0: {}", m_name, paths[0]);
                return false;
            }
            setFormatFromChannels(channels);
            m_width = w;
            m_height = h;
            m_layers = static_cast<int>(paths.size());

            logger->info("Loading Texture2DArray '{}' with {} layers ({}x{})", m_name, m_layers, m_width, m_height);

            glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, m_internalFormat, m_width, m_height, m_layers, 0, m_dataFormat, m_dataType, nullptr);
 
            glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0, m_width, m_height, 1, m_dataFormat, m_dataType, firstData);
            stbi_image_free(firstData);

            for (int i = 1; i < m_layers; ++i) {
                unsigned char* data = stbi_load(paths[i].c_str(), &w, &h, &channels, 0);
                if (!data) {
                    logger->error("Texture2DArray '{}' failed to load layer {}: {}", m_name, i, paths[i]);
                    continue;
                }
                if (w != m_width || h != m_height) {
                    logger->warn("Texture2DArray '{}' layer {} size mismatch ({}x{}) - expected ({}x{})",
                                 m_name, i, w, h, m_width, m_height);
                    stbi_image_free(data);
                    continue;
                }
                glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, m_width, m_height, 1, m_dataFormat, m_dataType, data);
                stbi_image_free(data);
            }

            setDefaultParams();
            if (generateMipmap) glGenerateMipmap(GL_TEXTURE_2D_ARRAY);

            unbind();
            logger->info("Texture2DArray '{}' loaded successfully", m_name);
            return true;
    }

    void Texture2DArray::bind(){
        glBindTexture(GL_TEXTURE_2D_ARRAY, textureID);
    }

    void Texture2DArray::unbind(){
        glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
    }

    void Texture2DArray::enable(Shader& shader, const std::string& uniformName, int pos){
        glActiveTexture(GL_TEXTURE0 + pos);
        bind();
        shader.setInt(uniformName.c_str(), pos);
    }

    void Texture2DArray::disable(){
        
    }

    void Texture2DArray::setDefaultParams(){
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
        if (generateMipmap) {
            glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        } else {
            glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        }
    }

    void Texture2DArray::setFormatFromChannels(int channels) {
        switch (channels) {
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
            default:
                logger->warn("Texture2DArray '{}' unknown channel count: {}", m_name, channels);
        }
    }

}