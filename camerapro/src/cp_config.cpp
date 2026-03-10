#include "cp_config.h"
#include <fstream>
#include <log4cxx/logger.h>
#include <logHelper.hpp>
#include <stdexcept>

namespace cam_pro {

static auto logger = log4cxx::Logger::getLogger("config");

Config::Config(const std::string &filename, int cameraIndex)
    : ipc_cfg{cameraIndex}, cameraIndex{cameraIndex},
      m_root(json_unique_ptr_create(
          json_load_file(filename.c_str(), 0, nullptr))) {
  if (!m_root) {
    LOG_FATAL("Invalid configuration file");
    throw std::runtime_error("Invalid configuration file");
  }

  loadIpc(m_root.get());
  loadCamera(m_root.get());
  loadTurret(m_root.get());
  loadDisplay(m_root.get());
}

void Config::loadIpc(json_t *parent) {
  jansson_cfg_obj_reader reader(parent, "ipc");
  ipc.bufferCount = reader.read_int_param("bufferCount");
  ipc.retryWait = reader.read_int_param("retryWait", ipc.retryWait);
}

void Config::loadCamera(json_t *parent) {
  jansson_cfg_obj_reader cameraReader(parent, "camera");

  jansson_cfg_obj_reader reader(cameraReader.root(),
                                std::to_string(cameraIndex).c_str());

  camera.type = reader.read_string_param("type");
  camera.reconnectWait =
      reader.read_int_param("reconnectWait", camera.reconnectWait);
  camera.cameraJson = reader.root();

  camera.delayms = reader.read_int_param("delay_ms", camera.delayms);
}

void Config::loadDisplay(json_t *parent) {
  jansson_cfg_obj_reader reader(parent, "display");
  display.display = reader.read_bool_param("display", display.display);
}

void Config::loadTurret(json_t *parent) {
  jansson_cfg_obj_reader reader(parent, "turret");
  turret.address = reader.read_string_param("address");
  turret.port = reader.read_string_param("port");
  turret.reconnectIntervalms = reader.read_int_param("reconnectIntervalms");
  turret.dataTimeotms = reader.read_int_param("dataTimeotms");
}

} // namespace cam_pro
