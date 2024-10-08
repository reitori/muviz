#ifndef _UTIL_HPP_
#define _UTIL_HPP_

#include <json.hpp>
#include <vector>
#include <string>

// Mostly the defaults, but using 32 instead of 64 bit int/float
using json=nlohmann::basic_json<std::map, std::vector, std::string, bool, std::int32_t, std::uint32_t, float>;

#endif
