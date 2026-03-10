

#pragma once

#include "serializer.hpp"
#include "turretTypes.hpp"
#include <memory>

namespace turret {

class Command;

using CommandPtr = std::unique_ptr<Command>;

class Command {
public:
  // type definition
  enum Id { Stop, Goto, Speed, Track, SetPosition };

  // Static "factory" method
  static CommandPtr deserialize(Deserializer deserializer);

  Command(Id id) : m_id(id){};
  virtual ~Command() = default;

  Id id() const { return m_id; }
  void serialize(Serializer &serializer) const;

protected:
  virtual void serializeBody(Serializer & /*serializer*/) const {};
  virtual void deserializeBody(Deserializer & /*deserializer*/) {};

private:
  Id m_id;
};

class CmdStop : public Command {
public:
  CmdStop() : Command(Id::Stop) {}
  CmdStop(Deserializer &) : Command(Id::Stop) {}
};

class CmdGoto : public Command {
public:
  CmdGoto(double alpha, double beta);
  CmdGoto(Deserializer &deserializer) : Command(Id::Goto) {
    deserializeBody(deserializer);
  }

  double alpha() const { return m_alpha; }
  double beta() const { return m_beta; }

protected:
  void serializeBody(Serializer &serializer) const;
  void deserializeBody(Deserializer &deserializer);

  double m_alpha;
  double m_beta;
};

class CmdSpeed : public Command {
public:
  CmdSpeed(double alphaSpeed, double betaSpeed);
  CmdSpeed(Deserializer &deserializer) : Command(Id::Speed) {
    deserializeBody(deserializer);
  }

  double alphaSpeed() const { return m_alphaSpeed; }
  double betaSpeed() const { return m_betaSpeed; }

protected:
  void serializeBody(Serializer &serializer) const;
  void deserializeBody(Deserializer &deserializer);

  double m_alphaSpeed;
  double m_betaSpeed;
};

class CmdTrack : public Command {
public:
  CmdTrack(DateTime time, double alpha, double beta, double alphaSpeed,
           double betaSpeed, double alphaAcceleration, double betaAcceleration);
  CmdTrack(Deserializer &deserializer) : Command(Id::Track) {
    deserializeBody(deserializer);
  }

  DateTime time() const { return m_time; }
  double alpha() const { return m_alpha; }
  double beta() const { return m_beta; }
  double alphaSpeed() const { return m_alphaSpeed; }
  double betaSpeed() const { return m_betaSpeed; }
  double alphaAcceleration() const { return m_alphaAcceleration; }
  double betaAcceleration() const { return m_betaAcceleration; }
protected:
  void serializeBody(Serializer &serializer) const;
  void deserializeBody(Deserializer &deserializer);

  DateTime m_time;
  double m_alpha;
  double m_beta;
  double m_alphaSpeed;
  double m_betaSpeed;
  double m_alphaAcceleration;
  double m_betaAcceleration;
};

class CmdSetPosition : public Command {
public:
  CmdSetPosition(double alpha, double beta);
  CmdSetPosition(Deserializer &deserializer) : Command(Id::SetPosition) {
    deserializeBody(deserializer);
  }

  double alpha() const { return m_alpha; }
  double beta() const { return m_beta; }

protected:
  void serializeBody(Serializer &serializer) const;
  void deserializeBody(Deserializer &deserializer);

  double m_alpha;
  double m_beta;
};

// class Unknown : public Command {
// public:
//   Unknown() : Command(Id::Unknown){};
//   Unknown(Deserializer &) : Command(Id::Unknown) {}
// };

} // namespace turret