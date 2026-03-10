#pragma once

namespace turret {

class PID {
public:
  struct Params {
    double p = 0;
    double i = 0;
    double d = 0;
    double v = 0;

    double maxOut = 0;          // Output limit
    double maxAcceleration = 0; // Output acceleration/decceleration limit
    double deccelerationZone = 0; // Output acceleration/decceleration limit
    double minAccOutLimit = 0;

    double integralLimit = 0;
  };

  struct Status {
    Status();
    double error = 0;
    double ierror = 0;
    double derror = 0;
    double p = 0;
    double i = 0;
    double d = 0;
    double v = 0;

    double maxK = 0;

    double outPid = 0;
    double outPidLim = 0;
    double outPidv = 0;
    double outPidvLim = 0;
  };

  explicit PID(const Params &params);

  /// @brief Process PID step
  /// @param error setPoint - actualValue
  /// @param v nominal setpoint rate
  /// @return Control action
  double process(double error, double v = 0);
  // PidParams params;
  const Status &status() { return m_status; }
  Params &params() { return m_params; }

private:
  Params m_params;
  Status m_status;
};

} // namespace turret
