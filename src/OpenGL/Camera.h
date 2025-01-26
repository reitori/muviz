#ifndef CAMERA_H
#define CAMERA_H

#include "core/header.h"
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace viz{
    struct CamData{
        glm::mat4 projection, orientation;
        glm::vec3 position;
        glm::vec3 front, right, up;
        glm::vec2 screenScale;
        glm::vec2 screenSize;
        float zoom;
    };

    class Camera{
        public:
            bool cameraLocked = false;

            Camera();
            Camera(CamData cameraData) : data(cameraData) {}

            void resize(int width, int height);
            void setPos(glm::vec3 pos);

            void displace(glm::vec3 disp);
            void setRotation(float rightAngle, float upAngle);
            void rotate(float dispRightAngle, float dispUpAngle); //rotate over an angle
            void addScroll(float scroll);

            const CamData& getCamData() const { return data; }

            glm::vec3 getFront() const { return data.front;}
            glm::vec3 getUp() const { return data.up; }
            glm::vec3 getRight() const { return data.right; }

            glm::mat4 getView() const;
        private:
            CamData data;

            void calcProj() { data.projection = glm::perspective(glm::radians(data.zoom * 45.0f), ((float)data.screenSize.x * data.screenScale.x) / ((float)data.screenSize.y * data.screenScale.y), 0.5f, 500.0f);}
            inline void updateOrientation();
    };
}

#endif