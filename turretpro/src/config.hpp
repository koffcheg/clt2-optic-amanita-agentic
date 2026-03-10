#ifndef CLT_OPTIC_CP_CONFIG_H
#define CLT_OPTIC_CP_CONFIG_H

#include <jansson.h>
#include <m_json_cfg_reader.h>
#include <m_json_unique_ptr.h>
#include <string>

namespace turret {
class Config {
public:
  explicit Config(const std::string &filename);

  // prohibit copy ctors and assign operator
  Config(const Config &) = delete;
  Config &operator=(const Config &) = delete;

  // parameters
  struct {
    std::string host;
    std::string port;
  } infoServer;

  struct {
    std::string host;
    std::string port;
  } controlServer;

  struct Turret {
    int reconnectIntervalms = 500;
    std::string type;
    json_t *params = nullptr;
    //jansson_cfg_obj_reader jparams;
  } turret;

private:
  std::string readConfigFile(const std::string &filename);
  void loadInfoServer(json_t *parent);
  void loadControlServer(json_t *parent);
  void loadTurret(json_t *parent);

  json_unique_ptr m_root;
};
} // namespace turret

#endif // CLT_OPTIC_CP_CONFIG_H
