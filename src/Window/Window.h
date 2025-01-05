#ifndef WINDOW_H
#define WINDOW_H

#include "core/header.h"


namespace viz{
    class Window{
        protected:
            std::string m_name;

        public:
            Window() = delete;
            Window(const char* name) : m_name(name) {}

            virtual void render() = 0;

            //virtual void OnEvent() will be necessary once we figure out events
            virtual ~Window() = default;
    };
}

#endif