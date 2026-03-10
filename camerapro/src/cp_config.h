#ifndef CLT_OPTIC_CP_CONFIG_H
#define CLT_OPTIC_CP_CONFIG_H

#include "m_cfg_if.h"
#include <m_json_cfg_reader.h>
#include <m_json_unique_ptr.h>
#include <string>

namespace cam_pro {
class Config {
public:
  explicit Config(const std::string &filename, int cameraIndex);
  // prohibit copy ctors and operators
  Config(const Config &) = delete;
  Config &operator=(const Config &) = delete;

  ipc_name_cfg ipc_cfg;

  struct {
    int bufferCount = 4;
    int retryWait = 10;
  } ipc;

  // parameters
  int cameraIndex = 0;
  struct {
    std::string type;
    int reconnectWait = 500;
    int delayms = 0; // Camera delay in milliseconds
    const json_t *cameraJson = nullptr;
  } camera;

  struct {
    std::string address;
    std::string port;
    int reconnectIntervalms = 1000;
    int dataTimeotms = 2000;
  } turret;

  struct {
    bool display = false;
  } display;

private:
  std::string readConfigFile(const std::string &filename);
  const std::string &load(const std::string &config);
  json_unique_ptr m_root;

  void loadIpc(json_t *parent);
  void loadCamera(json_t *parent);
  void loadDisplay(json_t *parent);
  void loadTurret(json_t *parent);
};

} // namespace cam_pro

#endif // CLT_OPTIC_CP_CONFIG_H
