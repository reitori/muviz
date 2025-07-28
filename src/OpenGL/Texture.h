#ifndef TEXTURE_H
#define TEXTURE_H

#include <string>
#include <deque>

#include "stb_image.h"
#include "OpenGL/Shader.h"


namespace viz{
	class Texture{
		public:
			Texture() = default;
			void fromPath(const std::string path, const std::string name);

			Texture(const Texture& texture) = delete;
			Texture& operator=(const Texture&) = delete; 

			virtual void bindTexture() = 0;
			virtual void unbindTexture() = 0;
	
			virtual void enableTexture(Shader& shader, std::string& name, int pos) = 0;
			virtual void disableTexture() = 0;

			unsigned int getID() const {return textureID; }
			std::string getPath() const { return m_path; }
			std::string getName() const { return m_name; }

			~Texture() { glDeleteTextures(1, &textureID); }
		protected:
			virtual void loadTexture() = 0;

			GLenum type;
			unsigned int textureID = 0;
			std::string m_path, m_name;
	};

	class Texture2D : public Texture{
		public:
			Texture2D(const std::string path, const std::string name, bool mipmap = true, bool setMultisample = false);
			Texture2D(uint16_t width, uint16_t height, std::string name, bool mipmap = true, bool setMultisample = false);

			Texture2D(const Texture2D& texture) = delete;
			Texture2D& operator=(const Texture2D&) = delete; 

			void bindTexture() override;
			void unbindTexture() override;

			void enableTexture(Shader& shader, std::string& name, int pos) override;
			void disableTexture() override;

			void resize(int width, int height); //clears the texture and resizes it
			void changeFormat(GLenum internalFormat, GLenum dataFormat, GLenum dataType);

			unsigned char* getDataPointer() { return data;}
			int getChannels() const { return m_nrChannel;}

			inline uint16_t getWidth() {return m_width; }
			inline uint16_t getHeight() { return m_height; }
			
			uint32_t m_TextureUnit;
			uint8_t samples = 4;
			bool generateMipmap;
		protected:
			void loadTexture() override;
			void setDefaultParams();
			
			unsigned char* data = nullptr;
			int m_width, m_height, m_nrChannel;
			bool multisample = false;
			std::string m_path, m_name;
			GLenum m_internalFormat = GL_RGBA8;
            GLenum m_dataFormat = GL_RGBA;
			GLenum m_dataType = GL_UNSIGNED_BYTE;
	};
}

#endif