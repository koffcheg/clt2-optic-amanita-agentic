#include "config.hpp"
#include "common/model/m_json_cfg_reader.h"
#include <fstream>
#include <jansson.h>
#include <log4cxx/logger.h>
#include <stdexcept>

namespace turret {
Config::Config(const std::string &filename)
    : m_configStr{load(readConfigFile(filename))} {}

std::string Config::readConfigFile(const std::string &filename) {
  std::ifstream ifs(filename);
  if (!ifs) {
    // todo: log
    throw std::runtime_error("Can not open configuration file: " + filename);
  }

  std::string file_content((std::istreambuf_iterator<char>(ifs)),
                           (std::istreambuf_iterator<char>()));
  return file_content;
}

const std::string &Config::load(const std::string &config) {
  // Parse the JSON string into a JSON object
  json_error_t error;
  json_t *root = json_loads(config.c_str(), 0, &error);

  if (!root) {
    throw std::runtime_error("Invalid configuration file");
  }

  return config;
}

} // namespace turret
