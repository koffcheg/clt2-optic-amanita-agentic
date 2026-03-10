
#pragma once

#include "ceSerial.h"
#include "pid.hpp"
#include "sharedQueue.hpp"
#include "sitech.hpp"
#include "turret.hpp"
#include "turretCommands.hpp"
#include "turretState.hpp"
#include "utils.hpp"
#include "utils/fileLogger.hpp"
#include <atomic>
#include <chrono>
#include <jansson.h>
#include <mutex>
#include <thread>

namespace turret {

class PidLogger;

// todo: choose where to place this definition
using steadyTP = std::chrono::steady_clock::time_point;

class PidLogger : public FileLogger {
public:
  PidLogger(const std::string &filename, bool enabled)
      : FileLogger(filename,
                   "date time error ierror derror p i d v maxK, outPid "
                   "outPidLim outPidv outPidvLim dt",
                   enabled) {}

  void logPid(std::chrono::system_clock::time_point t,
              const PID::Status &status, double dt) {
    log(t, {status.error, status.ierror, status.derror, status.p, status.i,
            status.d, status.v, status.maxK, status.outPid, status.outPidLim,
            status.outPidv, status.outPidvLim, dt*1000});
  }
};

class StateLogger : public FileLogger {
public:
  StateLogger(const std::string &filename, bool enabled)
      : FileLogger(filename, "date time alpha beta valpha vbeta Dalpha Dbeta",
                   enabled) {}

  void logState(const TurretState &state) {
    using namespace u;
    if (state.alpha.mode != AxisMode::stopped ||
        state.beta.mode != AxisMode::stopped) {
      log(state.time, {state.alpha.position / DEG, state.beta.position / DEG,
                       state.alpha.speed / DEG, state.beta.speed / DEG,
                       state.alpha.error / DEG, state.beta.error / DEG});
    }
  }
};

class TurretSitechParams {
  TurretSitechParams(const json_t *config);
  std::string port;
  int baudRate;
};

class TurretSitech : public Turret {
public:
  TurretSitech(const json_t *config);

  void stop() override;
  void moveto(double alpha, double beta) override;
  void speed(double alphaSpeed, double betaSpeed) override;
  void track(DateTime time, double alpha, double beta, double alphaSpeed,
             double betaSpeed, double alphaAcceleration,
             double betaAcceleration) override;
  void setPosition(double alpha, double beta) override;
  bool connected() override { return m_connected; };
  void run() override;
  void terminate() override;

protected:
  std::atomic_bool m_connected = false;

private:
  static constexpr double cps = 1953;

  void processOpened();
  void syncCommunication();
  void sendCommandRaw(const std::string &cmd);
  void sendCommandACS(const std::string &cmd);
  void requestStatus();
  bool decodeStatus();
  void threadProc();
  void processStatus(const SitechStatus &ss);
  void executeCommands();

  TurretState calculateSetPos(DateTime time);

  void doControl();
  void doDisconnected();
  void doStop();
  void checkStop(int a, bool stoppedBit);

  struct Config {
    std::string port = "/dev/ttyUSB0";
    int baudRate = 19200;
    struct {
      int PPR = 10000000; // 1 << 26; // 67108864
      double gotoSpeed = 50 * u::DEG;
      double setSpeedCorrectionCoefficient = 1; // 0.5;
      double errVCoef = 0.5;                    // 0.5;
      double tau = 0;
      PID::Params pid;
      bool pidDebug = false;
      std::string pidDebugFile;
    } axes[axesCount];
    int commandAdvancems = 0;      // time in ms of command
    int commandAdvanceSpeedms = 0; // time in ms of command
    int connectionTimeoutms = 2000;
    int reconnectIntevalms = 1000;
    int cyclems = 55;
    int trackTimeout = 1000;

    bool stateDebug = false;
    std::string stateDebugFile = "state.log";

    Config() = default;
    Config(const json_t *config);
  } m_config;

  struct _AxisState {
    int stopCounter = 0;
  } m_axesState[axesCount];

  PID pid[axesCount];
  PidLogger m_pidLog[axesCount];
  StateLogger m_stateLog;

  TurretState m_setState{};
  TurretState m_setStateSpeed{};
  TurretState m_state0{};

  ceSerial m_serial;

  bool m_synced = false;
  bool m_statusRequested = false;

  std::thread m_thread;
  std::atomic_bool m_run;
  steadyTP m_lastStatus;

  std::recursive_mutex m_mutex;
  SharedQueue<CommandPtr> m_commands;
  std::chrono::steady_clock::time_point m_lastCycle;
  std::chrono::steady_clock::time_point m_lastTrackCommand;
  DateTime m_requestSent;
};

} // namespace turret