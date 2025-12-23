#include "OpenGL/Camera.h"

namespace{
    auto logger = logging::make_log("Camera");
}

namespace viz{
    Camera::Camera(){
        data.projection = glm::mat4(1.0f);
        data.orientation = glm::mat4(1.0f);
        data.position = glm::vec3(0.0f, 0.0f, 0.0f);
        data.front = glm::vec3(0.0f, 0.0f, -1.0f);
        data.up = glm::vec3(0.0f, 1.0f, 0.0f);
        data.right = glm::vec3(1.0f, 0.0f, 0.0f);
        data.screenScale = glm::vec2(1.0f, 1.0f);
        data.screenSize = glm::vec2(1.0f, 1.0f);
        data.zoom = 1.0f;
        data.sensitivity = 1.0f;
    }

    void Camera::resize(int width, int height){
        data.screenSize.x = width; data.screenSize.y = height;
        calcProj();
    }

    void Camera::setPos(glm::vec3 pos) { 
        if(cameraLocked)
            return;
            
        data.position = pos; 
    }

    void Camera::setCamPath(const std::vector<CamPath>& paths){
        for(auto& path : fullPath){
            fullPath.push_back(path);
            pathTimeRanges.push_back({totalPathTime, totalPathTime + path.timeDuration, fullPath.size() - 1});
            totalPathTime += path.timeDuration;
        }
    }

    void Camera::pushCamPath(const CamPath& path){
        fullPath.push_back(path);
        pathTimeRanges.push_back({totalPathTime, totalPathTime + path.timeDuration, fullPath.size() - 1});
        totalPathTime += path.timeDuration;
    }

    void Camera::popCamPath(){
        if(!fullPath.empty()){
            totalPathTime -= fullPath.end()->timeDuration;
            fullPath.pop_back();
            pathTimeRanges.pop_back();
            if(currPathIndex >= fullPath.size()){
                currPathIndex = fullPath.size() - 1;
            }
        }   
    }

    void Camera::restartCamPath(){
        currPathIndex = 0;
        currPathTimeOffset = 0.0f;
        totalElapsedPathTime = 0.0f;
        currCyclePathDir = cyclePathDir::FORWARDS;
    }

    //TODO: Returns true when a cycle is completed. Do this by tracking how much distance camera has traveled along path
    bool Camera::cyclePath(float delTime){
        if(currCyclePathDir == cyclePathDir::FORWARDS){
            return pathForward(delTime);
        }
        else{
            return pathBackwards(delTime);
        }
    }

    bool Camera::pathForward(float delTime){
        if(fullPath.empty() || !enableCameraPath)
            return true;

        //Check to see if path is finished up
        totalElapsedPathTime += delTime;
        if(totalElapsedPathTime >= totalPathTime){
            calcFromCamPath(fullPath.back(), 1.0f);
            totalElapsedPathTime = totalPathTime;
            currPathIndex = fullPath.size() - 1;
            currCyclePathDir = cyclePathDir::BACKWARDS;
            return true;
        }

        auto& currCurve = fullPath[currPathIndex];
        float mapped_t = globalTimeFunc(totalElapsedPathTime / totalPathTime) * totalPathTime;

        //Check to see if we have finished up the current path and move to the next
        if(mapped_t - (globalTimeFunc(currPathTimeOffset / totalPathTime) * totalPathTime) > currCurve.timeDuration){
            currPathTimeOffset += currCurve.timeDuration;
            currPathIndex++;
            currCurve = fullPath[currPathIndex];
        }

        float normalizedCurrCurveTime = (mapped_t - (globalTimeFunc(currPathTimeOffset / totalPathTime) * totalPathTime)) / currCurve.timeDuration;
        calcFromCamPath(fullPath.at(currPathIndex), normalizedCurrCurveTime);

        return false;
    }

    bool Camera::pathBackwards(float delTime){
        if(fullPath.empty() || !enableCameraPath)
            return true;

        //Check to see if reverse path is finished
        totalElapsedPathTime -= delTime;
        if(totalElapsedPathTime < 0){
            calcFromCamPath(fullPath.front(), 0.0f);
            totalElapsedPathTime = 0.0f;
            currPathIndex = 0;
            currCyclePathDir = cyclePathDir::FORWARDS;
            return true;
        }

        auto& currCurve = fullPath[currPathIndex];
        float mapped_t = globalTimeFunc(totalElapsedPathTime / totalPathTime) * totalPathTime;

        if(mapped_t - (globalTimeFunc(currPathTimeOffset / totalPathTime) * totalPathTime) < 0){
            currPathIndex--;
            currCurve = fullPath[currPathIndex];
            currPathTimeOffset -= currCurve.timeDuration;
        }

        float normalizedCurrCurveTime = (mapped_t - (globalTimeFunc(currPathTimeOffset / totalPathTime) * totalPathTime)) / currCurve.timeDuration;
        calcFromCamPath(fullPath.at(currPathIndex), normalizedCurrCurveTime);

        return false;
    }

    //Treats fullPath as a paramaterized piecewise function of all of the CamPaths, where Domain=[0,1].
    //Domain of the piecewise function is split up into segments based on order in
    //which CamPaths are placed in fullPath as well as their associated timeDurations.
    //i.e. if fullPath = {CamPath5sec, CamPath10sec}
    //Then if normalizedPosition is in [0, 1/3], tracePath updates CameraOrientation based on CamPath5sec data
    //     if normalizedPosition is in (1/3, 1), tracePath updates CameraOrientation based on CamPath10sec data
    void Camera::tracePath(float normalizedPosition){
        if(fullPath.empty() || !enableCameraPath)
            return;

        if(normalizedPosition < 0 || normalizedPosition > 1)
            return;
        float totalTime = normalizedPosition * totalPathTime;

        auto it = std::lower_bound(pathTimeRanges.begin(), pathTimeRanges.end(), totalTime,
            [](const Range& r, float t) {
                return r.end <= t;
            });
        if (it != pathTimeRanges.end()){
            calcFromCamPath(fullPath[it->index], (totalTime - it->start) / fullPath[it->index].timeDuration);
        }
    }

    void Camera::displace(glm::vec3 disp) {
        if(cameraLocked)
            return;

        data.position += disp;
    }

    void Camera::displace(glm::vec3 direction, float delTime){
         if(cameraLocked)
            return;

        data.position += glm::normalize(direction) * speed * delTime;
    }

    void Camera::setRotation(float rightAngle, float upAngle){
        if(cameraLocked)
            return;

        float upAngleRad = viz_TO_RADIANS(upAngle);
        float rightAngleRad = viz_TO_RADIANS(rightAngle);

        float x = cos(upAngleRad) * sin(rightAngleRad);
        float y = sin(upAngleRad);
        float z = cos(upAngleRad) * cos(rightAngleRad);

        data.front = glm::vec3(x, y, z);
        updateOrientation();
    }

    void Camera::rotate(float dispRightAngle, float dispUpAngle) { 
        if(cameraLocked)
            return;

        float upAngleRad = data.sensitivity * viz_TO_RADIANS(dispRightAngle);
        float rightAngleRad = data.sensitivity * viz_TO_RADIANS(dispUpAngle);

        glm::vec3 x = (float)(cos(upAngleRad) * sin(rightAngleRad)) * data.right;
        glm::vec3 y = (float)sin(upAngleRad) * data.up;
        glm::vec3 z = (float)(cos(upAngleRad) * cos(rightAngleRad)) * data.front;

        glm::vec3 front = glm::normalize(x+y+z);

        //Change rotations so lock occurs
        if( glm::abs(glm::dot(front, glm::vec3(0.0f, 1.0f, 0.0f))) > 0.99f)
            data.front = glm::vec3(-data.front.x, data.front.y, -data.front.z);
        else{
            data.front = front;
            updateOrientation();
        }
    }

    void Camera::rotateAxis(glm::vec3 axis, float rotAngle){
        if(cameraLocked)
            return;

        float rotAngleRad = 2.0f * viz_TO_RADIANS(rotAngle);

        glm::quat q = glm::angleAxis(rotAngleRad, axis);
        glm::vec3 newFront = glm::rotate(q ,data.front);

        float smoothFactor = 0.5f;
        data.front = glm::mix(data.front, newFront, smoothFactor);

        updateOrientation();
    }    

    void Camera::addScroll(float scroll){
        if(cameraLocked)
            return;
        data.zoom += scroll;
        calcProj();
    }

    glm::mat4 Camera::getView() const{
        return data.orientation * glm::translate(glm::mat4(1.0f), -data.position);
    }
    
    glm::mat4 Camera::getProj() const{
        return data.projection;
    }

    void Camera::calcFromCamPath(const CamPath& path, float normalized_time){
        if(!path.position) //nullptr position and positive timeDuration => camera wait
            return;

        float mappedNormalizedTime = path.timeFunction(normalized_time);
        data.position = path.position->at(mappedNormalizedTime); 
        data.roll = path.rollAngleFunc(mappedNormalizedTime);
        if(path.lookAtCurve){
            data.front = glm::normalize(path.lookAtCurve->at(mappedNormalizedTime) - path.position->at(mappedNormalizedTime));
        }else{
            data.front = glm::normalize(path.lookAtPoint - path.position->at(mappedNormalizedTime));
        }

        if(!path.useWorldUp){
            if(path.fixedUpVector != glm::vec3(0.0f, 0.0f, 0.0f)){
                data.up = glm::normalize(path.fixedUpVector);
                data.right = glm::cross(data.front, data.up);
            }
            else{
                glm::mat4 rollMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(data.roll), data.front);
                data.right = rollMatrix * (glm::vec4(-glm::normalize(path.position->derivative(mappedNormalizedTime)), 1.0f));
                data.up = glm::cross(data.right, data.front);
            }
        }

        updateOrientation(path.useWorldUp);
    }

    void Camera::updateOrientation(bool useWorldUp){
        if(useWorldUp){
            glm::mat4 rollMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(data.roll), data.front);
            data.right = rollMatrix * glm::vec4(glm::normalize(glm::cross(data.front, worldUp)), 1.0f);
            data.up = glm::normalize(glm::cross(data.right, data.front));
        }

        //matrix is orthogonal with respect to normal dot product so transpose is inverse
        data.orientation[0][0] = data.right.x;
        data.orientation[1][0] = data.right.y;
        data.orientation[2][0] = data.right.z;
        data.orientation[0][1] = data.up.x;
        data.orientation[1][1] = data.up.y;
        data.orientation[2][1] = data.up.z;
        data.orientation[0][2] = -data.front.x;
        data.orientation[1][2] = -data.front.y;
        data.orientation[2][2] = -data.front.z;
    }
}