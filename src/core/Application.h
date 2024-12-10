#ifndef APPLICATION_H
#define APPLICATION_H

#include "header.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Window/glWindow.h"

#include "cli.h"

namespace viz{
    class Application{
        public:
            Application(); //Sets application to fullscreen upon initialization
            Application(int width, int height); //Specify dimensions

            static inline bool isInit() {return coreInit;}
            void run();

            ~Application();

        private:
            void cliInit();
            void glfwInit(int width, int height);
            void imGuiInit();
            void dockspaceInit();

            static bool coreInit = true; //At some point maybe change this to bit flags indicating which API's are initialized ex: 011 logger init true, glfw init true, glad init true
            static void GLFWErrorCallback(int err, const char* message){ logger->error("GLFW Code {0}: {1}", err, message); } // Should be for the glfw window to handle

            VisualizerCli m_cli;
            glWindow* appWin; //TODO: Change this to a list of windows. mainWin is a window that is rendered last
    };
}

#endif