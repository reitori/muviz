#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include "header.h"

namespace viz
{
    class Framebuffer{
        private:
            uint16_t m_id;

        public:
            Framebuffer();

            void bind();
    };
}


#endif