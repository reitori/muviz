#ifndef VIZHEADER
#define VIZHEADER

#include <iostream>
#include <stdint.h>
#include "logging.h"

#define viz_TO_RADIANS(x) (x * 0.01743f)

//Logger
namespace viz{
    extern std::shared_ptr<spdlog::logger> m_appLogger;
}

#endif