
#include "application.hpp"
#include "turretBuilder.hpp"
#include <chrono>
#include <cmath>
#include <iostream>
#include <logHelper.hpp>
#include <stdexcept>
#include <thread>

namespace turret {

static auto logger = log4cxx::Logger::getLogger("turretpro");

Application::Application(const Config &config)
    : m_config(config),
      m_controlServer(config.controlServer.host, config.controlServer.port,
                      m_commandsQueue),
      m_infoServer(config.infoServer.host, config.infoServer.port),
      m_turret(std::move(buildTurret(config))) {}

Application::~Application() {}

void Application::executeCommand(const Command &cmd) {
  if (!m_turret->connected()) {
    return;
  }

  switch (cmd.id()) {
  case Command::Stop: {
    m_turret->stop();
    break;
  }
  case Command::Goto: {
    const CmdGoto &cGoto = dynamic_cast<const CmdGoto &>(cmd);
    m_turret->moveto(cGoto.alpha(), cGoto.beta());
    break;
  }
  case Command::Speed: {
    const CmdSpeed &cSpeed = dynamic_cast<const CmdSpeed &>(cmd);
    m_turret->speed(cSpeed.alphaSpeed(), cSpeed.betaSpeed());
    break;
  }
  case Command::Track: {
    const CmdTrack &cTrack = dynamic_cast<const CmdTrack &>(cmd);
    m_turret->track(cTrack.time(), cTrack.alpha(), cTrack.beta(),
                    cTrack.alphaSpeed(), cTrack.betaSpeed(),
                    cTrack.alphaAcceleration(), cTrack.betaAcceleration());
    break;
  }
  case Command::SetPosition: {
    const CmdSetPosition &cPos = dynamic_cast<const CmdSetPosition &>(cmd);
    m_turret->moveto(cPos.alpha(), cPos.beta());
    break;
  }
  default: {
    LOG_WARN("Unknown command");
    break;
  }
  }
}

void Application::process() {
  if (!m_turret) {
    throw std::runtime_error("No turret");
  }
  // todo; thinkover state send
  m_infoServer.sendState(m_turret->state());
  // todo: think over! Send commands directly into turret thread?
  CommandPtr cmd = m_commandsQueue.get_ready();
  if (cmd) {
    executeCommand(*cmd.get());
  }
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  // todo:????std::chrono::milliseconds(m_config.turret.reconnectIntervalms));
}

int Application::run() {
  if (!m_turret) {
    // todo: application hangs if exception is thrown
    //        throw std::runtime_error("No turret loaded");
    return 1;
  }
  m_turret->run();
  m_infoServer.run();
  m_controlServer.run();

  try {
    while (m_run) {
      process();
    }
    m_infoServer.terminate();
    m_controlServer.terminate();
    m_turret->terminate();
  } catch (...) {
    m_infoServer.terminate();
    m_controlServer.terminate();
    m_turret->terminate();
    throw;
  }
  return 0;
}

} // namespace turret