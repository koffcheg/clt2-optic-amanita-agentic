#include "../../turretpro/src/commandsQueue.hpp"
#include "../../turretpro/src/controlServer.hpp"
#include "turretControlClient.hpp"
#include "turretState.hpp"
#include <gtest/gtest.h>
#include <vector>

using namespace turret;
using namespace turret;

const std::string INFO_PORT = "19811";
const std::string INFO_HOST = "localhost";

TEST(test_turretControlServer, test_turretControlServer) {
  CommandsQueue commandsQueue;
  TurretControlClient controlClient(INFO_HOST, INFO_PORT);
  ControlServer controlServer(INFO_HOST, INFO_PORT, commandsQueue);

  double setAlpha = 1;
  double setBeta = 2;
  double setAlphaSpeed = 3;
  double setBetaSpeed = 4;
  double setAlphaAcceleration = 5;
  double setBetaAcceleration = 6;
  DateTime setTime = DateTimeClock::now();

  controlServer.run();
  std::this_thread::sleep_for(std::chrono::milliseconds(50));
  controlClient.run();
  std::this_thread::sleep_for(std::chrono::milliseconds(50));
  controlClient.stop();
  controlClient.gotoPos(setAlpha, setBeta);
  controlClient.speed(setAlphaSpeed, setBetaSpeed);
  controlClient.track(setTime, setAlpha, setBeta, setAlphaSpeed, setBetaSpeed,
                      setAlphaAcceleration, setBetaAcceleration);
  controlClient.setPosition(setAlpha, setBeta);
  std::this_thread::sleep_for(std::chrono::milliseconds(50));
  controlServer.terminate();
  controlClient.terminate();

  auto cmd1 = commandsQueue.get();
  auto cmd2 = commandsQueue.get();
  auto cmd3 = commandsQueue.get();
  auto cmd4 = commandsQueue.get();
  auto cmd5 = commandsQueue.get();

  EXPECT_TRUE(commandsQueue.empty());

  EXPECT_TRUE(cmd1->id() == Command::Stop);
  EXPECT_TRUE(cmd2->id() == Command::Goto);
  EXPECT_TRUE(dynamic_cast<CmdGoto *>(cmd2.get())->alpha() == setAlpha);
  EXPECT_TRUE(dynamic_cast<CmdGoto *>(cmd2.get())->beta() == setBeta);
  EXPECT_TRUE(cmd3->id() == Command::Speed);
  EXPECT_TRUE(dynamic_cast<CmdSpeed *>(cmd3.get())->alphaSpeed() ==
              setAlphaSpeed);
  EXPECT_TRUE(dynamic_cast<CmdSpeed *>(cmd3.get())->betaSpeed() ==
              setBetaSpeed);
  EXPECT_TRUE(cmd4->id() == Command::Track);
  EXPECT_TRUE(dynamic_cast<CmdTrack *>(cmd4.get())->time() == setTime);
  EXPECT_TRUE(dynamic_cast<CmdTrack *>(cmd4.get())->alpha() == setAlpha);
  EXPECT_TRUE(dynamic_cast<CmdTrack *>(cmd4.get())->beta() == setBeta);
  EXPECT_TRUE(dynamic_cast<CmdTrack *>(cmd4.get())->alphaSpeed() ==
              setAlphaSpeed);
  EXPECT_TRUE(dynamic_cast<CmdTrack *>(cmd4.get())->betaSpeed() ==
              setBetaSpeed);
  EXPECT_TRUE(dynamic_cast<CmdTrack *>(cmd4.get())->alphaAcceleration() ==
              setAlphaAcceleration);
  EXPECT_TRUE(dynamic_cast<CmdTrack *>(cmd4.get())->betaAcceleration() ==
              setBetaAcceleration);
  EXPECT_TRUE(cmd5->id() == Command::SetPosition);
  EXPECT_TRUE(dynamic_cast<CmdSetPosition *>(cmd5.get())->alpha() == setAlpha);
  EXPECT_TRUE(dynamic_cast<CmdSetPosition *>(cmd5.get())->beta() == setBeta);
}