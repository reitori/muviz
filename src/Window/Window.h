#ifndef WINDOW_H
#define WINDOW_H

#include "header.h"

namespace{
     auto logger = logging::make_log("WindowLogger");
}


namespace viz{
    //Wrapper for windows (just GLFW and ImGUI for now)
    class Window{
        protected:
            std::uint16_t m_width;
            std::uint16_t m_height;

        public:
            Window() = delete;
            Window(std::uint16_t width, std::uint16_t height) : m_width(width), m_height(height) {}

            inline uint16_t getWidth() const {return m_width;}
            inline uint16_t getHeight() const {return m_height;}

            // virtual void setWidth() = 0; Probably not necessary now or maybe never
            // virtual void setHeight() = 0;
            virtual void render() = 0;

            //virtual void OnEvent() will be necessary once we figure out events
            virtual ~Window() = default;
    };
}

#endif