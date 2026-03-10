
#pragma once

#include "serializer.hpp"
#include "turretTypes.hpp"
#include <string>

namespace turret {

constexpr int axisAlpha = 0;
constexpr int axisBeta = 1;
constexpr int axesCount = 2;

const std::string& axisName(int a);

enum class AxisMode { stopped, moveto, speed, track };

std::string toString(const AxisMode &mode);

class AxisState {
public:
  double position; /// Axis actual position, rad
  double speed;    /// Axis actual speed, rad/s
  double error;    /// Difference between axis setpoint and actual position, rad
  AxisMode mode;   /// Actual mode of operation
  bool minLimit;   /// true if axis min limit is reached (left, bottom)
  bool maxLimit;   /// true if axis max limit is reached (right, top)
  int driveError;  /// Drive error code, nonzero in the case of an error
  int encoderError; /// Encoder error code, nonzero in the case of an error
  double dt;        /// extrapolation time, s

  void serialize(Serializer &serializer) const;
  void deserialize(Deserializer &deserializer);
};

struct TurretState {
  DateTime time; /// Moment of time when turret position was sampled
  bool online;   /// true - turret is connected. false - all further data is
                 /// invalid
  union {
    AxisState axes[axesCount];
    struct {
      AxisState alpha;
      AxisState beta;
    };
  };
  void serialize(Serializer &serializer) const;
  void deserialize(Deserializer &deserializer);
};

} // namespace turret