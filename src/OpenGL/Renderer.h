#ifndef RENDERER_H
#define RENDERER_H

#include "core/header.h"

#include "OpenGL/Framebuffer.h"
#include "OpenGL/Shader.h"
#include "OpenGL/Camera.h"
#include "OpenGL/Scene/Detector.h"

#include "Events/Event.h"

#include <glm/glm.hpp>
#include <chrono>

namespace viz{
    class Renderer{
        public:
            Renderer(int width, int height);

            void init();
            void resize(int width, int height);

            void setColor(glm::vec4 color) {m_color = color;}
            void setCurrentCamera(std::string id);

            void attachDetector(std::shared_ptr<Detector> detetctor) { m_detector = detetctor; }

            void attachCamera(std::string name, const Camera& camera);
            Camera* getCamera() {return &m_Cameras[m_currCam];}
            Camera* getCamera(std::string name);
            void destroyCamera(std::string name);

            inline std::uint16_t getWidth() const { return m_framebuffer->getWidth();}
            inline std::uint16_t getHeight() const { return m_framebuffer->getHeight();}
            inline GLuint getTexID() const { return m_framebuffer->getTexID(); }

            const std::shared_ptr<Detector> getDetector() const { return m_detector; }
            
            void render();
            void sortTransparentObjects();

            double getDelTime() const { return delTime; }
        private:
            friend class Application;
            std::shared_ptr<Detector> m_detector;
            std::unique_ptr<Framebuffer> m_framebuffer;

            std::uint8_t m_activeShaderNum;
            std::vector<Shader> m_Shaders;

            std::string m_currCam;
            std::map<std::string, Camera> m_Cameras;

            glm::vec4 m_color;

            glm::vec2 lastCursorPos;
            std::unique_ptr<SimpleMesh> testCube;

            double lastTime = 0.0f, delTime = 0.0f;
    };
}

#endif