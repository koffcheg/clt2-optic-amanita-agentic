#include "turretCommands.hpp"
#include <gtest/gtest.h>
#include <vector>

using namespace turret;

TEST(test_turretCommands, test_turretCmdStop) {
  CmdStop cmd;

  Data data;
  Serializer serializer(data);
  Deserializer deserializer(data);

  cmd.serialize(serializer);

  CommandPtr decodedCmd = Command::deserialize(deserializer);

  EXPECT_TRUE(cmd.id() == decodedCmd->id());
}

TEST(test_turretCommands, test_turretCmdGoto) {
  CmdGoto cmd(1, 2);

  Data data;
  Serializer serializer(data);
  Deserializer deserializer(data);

  cmd.serialize(serializer);

  CommandPtr pdecodedCmd = Command::deserialize(deserializer);
  auto &decodedCmd = *static_cast<CmdGoto *>(pdecodedCmd.get());

  EXPECT_TRUE(cmd.id() == decodedCmd.id());
  EXPECT_TRUE(cmd.alpha() == decodedCmd.alpha());
  EXPECT_TRUE(cmd.beta() == decodedCmd.beta());
}

TEST(test_turretCommands, test_turretCmdSpeed) {
  CmdSpeed cmd(1, 2);

  Data data;
  Serializer serializer(data);
  Deserializer deserializer(data);

  cmd.serialize(serializer);

  CommandPtr pdecodedCmd = Command::deserialize(deserializer);
  auto &decodedCmd = *static_cast<CmdSpeed *>(pdecodedCmd.get());

  EXPECT_TRUE(cmd.id() == decodedCmd.id());
  EXPECT_TRUE(cmd.alphaSpeed() == decodedCmd.alphaSpeed());
  EXPECT_TRUE(cmd.betaSpeed() == decodedCmd.betaSpeed());
}

TEST(test_turretCommands, test_turretCmdTrack) {
  CmdTrack cmd(DateTimeClock::now(), 1, 2, 3, 4, 5, 6);

  Data data;
  Serializer serializer(data);
  Deserializer deserializer(data);

  cmd.serialize(serializer);

  CommandPtr pdecodedCmd = Command::deserialize(deserializer);
  auto &decodedCmd = *static_cast<CmdTrack *>(pdecodedCmd.get());

  EXPECT_TRUE(cmd.id() == decodedCmd.id());
  EXPECT_TRUE(cmd.time() == decodedCmd.time());
  EXPECT_TRUE(cmd.alpha() == decodedCmd.alpha());
  EXPECT_TRUE(cmd.beta() == decodedCmd.beta());
  EXPECT_TRUE(cmd.alphaSpeed() == decodedCmd.alphaSpeed());
  EXPECT_TRUE(cmd.betaSpeed() == decodedCmd.betaSpeed());
}

TEST(test_turretCommands, test_turretCmdSetPosition) {
  CmdSetPosition cmd(1, 2);

  Data data;
  Serializer serializer(data);
  Deserializer deserializer(data);

  cmd.serialize(serializer);

  CommandPtr pdecodedCmd = Command::deserialize(deserializer);
  auto &decodedCmd = *static_cast<CmdSetPosition *>(pdecodedCmd.get());

  EXPECT_TRUE(cmd.id() == decodedCmd.id());
  EXPECT_TRUE(cmd.alpha() == decodedCmd.alpha());
  EXPECT_TRUE(cmd.beta() == decodedCmd.beta());
}

TEST(test_turretCommands, test_turretCmdUnknown) {
  Data data{255};
  Deserializer deserializer(data);

  CommandPtr cmd = Command::deserialize(deserializer);

  EXPECT_TRUE(!cmd);
}
