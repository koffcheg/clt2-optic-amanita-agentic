#include "pid.hpp"
#include "utils.hpp"
#include <cmath>

using namespace u;

namespace turret {

PID::Status::Status() : error(NAN) {}

PID::PID(const Params &params) : m_params(params), m_status() {}

double PID::process(double error, double v) {
  if (error < 1 * DEG) { // todo:
    m_status.ierror =
        coerceAbs(m_status.ierror + error, m_params.integralLimit);
  } else {
    m_status.ierror = 0;
  }

  if (!isnan(m_status.error)) {
    m_status.derror = error - m_status.error; // todo: dt????
  } else {
    m_status.derror = 0;
  }
  m_status.error = error;

  m_status.p = m_status.error * m_params.p;
  m_status.i = m_status.ierror * m_params.i;
  m_status.d = m_status.derror * m_params.d;
  m_status.v = v * m_params.v;

  m_status.outPid = m_status.p + m_status.i + m_status.d;

  //if (m_params.maxAcceleration > 0) {
  //  // maxv = ±sqrt(v1^2-2*a*Dx); v1=0
  //  double maxv = std::max(m_params.minAccOutLimit,
  //                         sqrt(v * v + fabs(2 * m_params.maxAcceleration * error)));
  //  if (error >= 0) {
  //    m_status.outPidLim = std::min(m_status.outPid, maxv);
  //  } else {
  //    m_status.outPidLim = std::max(m_status.outPid, -maxv);
  //  }
  //}
  // todo:
   if (fabs(error) < m_params.deccelerationZone) {
    m_status.maxK = (fabs(error) / m_params.deccelerationZone) * m_params.maxOut;
    if (m_status.maxK < m_params.minAccOutLimit)
      m_status.maxK = m_params.minAccOutLimit;
    m_status.outPidLim  = coerceAbs(m_status.outPid, m_status.maxK);
  } else{
    m_status.maxK = 0;
    m_status.outPidLim = m_status.outPid;
  }
  //m_status.outPidLim = m_status.outPid;
  m_status.outPidv = m_status.outPidLim + m_status.v;
  m_status.outPidvLim = coerceAbs(m_status.outPidv, m_params.maxOut);

  return m_status.outPidvLim;
}

} // namespace turret