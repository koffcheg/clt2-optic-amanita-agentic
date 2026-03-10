#include "slip.hpp"
#include <gtest/gtest.h>
#include <vector>

using namespace slip;

static int onFrameFired = 0;
static Data onFrameData;

void onFrame(const Data &decodedData) {
  onFrameFired++;
  onFrameData = decodedData;
}

void test_encode_decode(const Data &original, const Data &encodedRef) {
  // Encode data
  Data encoded;
  encode(original, encoded);
  EXPECT_TRUE(encoded == encodedRef);

  Decoder decoder(onFrame);
  onFrameData.clear();
  onFrameFired = 0;
  decoder.decode(encoded);
  EXPECT_TRUE(onFrameFired == 1 || original.empty());
  EXPECT_TRUE(onFrameData == original);
  EXPECT_TRUE(decoder.frameErrorCount() == 0);
}

void test_decode_invalid(const Data &encoded, const Data &decoded,
                         int errCount) {
  Decoder decoder(onFrame);
  onFrameData.clear();
  onFrameFired = 0;
  decoder.decode(encoded);
  EXPECT_TRUE(onFrameFired == 1);
  EXPECT_TRUE(onFrameData == decoded);
  EXPECT_TRUE(decoder.frameErrorCount() == errCount);
}

TEST(test_slip, test_slip_empty) { test_encode_decode(Data{}, Data{0xC0}); }

TEST(test_slip, test_slip_00) {
  test_encode_decode(Data{0x00}, Data{0x00, 0xC0});
}

TEST(test_slip, test_slip_00_01) {
  test_encode_decode(Data{0x00, 0x01}, Data{0x00, 0x01, 0xC0});
}

TEST(test_slip, test_slip_00_01_C0_DC) {
  test_encode_decode(Data{0x00, 0x01, 0xC0, 0xDB},
                     Data{0x00, 0x01, 0xDB, 0xDC, 0xDB, 0xDD, 0xC0});
}

TEST(test_slip, test_slip_decode_3xEND) {
  test_decode_invalid(Data{0xC0, 0x00, 0x01, 0xC0, 0xC0}, Data{0x00, 0x01}, 0);
}

TEST(test_slip, test_slip_decode_invalid_stuff) {
  test_decode_invalid(Data{0x00, 0x01, 0xDB, 0xDB, 0xDB, 0xDD, 0xC0},
                      Data{0x00, 0x01, 0xDB}, 1);
}

//todo: more tests? (reset, restore)
