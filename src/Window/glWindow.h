#ifndef GLWINDOW_H
#define GLWINDOW_H

#include "core/header.h"
#include "Window/Window.h"

#include "Events/Event.h"
#include "Events/ApplicationEvent.h"
#include "Events/KeyboardEvent.h"
#include "Events/MouseEvent.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace viz{
    //AppWindow already assumes that Application has initialized GLFW implementation
    //Do not create an AppWindow without creating Application
    //Wrapper class for GLFW windows
    //TODO: Decide whether or not to turn this into a Singleton or abstract further. 
    //      Will the application use only one GLFW window or is there any case where it could use multiple?


    class glWindow : public Window{
        public:
            glWindow() = delete;
            glWindow(const char* name, std::uint16_t width, std::uint16_t height);
            
            void render() override;
            void setEventCallback(const std::function<void(event& e)>& callback) { eventCallback = callback; }

            inline bool windowShouldClose() const { return glfwWindowShouldClose(m_window); }
            inline const GLFWwindow* getGLFWWindow() const { return m_window; }
            inline void setAsContext() {glfwMakeContextCurrent(m_window);}
            inline void swapBuffers() {glfwSwapBuffers(m_window);}
            virtual ~glWindow();

        private:
            friend class Application; //Application should be able to access m_window
            GLFWwindow* m_window;
            
            std::uint16_t m_width;
            std::uint16_t m_height; 

            std::function<void(event& e)> eventCallback;

            void windowSizeCallback(int width, int height);
            static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
            static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
		    static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
            static void mouse_pos_callback(GLFWwindow* window, double xpos, double ypos);
		    static void window_resize_callback(GLFWwindow* window, int i_width, int i_height);
		    static void window_close_callback(GLFWwindow* window);
    };
}

#endif