#ifndef MESH_H
#define MESH_H

#include "core/header.h"
#include "OpenGL/Shader.h"
#include <glm/glm.hpp>

//TODO: Abstract away SimpleMesh and NormalMesh to general Mesh class.
//      

namespace viz{
    struct SimpleVertex{
        glm::vec3 pos;
        glm::vec4 color;

        SimpleVertex(glm::vec3 vertexPosition, glm::vec4 vertexColor){ pos = vertexPosition; color = vertexColor;}
    };

    struct Vertex{
        glm::vec3 pos;
        glm::vec3 color;
        glm::vec2 UV;
        glm::vec2 texCoord;
    };

    enum VERTEX_TYPE{
        SIMPLE,
        NORMAL
    };


    class SimpleMesh{
        public:
            SimpleMesh() = default;
            SimpleMesh(const std::vector<SimpleVertex>& vertices, const std::vector<GLuint>& indices, bool isInstancedRendered);
            void setData(const std::vector<SimpleVertex>& vertices, const std::vector<GLuint>& indices, bool isInstancedRendered);

            void allocateInstances(std::uint16_t instances);
            void setInstances(std::uint16_t instances, const std::vector<glm::mat4>& transforms, const std::vector<glm::vec4>& colors);
            void setInstanceTransforms(const std::vector<glm::mat4>& transforms);
            void setInstanceColor(const std::vector<glm::vec4>& colors);

            void flagInstanceRendering() { m_isInstancedRendered = true; }
            void unflagInstanceRendereng() { m_isInstancedRendered = false; }

            void render(const Shader& shader);
        private:
            void init();
            bool m_isInit = false;

            bool m_isInstancedRendered;
            std::uint16_t m_numInstances;

            std::vector<SimpleVertex> m_vertices;
            std::vector<GLuint> m_indices;
            //Add support for textures

            GLuint m_VAO, m_VBO, m_EBO;
            GLuint m_transformsVBO, m_colorsVBO; //Instancing
            VERTEX_TYPE m_type = SIMPLE;
    };
}

#endif