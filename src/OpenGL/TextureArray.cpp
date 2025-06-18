#include "OpenGL/TextureArray.h"

namespace viz{
    TextureArray::TextureArray(uint16_t width, uint16_t height, uint16_t layers, const std::string& name)
        : m_width(width), m_height(height), m_layers(layers), m_name(name), m_arrayID(0) {}

    TextureArray::~TextureArray() {
        if (m_arrayID) {
            glDeleteTextures(1, &m_arrayID);
            m_arrayID = 0;
        }
    }

    void TextureArray::build() {
        if (m_arrayID) glDeleteTextures(1, &m_arrayID);
        glGenTextures(1, &m_arrayID);
        bind();
        allocateStorage();
        for (uint16_t i = 0; i < m_layers; ++i) {
            uploadLayer(i);
        }
        unbind();
    }

    void TextureArray::bind() const {
        glBindTexture(GL_TEXTURE_2D_ARRAY, m_arrayID);
    }

    void TextureArray::unbind() const {
        glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
    }

    void TextureArray::enable(Shader& shader, const std::string& uniformName, int unit) const {
        glActiveTexture(GL_TEXTURE0 + unit);
        bind();
        shader.setInt(uniformName.c_str(), unit);
    }

    void TextureArray::disable() const {
        glActiveTexture(GL_TEXTURE0);
        unbind();
    }



    Texture2DArray::Texture2DArray(uint16_t width, uint16_t height, uint16_t layers, const std::string& name) : TextureArray(width, height, layers, name) {
        m_layersData.reserve(layers);
        for (uint16_t i = 0; i < layers; ++i) {
            m_layersData.emplace_back(std::make_unique<Texture2D>(width, height, name + "[" + std::to_string(i) + "]"));
        }
    }

    Texture2DArray::~Texture2DArray() = default;

    Texture2D& Texture2DArray::operator[](size_t index) {
        if (index >= m_layers) throw std::out_of_range("Texture2DArray index out of range");
        return *m_layersData[index];
    }

    const Texture2D& Texture2DArray::operator[](size_t index) const {
        if (index >= m_layers) throw std::out_of_range("Texture2DArray index out of range");
        return *m_layersData[index];
    }

    void Texture2DArray::allocateStorage() {
        int channels = getChannelCount();
        GLenum internalFormat = (channels == 4 ? GL_RGBA8 : GL_RGB8);
        glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, internalFormat, m_width, m_height, m_layers);
    }

    void Texture2DArray::uploadLayer(uint16_t index) {
        m_layersData[index] = std::make_unique<Texture2D>(m_width, m_height, m_name + "[" + std::to_string(index) + "]");
        int channels = getChannelCount();
        GLenum format = (channels == 4 ? GL_RGBA : GL_RGB);
        glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, index, m_width, m_height, 1, format, GL_UNSIGNED_BYTE,
                        m_layersData[index]->getDataPointer());
    }

    int Texture2DArray::getChannelCount() const {
        return m_layersData.empty() ? 3 : m_layersData[0]->getChannels();
    }
}