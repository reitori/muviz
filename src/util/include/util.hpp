#ifndef _UTIL_HPP_
#define _UTIL_HPP_

#include <json.hpp>
#include <vector>
#include <string>
#include <filesystem>
#include <chrono>

// Mostly the defaults, but using 32 instead of 64 bit int/float
using json=nlohmann::basic_json<std::map, std::vector, std::string, bool, std::int32_t, std::uint32_t, float>;

#if defined(_WIN32)
    #include <windows.h>
#elif defined(__APPLE__)
    #include <mach-o/dyld.h>
#elif defined(__linux__)
    #include <unistd.h>
#endif

extern std::chrono::steady_clock::time_point ApplicationStartTimePoint;

namespace viz{
    std::filesystem::path getExecutableDir();
}

#endif
