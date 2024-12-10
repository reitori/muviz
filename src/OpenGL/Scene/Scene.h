#ifndef SCENE_H
#define SCENE_H

#include "core/header.h"
#include "OpenGL/Camera.h"
#include "OpenGL/Scene/Mesh.h"

namespace viz{
    class Scene{
        public:

        private:
            std::vector<Camera> m_Cameras;

    };
}

#endif