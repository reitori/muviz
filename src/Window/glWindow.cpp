#include "Window/glWindow.h"

namespace viz{
    void glWindow::windowSizeCallback(int width, int height){
        m_width = (std::uint16_t)width;
        m_height = (std::uint16_t)height;
    }

    glWindow::glWindow(std::uint16_t width, std::uint16_t height, const char* name) : Window(width, height){
        //TODO: Check initialization status the Application flags
        m_window = glfwCreateWindow(width, height, name, NULL, NULL);
        if(m_window == NULL){
            m_appLogger->error("Failed to create GLFW window");
            glfwDestroyWindow(m_window);
        }
        else{
            GLFWwindow* currWin = glfwGetCurrentContext();
            glfwMakeContextCurrent(m_window);

            //Workaround to make glfw callbacks methods of specific objects rather than global functions
            glfwSetWindowUserPointer(m_window, this);
            auto lambsizecallback = [](GLFWwindow* window, int callwidth, int callheight){
                static_cast<glWindow*>(glfwGetWindowUserPointer(window))->windowSizeCallback(callwidth, callheight);
            };
            glfwSetWindowSizeCallback(m_window, lambsizecallback);

            glfwMakeContextCurrent(currWin);
        }
    }

    void glWindow::render(){
        glfwMakeContextCurrent(m_window);

        glfwPollEvents();
        glfwSwapBuffers(m_window);
        glClear(GL_COLOR_BUFFER_BIT);
    }


    glWindow::~glWindow(){
        glfwDestroyWindow(m_window);
    }
}