#ifndef TEXTUREARRAY_H
#define TEXTUREARRAY_H

#include "OpenGL/Texture.h"

#include <string>

namespace viz{
    class TextureArray {
        public:
            TextureArray(uint16_t width, uint16_t height, uint16_t layers, const std::string& name);
            virtual ~TextureArray();

            void build();

            void bind() const;
            void unbind() const;

            void enable(Shader& shader, const std::string& uniformName, int unit) const;
            void disable() const;
        
            uint16_t getWidth() const { return m_width; }
            uint16_t getHeight() const { return m_height; }
            uint16_t getLayerCount() const { return m_layers; }
            const std::string& getName() const { return m_name; }
        
        protected:
            virtual void allocateStorage() = 0;
            virtual void uploadLayer(uint16_t index) = 0;
            virtual int getChannelCount() const = 0;
        
            uint16_t m_width;
            uint16_t m_height;
            uint16_t m_layers;
            std::string m_name;
            GLuint m_arrayID;
    };
    
    class Texture2DArray : public TextureArray {
        public:
            Texture2DArray(uint16_t width, uint16_t height, uint16_t layers, const std::string& name);
            ~Texture2DArray();
        
            // Access individual layer to load data
            Texture2D& operator[](size_t index);
            const Texture2D& operator[](size_t index) const;
        
        protected:
            void allocateStorage() override;
            void uploadLayer(uint16_t index) override;
            int getChannelCount() const override;
        
        private:
            std::vector<std::unique_ptr<Texture2D>> m_layersData;
    };        
}

#endif