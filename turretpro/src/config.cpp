#include "config.hpp"
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
  loadInfoServer(m_root.get());
  loadControlServer(m_root.get());
  loadTurret(m_root.get());
}

void Config::loadInfoServer(json_t *parent) {
  jansson_cfg_obj_reader reader(parent, "infoServer");
  infoServer.host = reader.read_string_param("host");
  infoServer.port = reader.read_string_param("port");
}

void Config::loadControlServer(json_t *parent) {
  jansson_cfg_obj_reader reader(parent, "controlServer");
  controlServer.host = reader.read_string_param("host");
  controlServer.port = reader.read_string_param("port");
}

void Config::loadTurret(json_t *parent) {
  jansson_cfg_obj_reader reader(parent, "turret");
  turret.reconnectIntervalms = reader.read_int_param("reconnectIntervalms");
  turret.type = reader.read_string_param("type");

  //json_t *jTurret = json_object_get(parent, "turret");
  //turret.params = json_object_get(jTurret, turret.type.c_str());
  turret.params = json_object_get(parent, "turret");
}

} // namespace turret
