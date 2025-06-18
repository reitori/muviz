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
        public:
            Shader() = delete;
            Shader(bool compileFromFile, const std::string& name, const std::string& vertexPath, const std::string& fragmentPath, const std::string& geometryPath = "");

            inline void use() const {glUseProgram(m_id);}

            inline void setBool(const char* name, bool value) const{         
                glUniform1i(glGetUniformLocation(m_id, name), (int)value); 
            }

            inline void setInt(const char* name, int value) const{ 
                glUniform1i(glGetUniformLocation(m_id, name), value); 
            }

            inline void setFloat(const char* name, float value) const{ 
                glUniform1f(glGetUniformLocation(m_id, name), value); 
            }

            inline void setVec2(const char* name, const glm::vec2 &value) const{ 
                glUniform2fv(glGetUniformLocation(m_id, name), 1, &value[0]); 
            }
            inline void setVec2(const char* name, float x, float y) const{ 
                glUniform2f(glGetUniformLocation(m_id, name), x, y); 
            }

            inline void setVec3(const char* name, const glm::vec3 &value) const{ 
                glUniform3fv(glGetUniformLocation(m_id, name), 1, &value[0]); 
            }
            inline void setVec3(const char* name, float x, float y, float z) const{ 
                glUniform3f(glGetUniformLocation(m_id, name), x, y, z); 
            }

            inline void setVec4(const char* name, const glm::vec4 &value) const{ 
                glUniform4fv(glGetUniformLocation(m_id, name), 1, &value[0]); 
            }
            inline void setVec4(const char* name, float x, float y, float z, float w) const{ 
                glUniform4f(glGetUniformLocation(m_id, name), x, y, z, w); 
            }

            inline void setMat2(const char* name, const glm::mat2 &mat) const{
                glUniformMatrix2fv(glGetUniformLocation(m_id, name), 1, GL_FALSE, &mat[0][0]);
            }

            inline void setMat3(const char* name, const glm::mat3 &mat) const{
                glUniformMatrix3fv(glGetUniformLocation(m_id, name), 1, GL_FALSE, &mat[0][0]);
            }

            inline void setMat4(const char* name, const glm::mat4 &mat) const{
                glUniformMatrix4fv(glGetUniformLocation(m_id, name), 1, GL_FALSE, &mat[0][0]);
            }

            ~Shader() {glDeleteShader(m_id);}

         private:
            friend class ShaderManager;

            GLuint m_id;
            std::string m_name;

            void checkCompileErrors(GLuint shader, const char* type);
    };
}

#endif