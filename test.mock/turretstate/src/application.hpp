
#pragma once

#include "turretInfoClient.hpp"
#include <memory>
#include <string>

namespace turret {

class Application {
public:
  explicit Application(const std::string &infoHost,
                       const std::string &infoPort);
  ~Application();
  int run();

  bool terminated() { return !m_run; };
  void terminate() { m_run = false; };

private:
  void processConnect();
  void processConnected();

  TurretInfoClient m_infoClient;

  bool m_run = true;

  int m_screen = 0;
};

} // namespace turret