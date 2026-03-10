#pragma once

#include <chrono>

namespace turret {

using DateTimeClock = std::chrono::system_clock;
using DateTime = DateTimeClock::time_point;

/// @brief Local Coordinate System
struct LCS {
  double alpha;
  double beta;
};

/// @brief Local Coordinate System + time + velocity
struct LCSv {
  DateTime time;
  double alpha;
  double beta;
  double alphaSpeed;
  double betaSpeed;
  double alphaAcceleration;
  double betaAcceleration;
};

} // namespace turret