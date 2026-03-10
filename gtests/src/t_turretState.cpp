#include "turretState.hpp"
#include <gtest/gtest.h>
#include <vector>

using namespace turret;

TEST(test_turretState, test_turretSerializer) {
  Data data;
  Serializer serializer(data);
  bool b = true;
  int32_t i32 = 0x12345678;
  int64_t i64 = 0x12345678ABCDEF01;
  float f32 = -100.5;
  double f64 = -100.5;
  char s[] = "Hello world!";

  serializer.write(b);
  serializer.write(i32);
  serializer.write(i64);
  serializer.write(f32);
  serializer.write(f64);
  serializer.write(s, strlen(s));

  Deserializer deserializer(data);
  bool br = deserializer.readBool();
  int32_t i32r = deserializer.readInt32();
  int64_t i64r = deserializer.readInt64();
  float f32r = deserializer.readFloat();
  double f64r = deserializer.readDouble();
  char sr[100] = {0};
  deserializer.read(sr, strlen(s));

  EXPECT_TRUE(br == b);
  EXPECT_TRUE(i32r == i32);
  EXPECT_TRUE(i64r == i64);
  EXPECT_TRUE(f32r == f32);
  EXPECT_TRUE(f64r == f64);
  EXPECT_TRUE(strcmp(sr, s) == 0);

  Deserializer deserializer2(data);
  bool brr;
  int32_t i32rr;
  int64_t i64rr;
  float f32rr;
  double f64rr;

  deserializer2.read(brr);
  deserializer2.read(i32rr);
  deserializer2.read(i64rr);
  deserializer2.read(f32rr);
  deserializer2.read(f64rr);

  EXPECT_TRUE(brr == b);
  EXPECT_TRUE(i32rr == i32);
  EXPECT_TRUE(i64rr == i64);
  EXPECT_TRUE(f32rr == f32);
  EXPECT_TRUE(f64rr == f64);
}

TEST(test_turretState, test_turretDeserializeEmpty) {
  Data data;
  Deserializer deserializer(data);
  bool exceptionFired = false;
  try {
    deserializer.readBool();
  } catch (const std::exception &e) {
    exceptionFired = true;
  }

  EXPECT_TRUE(exceptionFired);
}

TEST(test_turretState, test_turretState) {
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

  Data data;
  Serializer serializer(data);
  Deserializer deserializer(data);

  state.serialize(serializer);
  TurretState state2;
  state2.deserialize(deserializer);
  EXPECT_TRUE(state.time == state2.time);
  EXPECT_TRUE(state.online == state2.online);
  for (int a = 0; a < axesCount; a++) {
    EXPECT_TRUE(state.axes[a].position == state2.axes[a].position);
    EXPECT_TRUE(state.axes[a].speed == state2.axes[a].speed);
    EXPECT_TRUE(state.axes[a].error == state2.axes[a].error);
    EXPECT_TRUE(state.axes[a].mode == state2.axes[a].mode);
    EXPECT_TRUE(state.axes[a].minLimit == state2.axes[a].minLimit);
    EXPECT_TRUE(state.axes[a].maxLimit == state2.axes[a].maxLimit);
    EXPECT_TRUE(state.axes[a].driveError == state2.axes[a].driveError);
    EXPECT_TRUE(state.axes[a].encoderError == state2.axes[a].encoderError);
  }
}