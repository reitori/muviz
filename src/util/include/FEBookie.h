#ifndef FEBOOKIE_H
#define FEBOOKIE_H

#include <stdint.h>
#include <unordered_map>
#include <string>

namespace viz{
    class FEBookie{
        public:
            FEBookie() = default;

            static void addFE(uint8_t fe_id, std::string name);
            static std::string getName(uint8_t fe_id);
        private:
            static std::unordered_map<uint8_t, std::string> internal_map;
    };
}

#endif