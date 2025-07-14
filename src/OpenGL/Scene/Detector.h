#ifndef DETECTOR_H
#define DETECTOR_H

#include "core/header.h"
#include "OpenGL/Scene/Mesh.h"
#include "OpenGL/Scene/Chip.h"

#include "Events/ParticleEvent.h"
#include "OpenGL/Camera.h"

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <chrono>

#include "cli.h"

#include "CircularBuffer.h"

namespace viz{
    extern std::vector<SimpleVertex> CubeVertices;
    extern std::vector<GLuint> CubeIndices;

    struct Particle{
        glm::vec3 pos;
        glm::mat4 transform;
        glm::vec4 color;
        
        bool is_immortal;
        bool isHit;
        float lifetime, ndcDepth;
    };

    class Detector{
        public:
            bool startCLI = false;

            Detector();

            void init(const std::shared_ptr<VisualizerCli>& cli);

            void update(const Camera& cam, const float& dTime);
            void updateHitDurations(float dur);
            void setEventCallback(const std::function<void(system::event& e)>& callback) { eventCallback = callback; }

            void render(const Shader& shader);
            void sortTransparent(const Camera& cam);
            std::vector<Chip>& getChips() { return m_chips;}

            uint32_t totHits(); 

            float hitDuration = 0.5f;
        private:
            void updateParticles(const Camera& cam, const float& dTime);
            void configure();
            int findUnusedParticle();
            std::uint32_t LastUsedParticle = 0;
            std::uint32_t totalParticles = 100000;

            glm::mat4 transform(const glm::vec3& scale, const glm::quat& args_quat, const glm::vec3& pos);
            glm::mat4 transform(glm::vec3 scale, glm::vec3 eulerRot, glm::vec3 pos, OrientationMode orientation, bool isInRadians = false);
            std::shared_ptr<VisualizerCli> m_cli;

            std::uint32_t nHitsThisFrame = 0; //number of collected hits in a given frame
            
            glm::mat4 m_transform;
            std::vector<Chip> m_chips;
            std::vector<Particle> ParticlesContainer; //Geometry -- All the chips and hits are rendered as particles until I fix the rendering architecture

            SimpleMesh CubeMesh;
            std::size_t startOfHitBuffer = 0;

            glm::vec3 hitScale = glm::vec3(0.05f, 0.05f, 0.05f);
            glm::vec3 defaultHitColor = glm::vec4(1.0f, 0.2509f, 0.0235f, 1.0f);

            std::function<void(system::event& e)> eventCallback;

            float totalupdatetime = 0.0f;
            float detectorLength;
            int totalupdateframes = 0;
    };
}

#endif