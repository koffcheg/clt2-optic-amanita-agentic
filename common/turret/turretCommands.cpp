
#include "turretCommands.hpp"

namespace turret {

void Command::serialize(Serializer &serializer) const {
  serializer.write(static_cast<int8_t>(m_id));
  serializeBody(serializer);
}

CommandPtr Command::deserialize(Deserializer deserializer) {
  try {
    Command::Id id = static_cast<Command::Id>(deserializer.readInt8());
    switch (id) {
    case Command::Id::Stop: {
      return std::make_unique<CmdStop>(deserializer);
    }
    case Command::Id::Goto: {
      return std::make_unique<CmdGoto>(deserializer);
    }
    case Command::Id::Speed: {
      return std::make_unique<CmdSpeed>(deserializer);
    }
    case Command::Id::Track: {
      return std::make_unique<CmdTrack>(deserializer);
    }
    case Command::Id::SetPosition: {
      return std::make_unique<CmdSetPosition>(deserializer);
    }
    default: {
      // todo: log
      // throw std::runtime_error("Unknown command");
    }
    }
  } catch (const std::exception &e) {
  }

  return nullptr;
}

CmdGoto::CmdGoto(double alpha, double beta)
    : Command(Id::Goto), m_alpha(alpha), m_beta(beta) {}

void CmdGoto::serializeBody(Serializer &serializer) const {
  serializer.write(m_alpha);
  serializer.write(m_beta);
}

void CmdGoto::deserializeBody(Deserializer &deserializer) {
  deserializer.read(m_alpha);
  deserializer.read(m_beta);
}

CmdSpeed::CmdSpeed(double alphaSpeed, double betaSpeed)
    : Command(Id::Speed), m_alphaSpeed(alphaSpeed), m_betaSpeed(betaSpeed) {}

void CmdSpeed::serializeBody(Serializer &serializer) const {
  serializer.write(m_alphaSpeed);
  serializer.write(m_betaSpeed);
}

void CmdSpeed::deserializeBody(Deserializer &deserializer) {
  deserializer.read(m_alphaSpeed);
  deserializer.read(m_betaSpeed);
}

CmdTrack::CmdTrack(DateTime time, double alpha, double beta, double alphaSpeed,
                   double betaSpeed, double alphaAcceleration,
                   double betaAcceleration)
    : Command(Id::Track), m_time(time), m_alpha(alpha), m_beta(beta),
      m_alphaSpeed(alphaSpeed), m_betaSpeed(betaSpeed),
      m_alphaAcceleration(alphaAcceleration),
      m_betaAcceleration(betaAcceleration) {}

void CmdTrack::serializeBody(Serializer &serializer) const {
  serializer.write(m_time);
  serializer.write(m_alpha);
  serializer.write(m_beta);
  serializer.write(m_alphaSpeed);
  serializer.write(m_betaSpeed);
  serializer.write(m_alphaAcceleration);
  serializer.write(m_betaAcceleration);
}

void CmdTrack::deserializeBody(Deserializer &deserializer) {
  deserializer.read(m_time);
  deserializer.read(m_alpha);
  deserializer.read(m_beta);
  deserializer.read(m_alphaSpeed);
  deserializer.read(m_betaSpeed);
  deserializer.read(m_alphaAcceleration);
  deserializer.read(m_betaAcceleration);
}

CmdSetPosition::CmdSetPosition(double alpha, double beta)
    : Command(Id::SetPosition), m_alpha(alpha), m_beta(beta) {}

void CmdSetPosition::serializeBody(Serializer &serializer) const {
  serializer.write(m_alpha);
  serializer.write(m_beta);
}

void CmdSetPosition::deserializeBody(Deserializer &deserializer) {
  deserializer.read(m_alpha);
  deserializer.read(m_beta);
}

} // namespace turret