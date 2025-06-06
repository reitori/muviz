#include "OpenGL/Scene/Mesh.h"

namespace viz{
    void SimpleMesh::init(){
        glGenVertexArrays(1, &m_VAO);
        glBindVertexArray(m_VAO);
        
        std::size_t vec4Size = sizeof(glm::vec4);

        glGenBuffers(1, &m_instancesVBO);
        glBindBuffer(GL_ARRAY_BUFFER, m_instancesVBO);

        glEnableVertexAttribArray(2); //color
        glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(InstanceData), (void*)0);
        glEnableVertexAttribArray(3); //transform matrix
        glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(InstanceData), (void*)vec4Size);
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(InstanceData), (void*)(2*vec4Size));
        glEnableVertexAttribArray(5);   
        glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(InstanceData), (void*)(3*vec4Size));
        glEnableVertexAttribArray(6);
        glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(InstanceData), (void*)(4*vec4Size));

        glVertexAttribDivisor(2, 1); //color
        glVertexAttribDivisor(3, 1); //transform matrix
        glVertexAttribDivisor(4, 1);
        glVertexAttribDivisor(5, 1);
        glVertexAttribDivisor(6, 1);
        
        glGenBuffers(1, &m_VBO);
        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
        glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(SimpleVertex), &m_vertices[0], GL_STATIC_DRAW);

        glGenBuffers(1, &m_EBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(GLuint), &m_indices[0], GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(SimpleVertex), (void*)0);

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(SimpleVertex), (void*)(3 * sizeof(float)));

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        m_isInit = true;
    }

    SimpleMesh::SimpleMesh(const std::vector<SimpleVertex>& vertices, const std::vector<GLuint>& indices, bool isInstancedRendered){
        m_vertices = vertices;
        m_indices = indices;
        m_isInstancedRendered = isInstancedRendered;

        init();
    }

    void SimpleMesh::setData(const std::vector<SimpleVertex>& vertices, const std::vector<GLuint>& indices, bool isInstancedRendered){
        m_vertices = vertices;
        m_indices = indices;
        m_isInstancedRendered = isInstancedRendered;

        if(!m_isInit){
            init();
            m_isInit = true;
            return;
        }

        glBindVertexArray(m_VAO);
        glBindBuffer(GL_VERTEX_ARRAY, m_VBO);
        glBufferData(GL_VERTEX_ARRAY, m_vertices.size() * sizeof(SimpleVertex), &m_vertices[0], GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(GLuint), &m_indices[0], GL_STATIC_DRAW);

        glBindVertexArray(0);
        glBindBuffer(GL_VERTEX_ARRAY, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);   
    }

    void SimpleMesh::allocateInstances(std::uint16_t instances){
        glBindVertexArray(m_VAO);
        glBindBuffer(GL_ARRAY_BUFFER, m_instancesVBO);
        glBufferData(GL_ARRAY_BUFFER, instances * sizeof(InstanceData), nullptr, GL_STREAM_DRAW);

        glBindVertexArray(0);
    }

    void SimpleMesh::setInstances(const std::vector<InstanceData>&& instances){
        m_instancesToRender = instances.size();
        std::size_t vec4Size = sizeof(glm::vec4);

        m_instances = std::move(instances);
        updateInstances();
    }

    void SimpleMesh::updateInstances(){
        m_instancesToRender = m_instances.size();

        glBindVertexArray(m_VAO);
        glBindBuffer(GL_ARRAY_BUFFER, m_instancesVBO);
        glBufferData(GL_ARRAY_BUFFER, static_cast<GLuint>(m_instancesToRender * sizeof(InstanceData)), NULL, GL_STREAM_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, static_cast<GLuint>(m_instancesToRender * sizeof(InstanceData)), m_instances.data());

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    
    void SimpleMesh::render(const Shader& shader) const {
        shader.use();
        glBindVertexArray(m_VAO);
        if(!m_isInstancedRendered){
            shader.setBool("uIsInstanced", false);
            glDrawElements(GL_TRIANGLES, static_cast<GLuint>(m_indices.size()), GL_UNSIGNED_INT, (GLvoid*)0);
        }else{
            shader.setBool("uIsInstanced", true);
            glDrawElementsInstanced(GL_TRIANGLES, static_cast<GLuint>(m_indices.size()), GL_UNSIGNED_INT, (GLvoid*)0, m_instancesToRender);
        }
        glBindVertexArray(0);
        return;
    }
}