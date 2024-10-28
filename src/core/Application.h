#ifndef APPLICATION_H
#define APPLICATION_H

#include "header.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Window/glWindow.h"

namespace viz{
    namespace{
        auto logger = logging::make_log("VisualizerCLI");
    }

    class Application{
        private:
            static bool coreInit; //At some point maybe change this to bit flags indicating which API's are initialized ex: 011 logger init true, glfw init true, glad init true
            static void GLFWErrorCallback(int err, const char* message){ logger->error("GLFW Code {0}: {1}", err, message); }
            glWindow* appWin; //TODO: Change this to a list of windows. mainWin is a window that is rendered last

        public:
            Application(); //Sets application to fullscreen upon initialization
            Application(int width, int height); //Specify dimensions

            static inline bool isInit() {return coreInit;}
            void run();

            ~Application() = default;
    };
}

#endif