

#include "turretControlClient.hpp"
#include "turretCommands.hpp"
#include "turretState.hpp"
#include <functional>

namespace turret {

TurretControlClient::TurretControlClient(const std::string &address,
                                         const std::string &port,
                                         int reconnectIntervalms)
    : TcpClient(address, port, reconnectIntervalms), m_serializer(m_data) {}

void TurretControlClient::sendCommand(const Command &command) {
  std::lock_guard<std::recursive_mutex> guard(m_mutex);
  if (!connected()) {
    // log: throw std::runtime_error("turretpro not connected");
    return;
  }
  m_data.clear();
  m_slipData.clear();
  command.serialize(m_serializer);
  slip::encode(m_data, m_slipData);
  sendData(m_slipData.data(), m_slipData.size());
}

void TurretControlClient::stop() {
  CmdStop cmd;
  sendCommand(cmd);
}

void TurretControlClient::gotoPos(double alpha, double beta) {
  CmdGoto cmd(alpha, beta);
  sendCommand(cmd);
}

void TurretControlClient::speed(double alphaSpeed, double betaSpeed) {
  CmdSpeed cmd(alphaSpeed, betaSpeed);
  sendCommand(cmd);
}

void TurretControlClient::track(DateTime time, double alpha, double beta,
                                double alphaSpeed, double betaSpeed,
                                double alphaAcceleration,
                                double betaAcceleration) {
  CmdTrack cmd(time, alpha, beta, alphaSpeed, betaSpeed, alphaAcceleration,
               betaAcceleration);
  sendCommand(cmd);
}

void TurretControlClient::setPosition(double alpha, double beta) {
  CmdSetPosition cmd(alpha, beta);
  sendCommand(cmd);
}

} // namespace turret