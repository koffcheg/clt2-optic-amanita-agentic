
#include "turretState.hpp"
#include <cmath>

namespace turret {


const std::string& axisName(int a){
  static const std::string axesNames[axesCount] = {"alpha", "beta"};
  static const std::string unknown = "???";
  if (a>=0 && a<axesCount){
    return axesNames[a];
  }
  return unknown;
}


std::string toString(const AxisMode &mode) {
  switch (mode) {
  case AxisMode::stopped:
    return "stopped";
  case AxisMode::moveto:
    return "moveto";
  case AxisMode::speed:
    return "speed";
  case AxisMode::track:
    return "track";
  }
  return "?";
}

void AxisState::serialize(Serializer &serializer) const {
  serializer.write(position);
  serializer.write(speed);
  serializer.write(error);
  serializer.write(static_cast<int8_t>(mode));
  serializer.write(minLimit);
  serializer.write(maxLimit);
  serializer.write(driveError);
  serializer.write(encoderError);
}

void AxisState::deserialize(Deserializer &deserializer) {
  deserializer.read(position);
  deserializer.read(speed);
  deserializer.read(error);
  mode = static_cast<AxisMode>(deserializer.readInt8());
  deserializer.read(minLimit);
  deserializer.read(maxLimit);
  deserializer.read(driveError);
  deserializer.read(encoderError);
}

void TurretState::serialize(Serializer &serializer) const {
  serializer.write(time);
  serializer.write(online);
  for (int a = 0; a < axesCount; a++) {
    axes[a].serialize(serializer);
  }
}

void TurretState::deserialize(Deserializer &deserializer) {
  deserializer.read(time);
  deserializer.read(online);
  for (int a = 0; a < axesCount; a++) {
    axes[a].deserialize(deserializer);
  }
}

} // namespace turret