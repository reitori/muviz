#ifndef LOGGING_H
#define LOGGING_H

#include <string>
#include <memory>
#include <vector>
#include <string>

#include "util.hpp"
#include "logging.h"
#include "spdlog/logger.h"
#include "spdlog/spdlog.h"

namespace logging {
typedef spdlog::logger Logger;
typedef std::shared_ptr<spdlog::logger> LoggerStore;

// Make a logger without any sinks
inline std::shared_ptr<spdlog::logger> make_log(std::string name) {
  auto log = spdlog::get(name);
  if(log == nullptr)
  {
    auto tmp = std::make_shared<spdlog::logger>(name, spdlog::sinks_init_list{});
    spdlog::register_logger(tmp);
    return tmp;
  }
  return log;
}
std::vector<std::string> getLog(std::size_t lim);

/// Setup loggers according to configuration in json file
void setupLoggers(const json &j, const std::string &path="");

/// List loggers to std::cout, with details of sinks and levels
void listLoggers(bool print_details = false);
static bool initialized = false;

// get messages from ringbuffer sink
std::vector<std::string> getLog(size_t lim);


void banner(std::shared_ptr<spdlog::logger> &logger, const std::string &msg);

}
#endif
