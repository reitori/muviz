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
        float zoom;
    };

    class Camera{
        public:
            CamData data;

            Camera();
            Camera(CamData cameraData) : data(cameraData) {}

            void resize(int width, int height);
            void setPos(glm::vec3 pos) { data.position = pos; }

            glm::mat4 getView() const;
    };
}

#endif