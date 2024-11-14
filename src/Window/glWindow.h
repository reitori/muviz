#ifndef GLWINDOW_H
#define GLWINDOW_H

#include "core/header.h"
#include "Window/window.h"

#include <GLFW/glfw3.h>

namespace viz{
    //AppWindow already assumes that Application has initialized GLFW implementation
    //Do not create an AppWindow without creating Application
    //Wrapper class for GLFW windows
    //TODO: Decide whether or not to turn this into a Singleton or abstract further. 
    //      Will the application use only one GLFW window or is there any case where it could use multiple?


    class glWindow : public Window{
        private:
            friend class Application; //Application should be able to access m_window
            GLFWwindow* m_window;
            void windowSizeCallback(int width, int height);
            //void windowMaximizeCallback(GLFWwindow* window, int maximized); Not necessary now but could be later

        public:
            glWindow() = delete;
            glWindow(std::uint16_t width, std::uint16_t height, const char* name);

            void render() override;

            inline const GLFWwindow* getGLFWWindow() const { return m_window; }
            inline void setAsContext() {glfwMakeContextCurrent(m_window);}
            virtual ~glWindow();
    };
}

#endif GLWINDOW_H