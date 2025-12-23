#include "OpenGL/Cubemap.h"

namespace{
    auto logger = logging::make_log("Cubemap");
}

namespace viz{ 
    Cubemap::Cubemap(const std::vector<std::string>& paths, std::string name){
        m_paths = paths;
        m_name = name;

        glGenTextures(1, &textureID);
        loadFromPath(paths);
    }

    Cubemap::Cubemap(uint16_t width, uint16_t height, std::string name){
        m_width = width;
        m_height = height;
        m_name = name;
        
        glGenTextures(1, &textureID);

        bind();
        setDefaultParams();
        for(int i = 0; i < 6; i++){
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, m_internalFormat, m_width, m_height, 0, m_dataFormat, GL_UNSIGNED_BYTE, nullptr);
        }
        unbind();
    }

    bool Cubemap::loadFromPath(const std::vector<std::string>& paths){
        if (paths.empty()) {
            logger->error("Cubemap '{}' load failed: No paths provided", m_name);
            loadSuccessful = false;
            return false;
        }
        else if(paths.size() != 6){
            logger->error("Cubemap '{}' load failed: Provide 6 paths to load from", m_name);
            loadSuccessful = false;
            return false;
        }

        bind();
        stbi_set_flip_vertically_on_load(false);

        int w, h, channels;
        unsigned char* firstData = stbi_load(paths[0].c_str(), &w, &h, &channels, 0);
        if (!firstData) {
            logger->error("Cubemap '{}' failed to load layer 0: {}", m_name, paths[0]);
            loadSuccessful = false;
            return false;
        }
        setFormatFromChannels(channels);
        m_width = w;
        m_height = h;

        logger->info("Loading Cubemap '{}' ({}x{})", m_name, m_width, m_height);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, m_internalFormat, m_width, m_height, 0, m_dataFormat, GL_UNSIGNED_BYTE, firstData);
        stbi_image_free(firstData);

        for (int i = 1; i < 6; i++) {
            unsigned char* data = stbi_load(paths[i].c_str(), &w, &h, &channels, 0);
            if (!data) {
                logger->error("Cubemap '{}' failed to load layer {}: {}", m_name, i, paths[i]);
                continue;
            }
            if (w != m_width || h != m_height) {
                logger->warn("Cubemap '{}' layer {} size mismatch ({}x{}) - expected ({}x{})",
                                m_name, i, w, h, m_width, m_height);
                stbi_image_free(data);
                continue;
            }
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, m_internalFormat, m_width, m_height, 0, m_dataFormat, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }

        setDefaultParams();
        unbind();
        logger->info("Cubemap '{}' loaded successfully", m_name);
        loadSuccessful = true;
        return true;
    }

    void Cubemap::bind(){
        glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
    }

    void Cubemap::unbind(){
        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    }

    void Cubemap::enable(Shader& shader, const std::string& uniformName, int pos){
        glActiveTexture(GL_TEXTURE0 + pos);
        bind();
        shader.setInt(uniformName.c_str(), pos);
    }

    void Cubemap::setDefaultParams(){
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);  
    }

    void Cubemap::setFormatFromChannels(int channels) {
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

    Cubemap::~Cubemap(){
        glDeleteTextures(1, &textureID);
    }

}