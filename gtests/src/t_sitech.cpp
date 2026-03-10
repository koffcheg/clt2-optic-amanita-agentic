#include "sitech.hpp"
#include <gtest/gtest.h>
#include <array>

using namespace turret;

TEST(test_sitech, statusDecode) {
  SitechStatus status;

// clang-format off  
  unsigned char sampleRawData[] = {
    0xA9,                    //A8 + controller address
    0x1D, 0x5C, 0x00, 0x00,  //Alt/Dec motor position: 23,581
    0x5E, 0x67, 0x04, 0x00,  //Az/RA motor position: 288,606
    0x00, 0x00, 0x00, 0x00,  //Alt/Dec encoder location: 0
    0x1D, 0x19, 0x00, 0x00,  //Az/RA encoder location: 6,429
    0x00,                    //Keypad status: 0
    0x60,                    //XBits
    0x00,                    //YBits
    0x80,                    //Various bits
    0x00, 0x00,              //Analog input 1: 0
    0x00, 0x00,              //Analog input 2: 0
    0x5E, 0x96, 0x0E, 0x00,  //Millisecond Clock: 955,998
    0x50,                    //Temperature in degrees F: 80 (may change to Alt/Dec Worm Phase)
    0x99,                    //Az/RA Worm phase (0-255): 153 (153/256 = 59.7 percent)
    0x00, 0x00, 0x00, 0x00,  //Alt/Dec motor location at last Alt/Dec encoder location change
    0x2D, 0x67, 0x04, 0x00,  //Az/RA motor location at last Az/RA encoder location change
    0x84, 0xFA,              //Checksum (= (A9 + 1D + … + 67 + 04 + 00) ^ 0xFF00, LSB first)
  };
// clang-format on
  EXPECT_FALSE(status.decode(sampleRawData, std::size(sampleRawData)-1));//incorrect size
  EXPECT_TRUE(status.decode(sampleRawData, std::size(sampleRawData))); 
  EXPECT_TRUE(status.address == 0x01);
  EXPECT_TRUE(status.el == 23581);
  EXPECT_TRUE(status.az == 288606);
  EXPECT_TRUE(status.altScope == 0);
  EXPECT_TRUE(status.azScope == 6429);
  EXPECT_TRUE(status.keypadStatus.asInt == 0);  
  EXPECT_TRUE(status.xbits.asInt == 0x60);  
  EXPECT_TRUE(status.ybits.asInt == 0);  
  EXPECT_TRUE(status.extraBits.asInt == 0x80);  
  EXPECT_TRUE(status.analog1 == 0);  
  EXPECT_TRUE(status.analog2 == 0);  
  EXPECT_TRUE(status.milliseconds == 955998);  
  EXPECT_TRUE(status.temperature == 80);  
  EXPECT_TRUE(status.azWormPhase == 153);  
  EXPECT_TRUE(status.altAtLastScope == 0);  
  EXPECT_TRUE(status.azAtLastScope == 288557);  
  sampleRawData[0] += 1; //invalidate checksum
  EXPECT_FALSE(status.decode(sampleRawData, std::size(sampleRawData)));
}

TEST(test_sitech, xxrEncode) {
  SitechXXR xxr;
  xxr.elPosSet = 123456;
  xxr.elSpeedSet = 234567;
  xxr.azPosSet = -123;
  xxr.azSpeedSet = 12;
  xxr.useBits = 1;
  xxr.xBits = 2;
  xxr.yBits = 3;
  uint8_t data[xxr.rawSize];
  size_t written = sitechEncode(xxr, data, std::size(data));
  EXPECT_TRUE(written==xxr.rawSize);
  SitechDeserializer sd(data, std::size(data));
  EXPECT_TRUE(sd.checksumOk());
  //todo:test actual bytes??
}

TEST(test_sitech, yxrEncode) {
  SitechYXR yxr;
  yxr.elPosSet = 123456;
  yxr.elSpeedSet = 234567;
  yxr.azPosSet = -123;
  yxr.azSpeedSet = 12;
  yxr.elRateAdder = 1;
  yxr.azRateAdder = 2;
  yxr.elRateAdderTime = 3;
  yxr.azRateAdderTime = 4;
  
  uint8_t data[yxr.rawSize];
  size_t written = sitechEncode(yxr, data, std::size(data));
  EXPECT_TRUE(written==yxr.rawSize);
  SitechDeserializer sd(data, std::size(data));
  EXPECT_TRUE(sd.checksumOk());
  //todo:test actual bytes??
}
