#include "OpenGL/Shader.h"
#include "logging.h"

namespace {
    auto logger = logging::make_log("Shader");
}

namespace viz{
    //Clarify whether you are compiling from a string or from a file in compileFromFile (true: from file; false: from string literal)
    Shader::Shader(bool compileFromFile, const std::string& name, const std::string& vertexPath, const std::string& fragmentPath, const std::string& geometryPath){
        m_name = name;
        
        if(compileFromFile){
            std::string vertexCode;
            std::string fragmentCode;
            std::string geometryCode;
            std::ifstream vShaderFile;
            std::ifstream fShaderFile;
            std::ifstream gShaderFile;
        
            vShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
            fShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
            gShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
            try 
            {
                vShaderFile.open(vertexPath);
                fShaderFile.open(fragmentPath);
                std::stringstream vShaderStream, fShaderStream;

                vShaderStream << vShaderFile.rdbuf();
                fShaderStream << fShaderFile.rdbuf();		

                vShaderFile.close();
                fShaderFile.close();

                vertexCode = vShaderStream.str();
                fragmentCode = fShaderStream.str();			

                if(!geometryPath.empty())
                {
                    gShaderFile.open(geometryPath);
                    std::stringstream gShaderStream;
                    gShaderStream << gShaderFile.rdbuf();
                    gShaderFile.close();
                    geometryCode = gShaderStream.str();
                }
            }
            catch (std::ifstream::failure& e)
            {
                logger->error("{0} Could not read from file: {1}", m_name, e.what());
            }
            const char* vShaderCode = vertexCode.c_str();
            const char * fShaderCode = fragmentCode.c_str();
            // compile shaders
            unsigned int vertex, fragment;
            // vertex shader
            vertex = glCreateShader(GL_VERTEX_SHADER);
            glShaderSource(vertex, 1, &vShaderCode, NULL);
            glCompileShader(vertex);
            checkCompileErrors(vertex, "VERTEX");
            // fragment Shader
            fragment = glCreateShader(GL_FRAGMENT_SHADER);
            glShaderSource(fragment, 1, &fShaderCode, NULL);
            glCompileShader(fragment);
            checkCompileErrors(fragment, "FRAGMENT");
            // if geometry shader is given, compile geometry shader
            unsigned int geometry;
            if(!geometryPath.empty())
            {
                const char * gShaderCode = geometryCode.c_str();
                geometry = glCreateShader(GL_GEOMETRY_SHADER);
                glShaderSource(geometry, 1, &gShaderCode, NULL);
                glCompileShader(geometry);
                checkCompileErrors(geometry, "GEOMETRY");
            }
            // shader Program
            m_id = glCreateProgram();
            glAttachShader(m_id, vertex);
            glAttachShader(m_id, fragment);
            if(!geometryPath.empty())
                glAttachShader(m_id, geometry);
            glLinkProgram(m_id);
            checkCompileErrors(m_id, "PROGRAM");
            // delete the shaders as they're linked into our program now and no longer necessary
            glDeleteShader(vertex);
            glDeleteShader(fragment);
            if(!geometryPath.empty())
                glDeleteShader(geometry);
        }

        else{
            unsigned int vertex, fragment;
            const char* vertexSource = vertexPath.c_str();
            const char* fragmentSource = fragmentPath.c_str();
            // vertex shader
            vertex = glCreateShader(GL_VERTEX_SHADER);
            glShaderSource(vertex, 1, &vertexSource, NULL);
            glCompileShader(vertex);
            checkCompileErrors(vertex, "VERTEX");
            // fragment Shader
            fragment = glCreateShader(GL_FRAGMENT_SHADER);
            glShaderSource(fragment, 1, &fragmentSource, NULL);
            glCompileShader(fragment);
            checkCompileErrors(fragment, "FRAGMENT");
            // if geometry shader is given, compile geometry shader
            unsigned int geometry;
            if(!geometryPath.empty())
            {
                const char* geometrySource = geometryPath.c_str();
                geometry = glCreateShader(GL_GEOMETRY_SHADER);
                glShaderSource(geometry, 1, &geometrySource, NULL);
                glCompileShader(geometry);
                checkCompileErrors(geometry, "GEOMETRY");
            }
            // shader Program
            m_id = glCreateProgram();
            glAttachShader(m_id, vertex);
            glAttachShader(m_id, fragment);
            if(!geometryPath.empty())
                glAttachShader(m_id, geometry);
            glLinkProgram(m_id);
            checkCompileErrors(m_id, "PROGRAM");
            // delete the shaders as they're linked into our program now and no longer necessary
            glDeleteShader(vertex);
            glDeleteShader(fragment);
            if(!geometryPath.empty())
                glDeleteShader(geometry);
        }
    }

    void Shader::checkCompileErrors(GLuint shader, const char* type)
    {
        GLint success;
        GLchar infoLog[1024];
        if(type != "PROGRAM")
        {
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if(!success)
            {
                glGetShaderInfoLog(shader, 1024, NULL, infoLog);
                logger->error("{2} {0} SHADER COMPILATION: {1}", type, infoLog, m_name);
            }
        }
        else
        {
            glGetProgramiv(shader, GL_LINK_STATUS, &success);
            if(!success)
            {
                glGetProgramInfoLog(shader, 1024, NULL, infoLog);
                logger->error("{2} {0} SHADER LINKING: {1}", type, infoLog, m_name);
            }
        }
    }
}