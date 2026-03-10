#include "config.hpp"
#include "utils.hpp"
#include <fstream>
#include <logHelper.hpp>
#include <stdexcept>

namespace turret {

static auto logger = log4cxx::Logger::getLogger("config");

Config::Config(const std::string &filename)
    : m_root(json_unique_ptr_create(
          json_load_file(filename.c_str(), 0, nullptr))) {
  if (!m_root) {
    LOG_FATAL("Invalid configuration file");
    throw std::runtime_error("Invalid configuration file");
  }
  loadSimulator(m_root.get());
  loadTarget(m_root.get());
  loadField(m_root.get());
  loadFlog(m_root.get());
}

void Config::loadSimulator(json_t *parent) {
  jansson_cfg_obj_reader reader(parent, "simulator");
  simulator.deltaTms = reader.read_int_param("deltaTms", simulator.deltaTms);
}

void Config::loadTarget(json_t *parent) {
  jansson_cfg_obj_reader reader(parent, "target");
  target.radius = reader.read_int_param("host", target.radius);
}

void Config::loadField(json_t *parent) {
  jansson_cfg_obj_reader reader(parent, "field");
  field.width = reader.read_double_param("width", field.width) * u::DEG;
  field.height = reader.read_double_param("height", field.height) * u::DEG;
}

void Config::loadFlog(json_t *parent) {
  jansson_cfg_obj_reader reader(parent, "flog");
  flog.enabled = reader.read_bool_param("enabled", flog.enabled);
  flog.filename = reader.read_string_param("filename", flog.filename);
}

} // namespace turret
