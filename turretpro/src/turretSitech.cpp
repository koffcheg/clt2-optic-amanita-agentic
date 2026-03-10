#include "turretSitech.hpp"
#include "utils.hpp"
#include <algorithm>
#include <chrono>
#include <fmt/chrono.h>
#include <fmt/format.h>
#include <iostream>
#include <limits>
#include <logHelper.hpp>
#include <m_json_cfg_reader.h>
#include <math.h>

using namespace u;

namespace turret {

static auto logger = log4cxx::Logger::getLogger("turret.sitech");

// TurretSitech

TurretSitech::Config::Config(const json_t *config) {
  if (!config) {
    throw std::runtime_error(
        "TurretSitech not configured. No turret.sitech section");
  }

  jansson_cfg_obj_reader reader(config, "sitech");

  port = reader.read_string_param("port");
  baudRate = reader.read_int_param("baudRate");
  commandAdvancems =
      reader.read_int_param("commandAdvancems", commandAdvancems);
  commandAdvanceSpeedms =
      reader.read_int_param("commandAdvanceSpeedms", commandAdvanceSpeedms);
  connectionTimeoutms =
      reader.read_int_param("connectionTimeoutms", connectionTimeoutms);
  reconnectIntevalms =
      reader.read_int_param("reconnectIntevalms", reconnectIntevalms);
  cyclems = reader.read_int_param("cyclems", cyclems);
  trackTimeout = reader.read_int_param("trackTimeout", trackTimeout);
  stateDebug = reader.read_bool_param("stateDebug", stateDebug);
  stateDebugFile = reader.read_string_param("stateDebugFile", stateDebugFile);

  std::string axesNames[axesCount] = {"alpha", "beta"};
  for (int a = 0; a < axesCount; a++) {
    jansson_cfg_obj_reader axisReader(reader.root(), axesNames[a].c_str());
    axes[a].PPR = axisReader.read_int_param("PPR");
    axes[a].gotoSpeed = axisReader.read_double_param("gotoSpeed") * DEG;
    axes[a].setSpeedCorrectionCoefficient = axisReader.read_double_param(
        "setSpeedCorrectionCoefficient", axes[a].setSpeedCorrectionCoefficient);
    axes[a].errVCoef =
        axisReader.read_double_param("errVCoef", axes[a].errVCoef);
    axes[a].tau = axisReader.read_double_param("tau", axes[a].tau);

    axes[a].pid.p = axisReader.read_double_param("pid.p", axes[a].pid.p);
    axes[a].pid.i = axisReader.read_double_param("pid.i", axes[a].pid.i);
    axes[a].pid.d = axisReader.read_double_param("pid.d", axes[a].pid.d);
    axes[a].pid.v = axisReader.read_double_param("pid.v", axes[a].pid.v);
    axes[a].pid.maxOut =
        axisReader.read_double_param("pid.maxOut", axes[a].pid.maxOut) * DEG;
    axes[a].pid.deccelerationZone =
        axisReader.read_double_param("pid.deccelerationZone",
                                     axes[a].pid.deccelerationZone) *
        DEG;
    axes[a].pid.maxAcceleration =
        axisReader.read_double_param("pid.maxAcceleration",
                                     axes[a].pid.maxAcceleration) *
        DEG;
    axes[a].pid.minAccOutLimit =
        axisReader.read_double_param("pid.minAccOutLimit",
                                     axes[a].pid.minAccOutLimit) *
        DEG;
    axes[a].pid.integralLimit =
        axisReader.read_double_param("pid.integralLimit",
                                     axes[a].pid.integralLimit) *
        DEG;

    axes[a].pidDebug = axisReader.read_bool_param("pidDebug", false);
    axes[a].pidDebugFile = axisReader.read_string_param(
        "pidDebugFile", "pid" + axesNames[a] + ".log");
  }
}

TurretSitech::TurretSitech(const json_t *config)
    : Turret(), m_config(config), pid{PID(m_config.axes[0].pid),
                                      PID(m_config.axes[1].pid)},
      m_pidLog{
          PidLogger(m_config.axes[0].pidDebugFile, m_config.axes[0].pidDebug),
          PidLogger(m_config.axes[1].pidDebugFile, m_config.axes[1].pidDebug)},
      m_stateLog(m_config.stateDebugFile, m_config.stateDebug),
      m_serial(m_config.port, m_config.baudRate, 8, 'N', 1) {
  LOG_INFO("TurretSitech created: port=" << m_config.port
                                         << " baudRate=" << m_config.baudRate);
}

void TurretSitech::stop() {
  m_commands.push_back(std::make_unique<CmdStop>());
  LOG_INFO("");
}

void TurretSitech::moveto(double alpha, double beta) {
  m_commands.push_back(std::make_unique<CmdGoto>(alpha, beta));
  LOG_INFO(alpha / DEG << ", " << beta / DEG);
}

void TurretSitech::speed(double alphaSpeed, double betaSpeed) {
  m_commands.push_back(std::make_unique<CmdSpeed>(alphaSpeed, betaSpeed));
  LOG_INFO(alphaSpeed / DEG << ", " << betaSpeed / DEG);
}

void TurretSitech::track(DateTime time, double alpha, double beta,
                         double alphaSpeed, double betaSpeed,
                         double alphaAcceleration, double betaAcceleration) {
  m_commands.push_back(std::make_unique<CmdTrack>(time, alpha, beta, alphaSpeed,
                                                  betaSpeed, alphaAcceleration,
                                                  betaAcceleration));
  LOG_INFO(alpha / DEG << ", " << beta / DEG << ", " << alphaSpeed / DEG << ", "
                       << betaSpeed / DEG << ", " << alphaAcceleration / DEG
                       << ", " << betaAcceleration / DEG);
}

void TurretSitech::setPosition(double alpha, double beta) {
  m_commands.push_back(std::make_unique<CmdSetPosition>(alpha, beta));
  LOG_INFO("" << alpha / DEG << ", " << beta / DEG);
}

void TurretSitech::sendCommandACS(const std::string &cmd) {
  char cs = 0;
  for (char c : cmd) {
    cs += c;
  }
  cs = ~cs;
  m_serial.ClearRx();
  m_serial.Write(cmd.c_str(), cmd.length());
  m_serial.Write(&cs, sizeof(cs));
  LOG_TRACE(cmd);
}

void TurretSitech::sendCommandRaw(const std::string &cmd) {
  m_serial.Write(cmd.c_str(), cmd.length());
  LOG_TRACE(cmd);
}

void TurretSitech::syncCommunication() {
  // send nonexistent command to exit possible XXR or YXR commands
  sendCommandRaw("x\r");
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  m_serial.ClearRx();
  // Enter ACS mode
  // additional x
  sendCommandRaw("YXY1\r\xb7");
}

void TurretSitech::requestStatus() {
  m_serial.ClearRx();
  sendCommandACS("XXS\r");
  m_requestSent = DateTimeClock::now() + std::chrono::milliseconds(3);
  m_statusRequested = true;
}

void TurretSitech::checkStop(int a, bool stoppedBit) {
  AxisState &axisState = m_setState.axes[a];
  int &stopCounter = m_axesState[a].stopCounter;
  if (axisState.mode == AxisMode::moveto) {
    if (stoppedBit) {
      if (stopCounter > 10) {
        axisState.mode = AxisMode::stopped;
        LOG_INFO(axisName(a) << " stopped!");
      } else {
        stopCounter++;
      }
    } else {
      stopCounter = 0;
    }
  } else {
    stopCounter = 0;
  }
}

void TurretSitech::processStatus(const SitechStatus &ss) {
  TurretState state = {};
  state.time = m_requestSent;
  // todo: controller time?
  state.online = true;
  TurretState setState = calculateSetPos(m_requestSent);
  const int *posRaw[axesCount] = {&ss.az, &ss.el};
  double dt = std::chrono::duration<double>(state.time - m_state.time).count();
  for (int a = 0; a < axesCount; a++) {
    AxisState &currentAS = state.axes[a]; // current axis state
    AxisState &setAS = setState.axes[a];
    AxisState &prevAS = m_state.axes[a];
    currentAS.position = 2.0 * M_PI * static_cast<double>(*posRaw[a]) /
                         static_cast<double>(m_config.axes[a].PPR);

    currentAS.speed = (m_config.axes[a].tau * prevAS.speed +
                       (currentAS.position - prevAS.position) / dt) /
                      (m_config.axes[a].tau + 1);
    currentAS.error = currentAS.position - setAS.position;
    currentAS.mode = setAS.mode; //?; //todo:
    currentAS.minLimit = 0;      //?; //todo:
    currentAS.maxLimit = 0;      //?; //todo:
    currentAS.driveError = 0;    // todo:
    currentAS.encoderError = 0;  // todo:
  }
  // std::cout<<ss.extraBits.yStopped<<ss.extraBits.xStopped<<std::endl;
  checkStop(0, ss.extraBits.yStopped);
  checkStop(1, ss.extraBits.xStopped);

  // todo: thread safety: two states and copy them under the mutex
  std::lock_guard<std::recursive_mutex> guard(m_mutex);
  m_state = state;
  m_stateLog.logState(m_state);
}

bool TurretSitech::decodeStatus() {
  char buff[41];
  int bytesRead = m_serial.ReadBuff(buff, sizeof(buff));
  if (bytesRead == sizeof(buff)) {
    LOG_TRACE("bytesRead=" << bytesRead);

    SitechStatus ss;
    bool success =
        ss.decode(reinterpret_cast<const unsigned char *>(buff), sizeof(buff));
    if (success) {
      processStatus(ss);
      m_lastStatus = std::chrono::steady_clock::now();
      if (!m_connected) {
        m_connected = true;
        LOG_INFO("Turret connected");
      }
      return true;
    } else {
      LOG_WARN("Invalid data");
    }
  } else {
    LOG_WARN("No enought data: " << bytesRead);
  }
  return false;
}

void TurretSitech::doStop() {
  LOG_DEBUG("");
  sendCommandACS("XN\r");
  // todo: move this pause to the sendCommandACS and make it adaptible
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  sendCommandACS("YN\r");
  m_setState.alpha.mode = AxisMode::stopped;
  m_setState.beta.mode = AxisMode::stopped;
}

void TurretSitech::executeCommands() {
  CommandPtr cmd = m_commands.get_ready();
  while (cmd) {
    switch (cmd->id()) {
    case Command::Stop: {
      LOG_DEBUG("stop");
      doStop();
      break;
    }
    case Command::Goto: {
      auto &c = *dynamic_cast<const CmdGoto *>(cmd.get());
      m_setState.alpha.mode = AxisMode::moveto;
      m_setState.beta.mode = AxisMode::moveto;
      m_setState.alpha.position = c.alpha();
      m_setState.beta.position = c.beta();
      LOG_DEBUG("goto: a=" << c.alpha() / DEG << "°, b=" << c.beta() / DEG
                           << "°");
      break;
    }
    case Command::Speed: {
      auto &c = *dynamic_cast<const CmdSpeed *>(cmd.get());
      m_setState.alpha.mode = AxisMode::speed;
      m_setState.beta.mode = AxisMode::speed;
      m_state0.alpha.position = m_state.alpha.position;
      m_state0.beta.position = m_state.beta.position;
      m_state0.alpha.speed = c.alphaSpeed();
      m_state0.beta.speed = c.betaSpeed();
      m_state0.time = DateTimeClock::now();
      LOG_DEBUG(fmt::format(
          "speed: a0={:.2}, b0={:.2}, va={:.3}, vb={:.3}, t0={:%H:%M:%S}",
          m_state0.alpha.position / DEG, m_state0.beta.position / DEG,
          m_state0.alpha.speed / DEG, m_state0.beta.speed / DEG,
          m_state0.time));
      break;
    }
    case Command::Track: {
      auto &c = *dynamic_cast<const CmdTrack *>(cmd.get());
      m_setState.alpha.mode = AxisMode::track;
      m_setState.beta.mode = AxisMode::track;
      m_lastTrackCommand = std::chrono::steady_clock::now();
#if (0)
      // todo: temp debug
      double dt =
          std::chrono::duration<double>(c.time() - m_state0.time).count();
      if (dt < 10) {
        m_state0.alpha.speed = (c.alpha() - m_state0.alpha.position) / dt;
        m_state0.beta.speed = (c.beta() - m_state0.beta.position) / dt;
      } else {
        m_state0.alpha.speed = m_config.axes[0].gotoSpeed;
        m_state0.beta.speed = m_config.axes[1].gotoSpeed;
      }
#else
      m_state0.alpha.speed = c.alphaSpeed();
      m_state0.beta.speed = c.betaSpeed();
#endif
      m_state0.alpha.position = c.alpha();
      m_state0.beta.position = c.beta();
      m_state0.time = c.time();
      LOG_DEBUG(fmt::format(
          "track: a0={:.2}, b0={:.2}, va={:.3}, vb={:.3}, t0={:%H:%M:%S}",
          m_state0.alpha.position / DEG, m_state0.beta.position / DEG,
          m_state0.alpha.speed / DEG, m_state0.beta.speed / DEG, c.time()));
      break;
    }
    case Command::SetPosition: {
      auto &c = *dynamic_cast<const CmdSetPosition *>(cmd.get());
      LOG_WARN("setpos not implemented:"
               << " a0=" << c.alpha() / DEG << ", b0=" << c.beta() / DEG);
      break;
    }
    }
    cmd = m_commands.get_ready(); // get the next command
    // todo: replace queue by some other mechanism
  }
}

double sign(double x) {
  if (x < 0) {
    return -1;
  }
  if (x > 0) {
    return 1;
  }
  return 0;
}

void TurretSitech::doControl() {
  SitechYXR yxr{};
  int *yPos[axesCount] = {&yxr.azPosSet, &yxr.elPosSet};
  int *ySpeed[axesCount] = {&yxr.azSpeedSet, &yxr.elSpeedSet};

  for (int a = 0; a < axesCount; a++) {
    switch (m_setState.axes[a].mode) {
    case AxisMode::stopped: {
      *yPos[a] = m_state.axes[a].position / 2.0 / M_PI * m_config.axes[a].PPR;
      break;
    }
    case AxisMode::moveto: {
      *yPos[a] =
          m_setState.axes[a].position / 2.0 / M_PI * m_config.axes[a].PPR;
      *ySpeed[a] = m_config.axes[a].gotoSpeed / 2.0 / M_PI *
                   m_config.axes[a].PPR * 65536.0 / cps *
                   m_config.axes[a].setSpeedCorrectionCoefficient;
      break;
    }
    case AxisMode::track: {
#if (1)
      // pid
      const double farFarAway = 100 * DEG;
      double e = -m_state.axes[a].error;
      double v = m_setStateSpeed.axes[a].speed;

      double controlAction = pid[a].process(e, v);
      m_pidLog[a].logPid(std::chrono::system_clock::now(), pid[a].status(), m_setState.axes[a].dt);

      *yPos[a] = (m_setState.axes[a].position + sign(controlAction) * farFarAway) / 2.0 / M_PI * m_config.axes[a].PPR;
      // e * m_config.axes[a].errVCoef ;
      *ySpeed[a] = (abs(controlAction)) / 2.0 / M_PI * m_config.axes[a].PPR *
                   65536.0 / cps *
                   m_config.axes[a].setSpeedCorrectionCoefficient;
      // todo:
      yxr.azRateAdderTime = 220;
      yxr.elRateAdderTime = 220;

#elif (0)
      const double farFarAway = 10 * DEG;
      double e = m_state.axes[a].error;
      double v = m_setState.axes[a].speed;
      // double i = m_setState.axes[a].speed;

      double controlAction = 0 * v + e * 0.5;

      *yPos[a] = (m_setState.axes[a].position - controlAction * farFarAway) /
                 2.0 / M_PI * m_config.axes[a].PPR;
      // e * m_config.axes[a].errVCoef ;
      *ySpeed[a] = (abs(controlAction)) / 2.0 / M_PI * m_config.axes[a].PPR *
                   65536.0 / cps *
                   m_config.axes[a].setSpeedCorrectionCoefficient;
      // todo:
      yxr.azRateAdderTime = 220;
      yxr.elRateAdderTime = 220;

#else
      //"goto" mode
      *yPos[a] =
          m_setState.axes[a].position / 2.0 / M_PI * m_config.axes[a].PPR;
      double e = fabs(m_state.axes[a].error);
      e = e * 10; // m_config.axes[a].errVCoef;
      *ySpeed[a] = (abs(m_setState.axes[a].speed + e)) / 2.0 / M_PI *
                   m_config.axes[a].PPR * 65536.0 / cps *
                   m_config.axes[a].setSpeedCorrectionCoefficient;
      // todo:
      yxr.azRateAdderTime = 220;
      yxr.elRateAdderTime = 220;
#endif
      break;
    }
    case AxisMode::speed: {
      *yPos[a] =
          m_setState.axes[a].position / 2.0 / M_PI * m_config.axes[a].PPR;
      double e = m_state.axes[a].error;
      e = (e > 0 ? e : 0) * m_config.axes[a].errVCoef;
      *ySpeed[a] = (abs(m_setState.axes[a].speed) + e) / 2.0 / M_PI *
                   m_config.axes[a].PPR * 65536.0 / cps *
                   m_config.axes[a].setSpeedCorrectionCoefficient;
      // todo:
      yxr.azRateAdderTime = 220;
      yxr.elRateAdderTime = 220;
      break;
    }
    }
  }

  uint8_t buffer[yxr.rawSize];
  if (sitechEncode(yxr, buffer, sizeof(buffer)) == sizeof(buffer)) {
    sendCommandACS("YXR\r");
    m_serial.Write(reinterpret_cast<const char *>(buffer), sizeof(buffer));
    m_statusRequested = true;
    // time: when reqest was sent and received by the controller
    m_requestSent = DateTimeClock::now() +
                    std::chrono::milliseconds(21); // todo: (5+34)*10/19200+~1ms
  }
}

void TurretSitech::processOpened() {
  if (!m_connected) {
    LOG_DEBUG("Setting up the communication");
    syncCommunication();
    requestStatus();
    std::this_thread::sleep_for(std::chrono::milliseconds(m_config.cyclems));
    if (decodeStatus()) {
      LOG_DEBUG("Sync");
      m_synced = true;
    } else {
      std::this_thread::sleep_for(
          std::chrono::milliseconds(m_config.reconnectIntevalms));
    }
    m_lastCycle = std::chrono::steady_clock::now();
    m_statusRequested = false;
  } else {
    if (m_statusRequested) {
      m_statusRequested = false;
      decodeStatus();
    }
    executeCommands();
    // command now()+ delivery time + commandAdvance
    auto now = DateTimeClock::now();
    m_setState =       calculateSetPos(now + std::chrono::milliseconds(21 + m_config.commandAdvancems)); // why 2*cyclems????
    m_setStateSpeed =  calculateSetPos(now + std::chrono::milliseconds(21 + m_config.commandAdvanceSpeedms)); // why 2*cyclems????
    doControl();
    if (!m_statusRequested) {
      requestStatus();
    }
    if (std::chrono::steady_clock::now() - m_lastStatus >
        std::chrono::milliseconds(m_config.connectionTimeoutms)) {
      LOG_WARN("No response from the turret. Connection timeout...");
      doDisconnected();
    }
    // std::this_thread::sleep_for(std::chrono::milliseconds(cyclems));
    // todo: more precise timer?
    m_lastCycle += std::chrono::milliseconds(m_config.cyclems);
    std::this_thread::sleep_until(m_lastCycle);
  }
}

void TurretSitech::threadProc() {
  LOG_INFO("Sitech turret thread started");
  while (m_run) {
    if (m_serial.IsOpened()) {
      processOpened();
    } else {
      m_connected = false;
      m_serial.Open();
      if (!m_serial.IsOpened()) {
        LOG_WARN("Unable to open the port");
        std::this_thread::sleep_for(
            std::chrono::milliseconds(m_config.reconnectIntevalms));
      } else {
        LOG_INFO("Serial port opened successfully");
      }
    }
  }
  m_serial.Close();
  doDisconnected();
  LOG_INFO("Exiting Sitech turret thread...");
}

void TurretSitech::run() {
  LOG_INFO("");
  m_run = true;
  if (!m_thread.joinable()) {
    m_thread = std::thread([=, this]() { threadProc(); });
  } else {
    LOG_ERROR("Connect was called for the second time");
  }
};

void TurretSitech::terminate() {
  LOG_INFO("Disconecting");
  m_run = false;
  if (m_thread.joinable()) {
    m_thread.join();
  }
};

TurretState TurretSitech::calculateSetPos(DateTime time) {
  TurretState result = m_setState; // todo: refactro this low quality code!
  result.time = time;
  for (int a = 0; a < axesCount; a++) {
    switch (m_setState.axes[a].mode) {
    case AxisMode::stopped: {
      break;
    }
    case AxisMode::moveto: {
      break;
    }
    case AxisMode::speed: {
      double dt = std::chrono::duration<double>(time - m_state0.time).count();
      result.axes[a].speed = m_state0.axes[a].speed;
      result.axes[a].position =
          m_state0.axes[a].position + dt * m_state0.axes[a].speed;
      break;
    }
    case AxisMode::track: {
      double dt = std::chrono::duration<double>(time - m_state0.time).count();
      result.axes[a].dt = dt;
      result.axes[a].position =
          m_state0.axes[a].position + dt * m_state0.axes[a].speed;
      result.axes[a].speed = m_state0.axes[a].speed;
      break;
    }
    }
  }

  if (m_setState.alpha.mode == AxisMode::track ||
      m_setState.beta.mode == AxisMode::track) {
    if (std::chrono::steady_clock::now() - m_lastTrackCommand >
        std::chrono::milliseconds(m_config.trackTimeout)) {
      LOG_INFO("Tracking timeout");
      doStop();
    }
  }
  return result;
}

void TurretSitech::doDisconnected() {
  m_connected = false;
  m_state.online = false;
  LOG_INFO("Disconnected");
}

} // namespace turret