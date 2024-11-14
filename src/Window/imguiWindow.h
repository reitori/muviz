#ifndef IMGUIWINDOW_H
#define IMGUIWINDOW_H
#include "Window/Window.h"

namespace viz{
    class imguiWindow : public Window{
        private:

        public:
            virtual void render() = 0;
    };
}

#endif