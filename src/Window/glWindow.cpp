#include "Window/glWindow.h"

namespace viz{
    void glWindow::windowSizeCallback(int width, int height){
        m_width = (std::uint16_t)width;
        m_height = (std::uint16_t)height;
    }

    glWindow::glWindow(const char* name, std::uint16_t width, std::uint16_t height) : Window(name), m_width(width), m_height(height){
        //TODO: Check initialization status the Application flags
        m_window = glfwCreateWindow(width, height, name, NULL, NULL);
        if(m_window == NULL){
            m_appLogger->error("Failed to create GLFW window");
            glfwDestroyWindow(m_window);
        }
        else{
            glfwMakeContextCurrent(m_window);
            glfwSwapInterval(1);

            glfwSetWindowUserPointer(m_window, this);
            glfwSetKeyCallback(m_window, &key_callback);
			glfwSetCursorPosCallback(m_window, &mouse_pos_callback);
			glfwSetScrollCallback(m_window, &scroll_callback);
			glfwSetMouseButtonCallback(m_window, mouse_button_callback);
			glfwSetWindowSizeCallback(m_window, &window_resize_callback);
			glfwSetWindowCloseCallback(m_window, &window_close_callback);
        }
    }

    void glWindow::render(){

        glfwPollEvents();
        glfwSwapBuffers(m_window);
        glClear(GL_COLOR_BUFFER_BIT);
    }


    glWindow::~glWindow(){
        glfwDestroyWindow(m_window);
    }


    void glWindow::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
		glWindow* windowManager = static_cast<glWindow*>(glfwGetWindowUserPointer(window));
		switch (action) {
			case GLFW_PRESS:
			{
				viz::keyEventPressed keyEvent((viz::key::keyCodes)key);
				windowManager->eventCallback(keyEvent);
				break;
			}
			case GLFW_RELEASE: {
				viz::keyEventReleased keyEvent((viz::key::keyCodes)key);
				windowManager->eventCallback(keyEvent);
				break;
			}
			case GLFW_REPEAT: {
				viz::keyEventPressed keyEvent((viz::key::keyCodes)key);
				windowManager->eventCallback(keyEvent);
				break;
			}
		}
	}

	void glWindow::mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
		glWindow* windowManager = static_cast<glWindow*>(glfwGetWindowUserPointer(window));
		switch (action) {
			case GLFW_PRESS: {
				viz::mouseEventPressed mouseEvent((viz::mouse::mouseCodes)button);
				windowManager->eventCallback(mouseEvent);
				break;
			}
			case GLFW_RELEASE: {
				viz::mouseEventReleased mouseEvent((viz::mouse::mouseCodes)button);
				windowManager->eventCallback(mouseEvent);
				break;
			}
		}
	}

	void glWindow::scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
		glWindow* windowManager = static_cast<glWindow*>(glfwGetWindowUserPointer(window));
		viz::mouseEventScrolled m_event((const float)xoffset, (const float)yoffset);
		windowManager->eventCallback(m_event);
	}

	void glWindow::mouse_pos_callback(GLFWwindow* window, double xpos, double ypos) {
		glWindow* windowManager = static_cast<glWindow*>(glfwGetWindowUserPointer(window));
		viz::mouseEventMoved m_event((const float)xpos, (const float)ypos);
		windowManager->eventCallback(m_event);
	}

	//Maybe later static void window_maximize_callback
	void glWindow::window_resize_callback(GLFWwindow* window, int i_width, int i_height) {
		glWindow* windowManager = static_cast<glWindow*>(glfwGetWindowUserPointer(window));
		
		viz::windowEventResize wr_event(i_width, i_height);
		windowManager->eventCallback(wr_event);
	}

	void glWindow::window_close_callback(GLFWwindow* window) {
		glWindow* windowManager = static_cast<glWindow*>(glfwGetWindowUserPointer(window));
		glfwDestroyWindow(window);
		glfwTerminate();
		viz::windowEventClose wc_event;
		windowManager->eventCallback(wc_event);
	}

}