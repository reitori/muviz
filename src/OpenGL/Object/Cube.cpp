#include "OpenGL/Object/Cube.h"

namespace viz{
    std::vector<SimpleVertex> SimpleCubeVertices = {
        // Front Vertices //Color
        {glm::vec3(-1.0f, -1.0f,  1.0f), glm::vec4(0.3921f, 0.3921f, 0.3921f, 0.05f)},
        {glm::vec3(1.0f, -1.0f,  1.0f), glm::vec4(0.3921f, 0.3921f, 0.3921f, 0.05f)},
        {glm::vec3(1.0f,  1.0f,  1.0f), glm::vec4(0.3921f, 0.3921f, 0.3921f, 0.05f)},
        {glm::vec3(-1.0f,  1.0f,  1.0f), glm::vec4(0.3921f, 0.3921f, 0.3921f,0.05f)},
        // Back Vertices  //Color
        {glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec4(0.3921f, 0.3921f, 0.3921f, 0.05f)},
        {glm::vec3(1.0f, -1.0f, -1.0f), glm::vec4(0.3921f, 0.3921f, 0.3921f, 0.05f)},
        {glm::vec3(1.0f,  1.0f, -1.0f), glm::vec4(0.3921f, 0.3921f, 0.3921f, 0.05f)},
        {glm::vec3(-1.0f,  1.0f, -1.0f), glm::vec4(0.3921f, 0.3921f, 0.3921f, 0.05f)}
    };

    std::vector<GLuint> SimpleCubeIndices = {
        // front
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

    // 24 vertices (4 per face) with face normals
    std::vector<Vertex> CubeVertices = {
        // Front (+Z)
        {{-1,-1, 1}, {0,0, 1}, {0,0}}, // bottom-left
        {{ 1,-1, 1}, {0,0, 1}, {1,0}}, // bottom-right
        {{ 1, 1, 1}, {0,0, 1}, {1,1}}, // top-right
        {{-1, 1, 1}, {0,0, 1}, {0,1}}, // top-left

        // Back (-Z)
        {{ 1,-1,-1}, {0,0,-1}, {0,0}}, // bottom-left
        {{-1,-1,-1}, {0,0,-1}, {1,0}}, // bottom-right
        {{-1, 1,-1}, {0,0,-1}, {1,1}}, // top-right
        {{ 1, 1,-1}, {0,0,-1}, {0,1}}, // top-left

        // Right (+X)
        {{ 1,-1, 1}, {1,0,0}, {0,0}}, // bottom-left
        {{ 1,-1,-1}, {1,0,0}, {1,0}}, // bottom-right
        {{ 1, 1,-1}, {1,0,0}, {1,1}}, // top-right
        {{ 1, 1, 1}, {1,0,0}, {0,1}}, // top-left

        // Left (-X)
        {{-1,-1,-1}, {-1,0,0}, {0,0}}, // bottom-left
        {{-1,-1, 1}, {-1,0,0}, {1,0}}, // bottom-right
        {{-1, 1, 1}, {-1,0,0}, {1,1}}, // top-right
        {{-1, 1,-1}, {-1,0,0}, {0,1}}, // top-left

        // Top (+Y)
        {{-1, 1, 1}, {0,1,0}, {0,0}}, // bottom-left
        {{ 1, 1, 1}, {0,1,0}, {1,0}}, // bottom-right
        {{ 1, 1,-1}, {0,1,0}, {1,1}}, // top-right
        {{-1, 1,-1}, {0,1,0}, {0,1}}, // top-left

        // Bottom (-Y)
        {{-1,-1,-1}, {0,-1,0}, {0,0}}, // bottom-left
        {{ 1,-1,-1}, {0,-1,0}, {1,0}}, // bottom-right
        {{ 1,-1, 1}, {0,-1,0}, {1,1}}, // top-right
        {{-1,-1, 1}, {0,-1,0}, {0,1}}, // top-left
    };

    // 36 indices (6 faces × 2 triangles × 3 vertices)
    std::vector<GLuint> CubeIndices = {
        // Front (+Z)
        0, 1, 2, 2, 3, 0,
        // Back (-Z)
        4, 5, 6, 6, 7, 4,
        // Right (+X)
        8, 9,10,10,11, 8,
        // Left (-X)
        12,13,14,14,15,12,
        // Top (+Y)
        16,17,18,18,19,16,
        // Bottom (-Y)
        20,21,22,22,23,20
    };
}