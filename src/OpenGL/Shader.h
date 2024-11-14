#ifndef SHADER_H
#define SHADER_H

#include "core/header.h"

#include <string>
#include <fstream>
#include <sstream>

#include <glad/glad.h>
#include <glm/glm.hpp>

namespace viz{
    class Shader{
        private:
            uint16_t m_id;
            
            void checkCompileErrors(GLuint shader, const char* type);

        public:
            Shader() = delete;
            Shader(bool compileFromFile, const char* vertexPath, const char* fragmentPath, const char* geometryPath = nullptr);

            inline void use() const {glUseProgram(m_id);}

            inline void setBool(const char* name, bool value){         
                glUniform1i(glGetUniformLocation(m_id, name), (int)value); 
            }

            inline void setInt(const char* name, int value){ 
                glUniform1i(glGetUniformLocation(m_id, name), value); 
            }

            inline void setFloat(const char* name, float value){ 
                glUniform1f(glGetUniformLocation(m_id, name), value); 
            }

            inline void setVec2(const char* name, const glm::vec2 &value) { 
                glUniform2fv(glGetUniformLocation(m_id, name), 1, &value[0]); 
            }
            inline void setVec2(const char* name, float x, float y){ 
                glUniform2f(glGetUniformLocation(m_id, name), x, y); 
            }

            inline void setVec3(const char* name, const glm::vec3 &value){ 
                glUniform3fv(glGetUniformLocation(m_id, name), 1, &value[0]); 
            }
            inline void setVec3(const char* name, float x, float y, float z){ 
                glUniform3f(glGetUniformLocation(m_id, name), x, y, z); 
            }

            inline void setVec4(const char* name, const glm::vec4 &value){ 
                glUniform4fv(glGetUniformLocation(m_id, name), 1, &value[0]); 
            }
            inline void setVec4(const char* name, float x, float y, float z, float w){ 
                glUniform4f(glGetUniformLocation(m_id, name), x, y, z, w); 
            }

            inline void setMat2(const char* name, const glm::mat2 &mat){
                glUniformMatrix2fv(glGetUniformLocation(m_id, name), 1, GL_FALSE, &mat[0][0]);
            }

            inline void setMat3(const char* name, const glm::mat3 &mat){
                glUniformMatrix3fv(glGetUniformLocation(m_id, name), 1, GL_FALSE, &mat[0][0]);
            }

            inline void setMat4(const char* name, const glm::mat4 &mat) {
                glUniformMatrix4fv(glGetUniformLocation(m_id, name), 1, GL_FALSE, &mat[0][0]);
            }
    };
}

#endif