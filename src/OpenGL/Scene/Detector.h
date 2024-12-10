#ifndef ENTITY_H
#define ENTITY_H


#include "core/header.h"
#include "OpenGL/Scene/Mesh.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "cli.h"

namespace viz{

    std::vector<SimpleVertex> ChipVertices = {
        // Front Vertices //Color
        {glm::vec3(-1.0f, -1.0f,  1.0f), glm::vec4(0.3921f, 0.3921f, 0.3921f, 1.0f)},
        {glm::vec3(1.0f, -1.0f,  1.0f), glm::vec4(0.3921f, 0.3921f, 0.3921f, 1.0f)},
        {glm::vec3(1.0f,  1.0f,  1.0f), glm::vec4(0.3921f, 0.3921f, 0.3921f, 1.0f)},
        {glm::vec3(-1.0f,  1.0f,  1.0f), glm::vec4(0.3921f, 0.3921f, 0.3921f, 1.0f)},
        // Back Vertices  //Color
        {glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec4(0.3921f, 0.3921f, 0.3921f, 1.0f)},
        {glm::vec3(1.0f, -1.0f, -1.0f), glm::vec4(0.3921f, 0.3921f, 0.3921f, 1.0f)},
        {glm::vec3(1.0f,  1.0f, -1.0f), glm::vec4(0.3921f, 0.3921f, 0.3921f, 1.0f)},
        {glm::vec3(-1.0f,  1.0f, -1.0f), glm::vec4(0.3921f, 0.3921f, 0.3921f, 1.0f)}
    };

    std::vector<SimpleVertex> HitVertices = {
        // Front Vertices //Color
        {glm::vec3(-1.0f, -1.0f,  1.0f), glm::vec4(1.0f, 0.2509f, 0.0235f, 0.95f)},
        {glm::vec3(1.0f, -1.0f,  1.0f), glm::vec4(1.0f, 0.2509f, 0.0235f, 0.95f)},
        {glm::vec3(1.0f,  1.0f,  1.0f), glm::vec4(1.0f, 0.2509f, 0.0235f, 0.95f)},
        {glm::vec3(-1.0f,  1.0f,  1.0f), glm::vec4(1.0f, 0.2509f, 0.0235f, 0.95f)},
        // Back Vertices  //Color
        {glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec4(1.0f, 0.2509f, 0.0235f, 0.95f)},
        {glm::vec3(1.0f, -1.0f, -1.0f), glm::vec4(1.0f, 0.2509f, 0.0235f, 0.95f)},
        {glm::vec3(1.0f,  1.0f, -1.0f), glm::vec4(1.0f, 0.2509f, 0.0235f, 0.95f)},
        {glm::vec3(-1.0f,  1.0f, -1.0f), glm::vec4(1.0f, 0.2509f, 0.0235f, 0.95f)}
    };

    std::vector<GLuint> CubeIndices = {// front
        0, 1, 2,
        2, 3, 0,
        // right
        1, 5, 6,
        6, 2, 1,
        // back
        7, 6, 5,
        5, 4, 7,
        // left
        4, 0, 3,
        3, 7, 4,
        // bottom
        4, 5, 1,
        1, 0, 4,
        // top
        3, 2, 6,
        6, 7, 3
    };

    SimpleMesh ChipMesh(ChipVertices, CubeIndices, true);
    SimpleMesh HitMesh(HitVertices, CubeIndices, true);

    struct Chip{
        std::uint16_t fe_id, maxCols, maxRows;
        std::uint64_t hits;

        glm::vec3 pos, eulerRot;
        glm::vec3 scale;
    };

    class Detector{
        public:
            void init(const VisualizerCli& cli);

            void update(VisualizerCli& cli);
            void renderChips(const Shader& shader);
            void renderHits(const Shader& shader);

            uint32_t totHits();
        private:
            glm::mat4 transform(glm::vec3 scale, glm::vec3 eulerRot, glm::vec3 pos, bool isInRadians = false);

            GLuint m_instBufID; 
            std::uint32_t m_size = 0; //per frame basis
            std::uint16_t m_nfe;

            glm::mat4 m_transform;
            std::vector<Chip> m_chips;
            std::vector<glm::mat4> m_chipTransforms, m_Hits; //stored separately from within Chip to speed up data reading when passing into buffers
            std::vector<glm::vec4> m_chipColors;

            glm::vec3 hitScale = glm::vec3(0.05f, 0.05f, 0.05f);
    };
}

#endif