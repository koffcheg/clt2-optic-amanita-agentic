#include "turretSim.hpp"
#include "utils.hpp"
#include <algorithm>
#include <limits>
#include <logHelper.hpp>
#include <math.h>

using namespace u;

namespace turret {

static auto logger = log4cxx::Logger::getLogger("turret.sim");

TurretAxisSim::TurretAxisSim(AxisState &state)
    : m_state{state},
      m_pid(PID::Params(2, 0.01, 0, 20 * DEG, 10 * DEG, 1 * DEG, 0.1 * DEG, 1)),
      m_speed{0}, m_setpoint{0}, m_speedSetpoint{0}, m_minLimit{-180 * DEG},
      m_maxLimit{180 * DEG} {};

// TurretAxisSim
void TurretAxisSim::stop() {
  m_speed = 0;
  m_state.mode = AxisMode::stopped;
}

void TurretAxisSim::moveto(double position) {
  m_setpoint = position;
  m_state.mode = AxisMode::moveto;
}

void TurretAxisSim::speed(double speed) {
  m_speed = m_speedSetpoint = speed;
  m_state.mode = AxisMode::speed;
}

void TurretAxisSim::track(DateTime time, double position, double speed) {
  m_time = time;
  m_setpoint = position;
  m_speedSetpoint = speed;
  m_state.mode = AxisMode::track;
}

void TurretAxisSim::setPosition(double position) {
  m_state.position = position;
}

void TurretAxisSim::processMoveto() {
  m_speedSetpoint = 0;
  m_speed = m_pid.process(m_setpoint - m_state.position);
}

void TurretAxisSim::processTrack() {
  m_speed = m_pid.process(m_setpoint - m_state.position, m_speedSetpoint);
}

void TurretAxisSim::processSpeed(double dt) {
  double dv = m_speed - m_state.speed;
  dv = u::coerceAbs(dv, m_pid.params().maxAcceleration * dt);
  m_state.speed = u::coerceAbs(m_state.speed + dv, m_pid.params().maxOut);
}

void TurretAxisSim::advanceState(double dt) {
  switch (m_state.mode) {
  case AxisMode::stopped: {
    m_speed = m_speedSetpoint = 0;
    processSpeed(dt);
    break;
  }
  case AxisMode::moveto: {
    processMoveto();
    processSpeed(dt);
    m_state.error = m_setpoint - m_state.position;

    if (fabs(m_state.error) < 0.004_deg && fabs(m_state.speed) < 0.01_deg) {
      m_state.mode = AxisMode::stopped;
    }
    break;
  }

  case AxisMode::speed: {
    processSpeed(dt);
    break;
  }
  case AxisMode::track: {
    processTrack();
    break;
  }
  }

  m_state.position += u::coerce(m_state.speed * dt, m_minLimit, m_maxLimit);
  m_state.error = m_state.position - m_setpoint;
  m_state.minLimit = m_state.position <= m_minLimit;
  m_state.maxLimit = m_state.position >= m_maxLimit;
  m_state.driveError = 0;
  m_state.encoderError = 0;
}

// TurretSim
TurretSim::TurretSim()
    : m_axes{TurretAxisSim(m_state.alpha), TurretAxisSim(m_state.beta)} {}

// todo:
TurretSim::TurretSim(const json_t * /*config*/)
    : m_axes{TurretAxisSim(m_state.alpha), TurretAxisSim(m_state.beta)} {}

void TurretSim::stop() {
  for (int a = 0; a < axesCount; a++) {
    m_axes[a].stop();
  }
}

void TurretSim::moveto(double alpha, double beta) {
  m_axes[axisAlpha].moveto(alpha);
  m_axes[axisBeta].moveto(beta);
}

void TurretSim::speed(double alphaSpeed, double betaSpeed) {
  m_axes[axisAlpha].speed(alphaSpeed);
  m_axes[axisBeta].speed(betaSpeed);
}

void TurretSim::track(DateTime time, double alpha, double beta,
                      double alphaSpeed, double betaSpeed,
                      double /*alphaAcceleration*/, double /*betaAcceleration*/) {
  m_axes[axisAlpha].track(time, alpha, alphaSpeed);
  m_axes[axisAlpha].track(time, beta, betaSpeed);
}

void TurretSim::setPosition(double alpha, double beta) {
  m_state.alpha.position = alpha;
  m_state.beta.position = beta;
}

void TurretSim::advanceState(double dt) {
  m_state.time = DateTimeClock::now();
  for (int a = 0; a < axesCount; a++) {
    m_axes[a].advanceState(dt);
  }
}

void TurretSim::threadProc() {
  while (m_run) {
    advanceState(0.01); // todo: better timer? and get read of magic consts
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
}

void TurretSim::run() {
  if (m_thread.joinable()) {
    // LOG4CXX_FATAL(logger, "InfoServer is already started");
    throw std::logic_error("Turret sim is already started");
  }
  m_connected = true;
  m_state.online = true;
  m_run = true;

  m_thread = std::thread([=, this]() { threadProc(); });
};

void TurretSim::terminate() {
  m_connected = false;
  m_state.online = false;
  m_run = false;

  if (m_thread.joinable()) {
    m_thread.join();
  }
};

} // namespace turret