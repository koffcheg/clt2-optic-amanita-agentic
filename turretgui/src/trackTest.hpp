
#pragma once

#include <turretTypes.hpp>

struct TrackTestAxis {
  double p0 = 0;
  double A = 0;
  double Ph = 0;
  double T = 0;
  double position(double t);
  double speed(double t);
  double acceleration(double t);
  double position(turret::DateTime t);
  double speed(turret::DateTime t);
  double acceleration(turret::DateTime t);
  turret::DateTime t0;
};

struct TrackTest {
  static constexpr int axesCount = 2;
  TrackTestAxis axes[axesCount];
  //TrackTestAxis & az{axes[0]};
  //TrackTestAxis & el{axes[1]};
  void startTrack(turret::DateTime t0);
  turret::LCSv track(turret::DateTime t);
};
