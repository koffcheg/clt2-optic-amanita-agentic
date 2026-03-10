

#pragma once

#include "infoServer.hpp"
#include "turretState.hpp"

namespace turret {

class TurretInfoServer : public InfoServer {
public:
  TurretInfoServer(const std::string &host, const std::string &port)
      : InfoServer(host, port), m_serializer(m_data){};

  void sendState(const turret::TurretState &state) {
    m_data.clear();
    m_slipData.clear();
    state.serialize(m_serializer);
    slip::encode(m_data, m_slipData);
    sendData(m_slipData.data(), m_slipData.size());
  }

private:
  turret::Data m_data;
  turret::Data m_slipData;
  turret::Serializer m_serializer;
};

} // namespace turret