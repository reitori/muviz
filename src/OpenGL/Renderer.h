#ifndef RENDERER_H
#define RENDERER_H

#include "core/header.h"

#include "OpenGL/Framebuffer.h"
#include "OpenGL/ShaderManager.h"
#include "OpenGL/Shader.h"
#include "OpenGL/Camera.h"
#include "OpenGL/Scene/Detector.h"

#include "Events/Event.h"

#include <glm/glm.hpp>
#include <chrono>

namespace viz{

    //TODO: Refactor of Renderer engine to support an ECS-design pattern
    //      Why: Makes it easier to create new/future objects with certain properties
    //           decouples properties from these objects so that they are applied to objects
    //           rather than tightly bound to their internals
    //      Example: Entity Player takes on Components Light, Material, Camera, etc. Systems evalute these properties
    //      Look at TODO for Camera.h as another example

    class Renderer{
        public:
            Renderer(int width, int height);

            void init();
            void resize(int width, int height);

            void setColor(glm::vec4 color) {m_color = color;}
            void setCurrentCamera(std::string id);

            void addCurve(const std::shared_ptr<Curve>& curve);

            void attachDetector(std::shared_ptr<Detector> detetctor) { m_detector = detetctor; }

            void attachCamera(std::string name, const Camera& camera);
            Camera* getCamera() {return &m_cameras[m_currCam];}
            Camera* getCamera(std::string name);
            void destroyCamera(std::string name);

            inline std::uint16_t getWidth() const { return m_screenFramebuffer->getWidth();}
            inline std::uint16_t getHeight() const { return m_screenFramebuffer->getHeight();}
            inline GLuint getTexID() const { return m_screenFramebuffer->getTexID(); }

            const std::shared_ptr<Detector> getDetector() const { return m_detector; }
            
            void render();
            void sortTransparentObjects();

            double getDelTime() const { return delTime; }
        private:
            friend class Application;
            std::shared_ptr<Detector> m_detector;
            std::unique_ptr<Framebuffer> m_framebuffer;
            std::unique_ptr<Framebuffer> m_screenFramebuffer;
            std::vector<std::shared_ptr<Curve>> m_curves;

            std::string m_currCam;
            std::map<std::string, Camera> m_cameras;

            glm::vec4 m_color;

            glm::vec2 lastCursorPos;
            double lastTime = 0.0f, delTime = 0.0f;
    };
}

#endif