#ifndef MESH_H
#define MESH_H

#include "core/header.h"
#include "OpenGL/Shader.h"
#include <glm/glm.hpp>

//TODO: Allocate a maximum instances count; use glSubBufferData rather than glBufferData in SimpleMesh (this should optimize rendering)

namespace viz{
    struct InstanceData{
        glm::vec4 color;
        glm::mat4 transform = glm::mat4(1.0f);
    };

    struct Vertex{
        glm::vec3 pos;
        glm::vec3 normal;
        glm::vec2 texCoord;

    };

    class Mesh{
        public:
            Mesh() = default;
            Mesh(const std::vector<Vertex>& vertices, const std::vector<GLuint>& indices);
            void setData(const std::vector<Vertex>& vertices, const std::vector<GLuint>& indices);

            void allocateInstances(size_t numInstances);
            void setInstances(const std::vector<InstanceData>&& instances_vector);
            void setMeshModelData(const InstanceData& data);
            void updateInstances();

            void render() const;

            std::vector<InstanceData> instances;
        private:
            void init();
            std::vector<Vertex> m_vertices;
            std::vector<GLuint> m_indices;

            GLuint m_VAO, m_VBO, m_EBO;
            GLuint m_instancesVBO;
            bool m_isInit = false;
    };

    //Depracated -- useful only for debugging without textures
    struct SimpleVertex{
        glm::vec3 pos;
        glm::vec4 color;

        SimpleVertex(glm::vec3 vertexPosition, glm::vec4 vertexColor){ pos = vertexPosition; color = vertexColor;}
    };

    class SimpleMesh{
        public:
            SimpleMesh() = default;
            SimpleMesh(const std::vector<SimpleVertex>& vertices, const std::vector<GLuint>& indices, bool isInstancedRendered);
            void setData(const std::vector<SimpleVertex>& vertices, const std::vector<GLuint>& indices, bool isInstancedRendered);

            //Allocate instances and pass data to graphics pipeline
            void allocateInstances(std::uint16_t instances);
            void setInstances(const std::vector<InstanceData>&& instances); //
            void updateInstances();

            void setInstancedRendering(bool isInstancedRendered) { m_isInstancedRendered = isInstancedRendered; }

            void render(const Shader& shader) const;

            //Interface with instance data. Must call updateInstances() to pass new data to graphics pipeline
            std::vector<InstanceData> m_instances;
        private:
            void init();
            bool m_isInit = false;

            bool m_isInstancedRendered;
            std::uint16_t m_instancesToRender = 0;

            std::vector<SimpleVertex> m_vertices;
            std::vector<GLuint> m_indices;
            //Add support for textures

            GLuint m_VAO, m_VBO, m_EBO;
            GLuint m_instancesVBO; //Instancing
    };

}

#endif