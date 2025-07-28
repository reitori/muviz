#ifndef CAMERA_H
#define CAMERA_H

#include "core/header.h"
#include "util/include/logging.h"
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "OpenGL/Scene/Curve.h"

//TODO: Abstract CamPaths away from Camera. Should be general type Path. 
//      Why: Any entity should be able to follow a path/trajectory.
//      How: Need to refactor our renderer to accomodate for an ECS.
//      Example: Entity Object has Component Camera and Path
//               Camera system evaluates Camera related tasks
//               Path system evaluates path-following related tasks

namespace viz{
    struct CamData{
        glm::mat4 projection, orientation;
        glm::vec3 position;
        glm::vec3 front, right, up;
        glm::vec2 screenScale;
        glm::vec2 screenSize;
        float zoom, sensitivity, roll;
    };

    //All functions must be normalized (must be mappings f:[0,1]->[0,1])
    struct CamPath{
        std::shared_ptr<Curve> position = nullptr;
        std::shared_ptr<Curve> lookAtCurve = nullptr; //lookatCurve takes higher precendence than lookAtPoint
        glm::vec3 lookAtPoint = glm::vec3(0.0f, 0.0f, 0.0f);
        std::function<float(float)> rollAngleFunc = [](float t) { return 0; };
        glm::vec3 fixedUpVector = glm::vec3(0.0f, 0.0f, 0.0f);
        std::function<float(float)> timeFunction = [](float t){ return t; };
        float timeDuration = 1.0f;
        bool useWorldUp = false;
    };

    class Camera{
        public:
            bool cameraLocked = false;
            float speed = 50.0f;

            Camera();
            Camera(CamData cameraData) : data(cameraData) {}

            void resize(int width, int height);
            void setPos(glm::vec3 pos);

            void setCamPath(const std::vector<CamPath>& paths);
            void pushCamPath(const CamPath& path);
            void popCamPath();
            void restartCamPath();
            bool cyclePath(float delTime); //Move forwards/backwards across the path in a loop
            bool pathForward(float delTime); //Returns true when end of path is reached
            bool pathBackwards(float delTime); //Returns true when start of path is reached
            void tracePath(float normalizedPosition);

            void displace(glm::vec3 disp);
            void displace(glm::vec3 direction, float delTime);

            void setRotation(float rightAngle, float upAngle);
            void rotate(float dispRightAngle, float dispUpAngle); //rotate over an angle
            void rotateAxis(glm::vec3, float rotAngle); //rotate front of camera about an axis
            void addScroll(float scroll);
            

            void setRotSensitivity(float sensitivity) {data.sensitivity = sensitivity; }

            const CamData& getCamData() const { return data; }

            glm::vec3 getFront() const { return data.front;}
            glm::vec3 getUp() const { return data.up; }
            glm::vec3 getRight() const { return data.right; }

            glm::mat4 getView() const;
            glm::mat4 getProj() const;

            //Must be a strictly increasing function from [0,1]->[0,1]
            //Think either ease function or linear function
            //Global is set for linear function
            //Can do some interesting function compositions by changing global
            //and local (inside CamPath) timeFunctions
            std::function<float(float)> globalTimeFunc = [](float t){ return t; };
            bool enableCameraPath = true;
            enum cyclePathDir{
                FORWARDS, BACKWARDS
            };
            cyclePathDir currCyclePathDir = cyclePathDir::FORWARDS;

            //Describes pitch
            glm::vec3 worldUp = glm::vec3(0.0f, 0.0f, 1.0f);
        private:
            struct Range{
                float start;
                float end;
                size_t index;
            };

            CamData data;
            std::vector<CamPath> fullPath;
            std::vector<Range> pathTimeRanges; //first stores associated index of fullPath; second stores the range
            float totalElapsedPathTime = 0.0f;
            float currPathTimeOffset = 0.0f;
            float totalPathTime = 0.0f;
            size_t currPathIndex = 0;

            void calcFromCamPath(const CamPath& path, float normalized_time);
            void calcProj() { data.projection = glm::perspective(glm::radians(data.zoom * 45.0f), ((float)data.screenSize.x * data.screenScale.x) / ((float)data.screenSize.y * data.screenScale.y), 0.5f, 500.0f);}
            void updateOrientation(bool useWorldUp = true);
    };
}

#endif