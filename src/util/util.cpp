#include "util/include/util.hpp"

namespace viz{
    std::filesystem::path getExecutableDir() {
        char buffer[1024];
        #if defined(_WIN32)
            GetModuleFileNameA(NULL, buffer, sizeof(buffer));
        #elif defined(__APPLE__)
            uint32_t size = sizeof(buffer);
            _NSGetExecutablePath(buffer, &size);
        #elif defined(__linux__)
            ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer));
            buffer[len] = '\0';
        #endif
            return std::filesystem::path(buffer).parent_path();
    }
}