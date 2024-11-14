#ifndef VIZHEADER
#define VIZHEADER

#include <iostream>
#include <stdint.h>
#include "logging.h"

//Logger
namespace viz{
    extern std::shared_ptr<spdlog::logger> m_appLogger;
}

#endif