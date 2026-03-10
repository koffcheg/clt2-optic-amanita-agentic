
#pragma once

#include "pid.hpp"
#include "turret.hpp"
#include "turretState.hpp"
#include <atomic>
#include <jansson.h>
#include <thread>

namespace turret {

class TurretAxisSim {
public:
  explicit TurretAxisSim(AxisState &state);

  void stop();

  void moveto(double position);
  void speed(double speed);
  void track(DateTime time, double posiiton, double speed);
  void setPosition(double position);

  void advanceState(double dt);

private:
  void processMoveto();
  void processTrack();
  void processSpeed(double dt);

  AxisState &m_state;

  PID m_pid;

  DateTime m_time;
  double m_speed;         // Output of the regulator
  double m_setpoint;      // Position setpoint
  double m_speedSetpoint; // Speed setpoint

  double m_minLimit;
  double m_maxLimit;
};

class TurretSim : public Turret {
public:
  TurretSim();
  TurretSim(const json_t *config);
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
  bool m_connected = false;

private:
  /// @brief Calculates new state of the simulated turret out of old one, params
  /// and DT
  /// @param dT
  void advanceState(double dt);

  void threadProc();

  TurretAxisSim m_axes[axesCount];
  std::thread m_thread;
  std::atomic_bool m_run;
};

} // namespace turret