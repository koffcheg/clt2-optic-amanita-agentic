
#pragma once

#include "config.hpp"
#include "controlServer.hpp"
#include "turretInfoServer.hpp"
#include "turret.hpp"
#include <memory>
#include <string>

namespace turret {

class Application {
public:
  explicit Application(const Config &config);
  ~Application();
  int run();

  bool terminated() { return !m_run; };
  void terminate() { m_run = false; };

private:
  void process();
  void executeCommand(const Command & cmd);

  const Config &m_config;
  
  CommandsQueue m_commandsQueue;
  ControlServer m_controlServer;
  TurretInfoServer m_infoServer;
  TurretPtr m_turret;

  bool m_run = true;
};

} // namespace turret