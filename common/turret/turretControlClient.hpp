

#pragma once

#include "slip.hpp"
#include "tcpClient.hpp"
#include "turretCommands.hpp"
#include "turretState.hpp"
#include <mutex>

namespace turret {

class TurretControlClient : public TcpClient {
public:
  /// @brief TurretControlClient constructor
  /// @param address server address
  /// @param port    server port
  /// @param reconnectIntervalms time between successive conection attempts
  TurretControlClient(const std::string &address, const std::string &port,
                      int reconnectIntervalms = 1000);

  /// @brief Stop turret movement
  void stop();

  /// @brief Move the turret to the specified position
  /// @param alpha horizontal axis position, rad
  /// @param beta vertical axis position, rad
  void gotoPos(double alpha, double beta);

  /// @brief Move turret at specified speed
  /// @param alphaSpeed  horizontal axis speed, rad/s
  /// @param betaSpeed vertical axis speed, rad/s
  void speed(double alphaSpeed, double betaSpeed);

  /// @brief Track specified point at the specified speed
  /// @param alpha horizontal axis position, rad
  /// @param beta vertical axis position, rad
  /// @param alphaSpeed  horizontal axis speed, rad/s
  /// @param betaSpeed vertical axis speed, rad/s
  void track(DateTime time, double alpha, double beta, double alphaSpeed,
             double betaSpeed, double alphaAcceleration, double betaAcelleration);

  /// @brief Set turret axes position to specified point. Axes are not moved!
  /// @param alpha horizontal axis position, rad
  /// @param beta vertical axis position, rad
  void setPosition(double alpha, double beta);

private:
  void sendCommand(const Command &command);
  std::recursive_mutex m_mutex;

  Data m_data;
  Data m_slipData;
  Serializer m_serializer;
};

} // namespace turret
