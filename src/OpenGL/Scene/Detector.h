#ifndef ENTITY_H
#define ENTITY_H


#include "core/header.h"
#include "OpenGL/Scene/Mesh.h"

#include "Events/ParticleEvent.h"
#include "OpenGL/Camera.h"

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "cli.h"

#include "CircularBuffer.h"

namespace viz{
    extern std::vector<SimpleVertex> ChipVertices;
    extern std::vector<SimpleVertex> HitVertices;
    extern std::vector<GLuint> CubeIndices;

    struct Chip{
        std::string name;
        std::uint16_t fe_id, maxCols, maxRows;
        std::uint64_t hits;

        glm::vec3 pos, eulerRot;
        glm::vec3 scale;
    };

    class Detector{
        public:
            bool startCLI = false;

            Detector();

            void init(const std::shared_ptr<VisualizerCli>& cli);

            void update();
            void setEventCallback(const std::function<void(event& e)>& callback) { eventCallback = callback; }

            void render(const Shader& shader);
            void sortTransparent(const Camera& cam);
            std::vector<Chip> getChips() const { return m_chips;}

            uint32_t totHits();
        private:
            glm::mat4 transform(glm::vec3 scale, glm::vec3 eulerRot, glm::vec3 pos, bool isInRadians = false);
            std::shared_ptr<VisualizerCli> m_cli;

            GLuint m_instBufID; 
            std::uint32_t m_size = 0; //per frame basis
            std::uint16_t m_nfe;

            std::uint32_t nHits = 0;

            //CLI state is RECONSTRUCTED
            //CircularBuffer<ReconstructedBunch> circularEventBuffer; //Moving window of reconstructed events are displayed at a time
            std::vector<ReconstructedBunch> eventBuffer; //Indefinite number of reconstructed events displayed
            
            glm::mat4 m_transform;
            std::vector<Chip> m_chips;

            SimpleMesh ChipMesh, HitMesh;
            std::vector<glm::mat4> m_chipTransforms, m_hitTransforms; //stored separately from within Chip to speed up data reading when passing into buffers
            std::vector<glm::vec4> m_chipColors, m_hitColors;

            std::size_t startOfHitBuffer = 0;

            glm::vec3 hitScale = glm::vec3(0.05f, 0.05f, 0.05f);

            std::function<void(event& e)> eventCallback;
    };
}

#endif