#include "turretState.hpp"
#include "turretInfoClient.hpp"
#include "../../turretpro/src/turretInfoServer.hpp"
#include <gtest/gtest.h>
#include <vector>


using namespace turret;
using namespace turret;

const std::string INFO_PORT = "19810";
const std::string INFO_HOST = "localhost";

TurretState sampleStateData(){
  TurretState state;

  state.alpha.position = 1.1;
  state.alpha.speed = 2.2;
  state.alpha.error = 0.1;
  state.alpha.mode = AxisMode::moveto;
  state.alpha.minLimit = true;
  state.alpha.maxLimit = false;
  state.alpha.driveError = 12;
  state.alpha.encoderError = 13;

  state.beta.position = 5.1;
  state.beta.speed = 5.2;
  state.beta.error = 5.1;
  state.beta.mode = AxisMode::speed;
  state.beta.minLimit = false;
  state.beta.maxLimit = true;
  state.beta.driveError = 52;
  state.beta.encoderError = 53;

  state.online = true;
  state.time = DateTimeClock::now();

  return state;
} 
 
TEST(test_turretInfoServer, test_turretInfoServer) {

  TurretState state(sampleStateData());

  TurretInfoClient infoClient(INFO_HOST, INFO_PORT);
  TurretInfoServer infoServer(INFO_HOST, INFO_PORT);

  infoServer.run();
  std::this_thread::sleep_for(std::chrono::milliseconds(50));
  infoClient.run();  
  std::this_thread::sleep_for(std::chrono::milliseconds(50));
  infoServer.sendState(state);
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  infoServer.terminate();
  infoClient.terminate();


  EXPECT_TRUE(infoClient.state().online == state.online);
  EXPECT_TRUE(infoClient.state().time == state.time);

  for (int a = 0; a < axesCount; a++) {
    EXPECT_TRUE(infoClient.state().axes[a].position == state.axes[a].position);
    EXPECT_TRUE(infoClient.state().axes[a].speed == state.axes[a].speed);
    EXPECT_TRUE(infoClient.state().axes[a].error == state.axes[a].error);
    EXPECT_TRUE(infoClient.state().axes[a].mode == state.axes[a].mode);
    EXPECT_TRUE(infoClient.state().axes[a].minLimit == state.axes[a].minLimit);
    EXPECT_TRUE(infoClient.state().axes[a].maxLimit == state.axes[a].maxLimit);
    EXPECT_TRUE(infoClient.state().axes[a].driveError == state.axes[a].driveError);
    EXPECT_TRUE(infoClient.state().axes[a].encoderError == state.axes[a].encoderError);
  }  
}