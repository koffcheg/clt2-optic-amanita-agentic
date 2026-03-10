
#include "trackTest.hpp"
#include <chrono>
#include <cmath>

double TrackTestAxis::position(double t) {
  return p0 + A * sin(2 * M_PI * t / T - Ph);
}

double TrackTestAxis::speed(double t) {
  return 2 * M_PI * A / T * cos(2 * M_PI * t / T - Ph);
}

double TrackTestAxis::acceleration(double t) {
  return -4 * M_PI * M_PI * A / (T * T) * sin(2 * M_PI * t / T - Ph);
}

double TrackTestAxis::position(turret::DateTime t) {
  return position(std::chrono::duration<double>(t - t0).count());
}

double TrackTestAxis::speed(turret::DateTime t) {
  return speed(std::chrono::duration<double>(t - t0).count());
}

double TrackTestAxis::acceleration(turret::DateTime t) {
  return acceleration(std::chrono::duration<double>(t - t0).count());
}

void TrackTest::startTrack(turret::DateTime t0) {
  for (int a = 0; a < axesCount; a++) {
    axes[a].t0 = t0;
  }
}

turret::LCSv TrackTest::track(turret::DateTime t){
  turret::LCSv result;  
  result.alpha = axes[0].position(t);
  result.beta = axes[1].position(t);
  result.alphaSpeed = axes[0].speed(t);
  result.betaSpeed = axes[1].speed(t);
  result.alphaAcceleration = axes[0].acceleration(t);
  result.betaAcceleration = axes[1].acceleration(t);
  result.time = t;
  return result;
}

