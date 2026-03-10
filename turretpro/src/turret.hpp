
#pragma once

#include "turretState.hpp"
#include <memory>

namespace turret {

class Turret {
public:
  virtual ~Turret() = default;

  const TurretState &state() const { return m_state; }

  virtual void stop() = 0;

  virtual void moveto(double alpha, double beta) = 0;

  virtual void speed(double alphaSpeed, double betaSpeed) = 0;

  virtual void track(DateTime time, double alpha, double beta,
                     double alphaSpeed, double betaSpeed,
                     double alphaAcceleration, double betaAcceleration) = 0;

  virtual void setPosition(double alpha, double beta) = 0;

  virtual bool connected() = 0;

  virtual void run() = 0;

  virtual void terminate() = 0;

protected:
  TurretState m_state{};
};

using TurretPtr = std::unique_ptr<Turret>;

} // namespace turret