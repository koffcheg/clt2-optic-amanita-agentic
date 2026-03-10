

#include "turretInfoClient.hpp"
#include "turretState.hpp"
#include <functional>

namespace turret {

TurretInfoClient::TurretInfoClient(const std::string &address,
                                   const std::string &port,
                                   int reconnectIntervalms, int dataTimeout)
    : TcpClient(address, port, reconnectIntervalms, dataTimeout),
      m_slipDecoder(std::bind(&TurretInfoClient::onSlipFrame, this,
                              std::placeholders::_1)) {}

void TurretInfoClient::onSlipFrame(const slip::Data &decodedData) {
  turret::Deserializer deserializer(decodedData);
  m_state.deserialize(deserializer);
  m_stateReceived = std::chrono::steady_clock::now();
  // todo: check validity
  // todo: callback????
}

TurretState TurretInfoClient::TurretInfoClient::state(DateTime time) {
  TurretState s;
  s = m_state;
  // todo: state history!!!
  double dt = std::chrono::duration<double>(time - s.time).count();

  s.time = time;
  for (int a = 0; a < axesCount; a++) {
    s.axes[a].position += s.axes[a].speed * dt;
  }
  return s;
}

bool TurretInfoClient::stateValid() {
  return connected() && m_state.online &&
         std::chrono::steady_clock::now() - m_stateReceived <
             std::chrono::milliseconds(2000); // todo: option
};

void TurretInfoClient::processData(const void *data, size_t size) {
  m_slipDecoder.decode(data, size);
}

} // namespace turret