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
    int deltaTms = 20;
  } simulator;

  struct {
    int radius = 2;
  } target;

  struct {
    double width = 70;
    double height = 40;
  } field;

  struct 
  {
    bool enabled = false;
    std::string filename;
  }flog;
  

private:
  void loadSimulator(json_t *parent);
  void loadTarget(json_t *parent);
  void loadField(json_t *parent);
  void loadFlog(json_t *parent);

  json_unique_ptr m_root;
};
} // namespace turret

#endif // CLT_OPTIC_CP_CONFIG_H
