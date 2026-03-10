

#pragma once

#include "slip.hpp"
#include "tcpClient.hpp"
#include "turretState.hpp"

namespace turret {

class TurretInfoClient : public TcpClient {
public:
  /// @brief TurretInfoClient constructor
  /// @param address server address
  /// @param port    server port
  /// @param reconnectIntervalms time between successive conection attempts
  TurretInfoClient(const std::string &address, const std::string &port,
                   int reconnectIntervalms = 1000, int dataTimeout = 2000);

  /// @brief Check if turret state have not expired
  /// @return true if turret state is fresh enough (2000ms)
  bool stateValid();

  // todo: make it thread-safe!
  /// @brief returns the latest turret state
  const turret::TurretState &state() { return m_state; };
  turret::TurretState state(DateTime time);

protected:
  void processData(const void *data, size_t size) override;

private:
  void onSlipFrame(const slip::Data &decodedData);

  slip::Decoder m_slipDecoder;
  turret::TurretState m_state{};
  std::chrono::time_point<std::chrono::steady_clock> m_stateReceived;
};

} // namespace turret
