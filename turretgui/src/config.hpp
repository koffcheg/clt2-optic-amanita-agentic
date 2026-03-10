#ifndef CLT_OPTIC_CP_CONFIG_H
#define CLT_OPTIC_CP_CONFIG_H

#include <string>
#include <cinttypes>

namespace turret {
class Config {
public:
  explicit Config(const std::string &filename);
  // prohibit copy ctors and operators
  Config(const Config &) = delete;
  Config &operator=(const Config &) = delete;

  const std::string &configStr() const { return m_configStr; }

  // parameters

  std::string infoHost;
  std::string infoPort;

  std::string controlHost;
  std::string controlPort;

  uint16_t dp_data_port;

  struct Turret {
    int reconnectIntervalms = 500;
  } turret;

private:
  std::string readConfigFile(const std::string &filename);
  const std::string &load(const std::string &config);
  std::string m_configStr;
};
} // namespace turret

#endif // CLT_OPTIC_CP_CONFIG_H
