#ifndef CAMERA_H
#define CAMERA_H

#include "core/header.h"
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace viz{
    struct CamData{
        glm::mat4 projection;
        glm::vec3 position;
        glm::quat orientation;
        glm::vec2 screenScale;
        glm::vec2 screenSize;
        float zoom;
    };

    class Camera{
        public:
            CamData data;
            bool cameraLocked = false;

            Camera();
            Camera(CamData cameraData) : data(cameraData) {}

            void resize(int width, int height);
            void setPos(glm::vec3 pos);

            void displace(glm::vec3 disp);
            void rotate(glm::vec3 rot);
            void addScroll(float scroll);

            glm::vec3 getFront() const { return glm::normalize(glm::mat4_cast(data.orientation) * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f));}
            glm::vec3 getUp() const { return glm::normalize(glm::mat4_cast(data.orientation) * glm::vec4(0.0f, 1.0f, 0.0f, 0.0f));}
            glm::vec3 getRight() const { return glm::normalize(glm::mat4_cast(data.orientation) * glm::vec4(1.0f, 0.0f, 0.0f, 0.0f));}

            glm::mat4 getView() const;

        private:
            void calcProj() { data.projection = glm::perspective(glm::radians(data.zoom * 45.0f), ((float)data.screenSize.x * data.screenScale.x) / ((float)data.screenSize.y * data.screenScale.y), 0.5f, 500.0f);}
    };
}

#endif