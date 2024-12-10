#ifndef IMGUIWINDOW_H
#define IMGUIWINDOW_H

#include "Window/Window.h"
#include "OpenGL/Renderer.h"


namespace viz{
    class renderWindow : public Window{
        private:
            Renderer m_Renderer;
        public:
            virtual void render() = 0;
    };
}

#endif