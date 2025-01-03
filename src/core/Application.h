#ifndef APPLICATION_H
#define APPLICATION_H

#include "header.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imgui_internal.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Window/glWindow.h"
#include "Window/GUIWindows.h"

#include "Events/Event.h"

#include "cli.h"

namespace viz{
    class Application{
        public:
            Application(int argc, char** argv); //Sets application to fullscreen upon initialization

            Application(int argc, char** argv, int width, int height); //Specify dimensions

            inline bool isInit() {return coreInit;}

            void run();
            void onEvent(event& e);

            ~Application();

        private:
            void cliInit(int& argc, char**& argv);
            void glInit(bool fullscreen, int width = 100, int height = 100);
            void imGuiInit();

            void init();

            bool coreInit; //At some point maybe change this to bit flags indicating which API's are initialized ex: 011 logger init true, glfw init true, glad init true
            static void GLFWErrorCallback(int err, const char* message){ m_appLogger->error("GLFW Code {0}: {1}", err, message); } // Should be for the glfw window to handle

            VisualizerCli m_cli;
            std::unique_ptr<glWindow> m_appWin;
            std::shared_ptr<Detector> m_detector;
            std::shared_ptr<Renderer> m_renderer;
            std::vector<std::unique_ptr<GUIWindow>> m_GUIWindows;      
    };
}

#endif