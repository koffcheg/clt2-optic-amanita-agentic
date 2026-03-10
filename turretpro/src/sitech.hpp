

#pragma once

#include <cstdint>
#include <stddef.h>

namespace turret {

class SitechDeserializer {
public:
  explicit SitechDeserializer(const unsigned char *data, size_t size);
  SitechDeserializer(const SitechDeserializer &) = delete;
  template <typename T> T read();
  void read(void *x, size_t size);
  template <typename T> void read(T &x);
  size_t dataLeft() { return m_size; };
  bool checksumOk() { return m_checksumOk; }

private:
  void checkSize(size_t size);
  const unsigned char *m_data;
  size_t m_size;
  bool m_checksumOk;
};

class SitechSerializer {
public:
  explicit SitechSerializer(unsigned char *data, size_t size);
  SitechSerializer(const SitechDeserializer &) = delete;
  void write(const void *x, size_t size);
  template <typename T> void write(T x);
  size_t dataLeft() { return m_size; };
  size_t written() { return m_written; };

private:
  void checkSize(size_t size);
  unsigned char *m_data;
  size_t m_size;
  size_t m_written;
};

struct SitechStatus {
  static constexpr size_t rawSize = 41;

  bool decode(const unsigned char *buff, std::size_t size);

  uint8_t address; // 0: 0xA8 + controller address (1, 3, or 5)
  int el;         // 1-4:  Alt/Dec motor position
  int az;          // 5-8: Az/RA motor position
  int altScope;    // 9-12: Alt/Dec scope encoder position
  int azScope;     // 13-16: Az/RA scope encoder position
  union {
    uint8_t asInt;
    struct KeypadStatus { // 17: Keypad status
      bool left : 1;      // Bit 1: Left button pressed (0x01)
      bool right : 1;     // Bit 2: Right button pressed (0x02)
      bool up : 1;        // Bit 3: Up button pressed (0x04)
      bool down : 1;      // Bit 4: Down button pressed (0x08)
      bool pan : 1;    // Bit 5: Pan mode toggled (see note below) (0x10 = 16)
      bool rtn : 1;    // Bit 6: RTN button pressed (0x20 = 32)
      bool esc : 1;    // Bit 7: ESC button pressed (0x40 = 64)
      bool anyDir : 1; // Bit 8: Any directional button pressed
    };
  } keypadStatus;
  union {
    uint8_t asInt;
    struct {                 // 18: XBits (see XBits table for description)
      bool encReverse : 1;   // 0: motor encoder is reversed
      bool motReverse : 1;   // 1: motor polarity is reversed
      bool scopeReverse : 1; // 2: axis encoder is reversed
      bool dragAndTrack : 1; // 3: computerless Drag and Track mode
      bool tracking : 1;     // 4: tracking platform mode
      bool handPaddle : 1;   // 5: hand paddle is enabled
      bool newHandpad : 1;   // 6: new hand paddle
      bool guideMode : 1;    // 7: guide mode.
    };
  } xbits;

  union {
    uint8_t asInt;
    struct {                 // 19: YBits (see YBits table for description)
      bool encReverse : 1;   // 0: motor encoder is reversed
      bool motReverse : 1;   // 1: motor polarity is reversed
      bool scopeReverse : 1; // 2: axis encoder is reversed
      bool slewAndTrack : 1; // 3: computerless Slew and Track mode
      bool diHandpad0 : 1;   // 4: Digital input, or RA PEC Sensor sync
      bool diHandpad1 : 1;   // 5: Digital input
      bool diHandpad2 : 1;   // 6: Digital input
      bool diHandpad3 : 1;   // 7: Digital input
    };
  } ybits;

  union {
    uint8_t asInt;
    struct {             // 20: ExtraBits
      bool xStopped : 1; //  Bit 1: If set, X axis is stopped
      bool xManual : 1;  //  Bit 2: If set, X axis is in Manual (blinking?) mode
      bool DI0 : 1;      //  Bit 3: Digital In 0 (RA/Azm Home Mask).
      bool DI1 : 1;      //  Bit 4: Digital In 1 (Dec/Alt Home Mask).
      bool yStopped : 1; //  Bit 5: If set, Y axis is stopped
      bool Ymanual : 1;  //  Bit 6: If set, Y axis is in Manual (blinking?) mode
      bool YPECrec : 1;  //  Bit 7: Y PEC Recording
      bool YPECplay : 1; //  Bit 8: Y PEC Playing
    };
  } extraBits;
  uint16_t analog1;      // 21-22: Analog input 1
  uint16_t analog2;      // 23-24: Analog input 2
  uint32_t milliseconds; // 25-28: Millisecond clock
  uint8_t temperature;   // 29: Temperature, Degs F or Alt/Dec Worm Phase
  uint8_t azWormPhase;   // 30: Az/RA Worm Phase
  int altAtLastScope;    // 31-34: Alt/Dec motor location at last Alt/Dec change
  int azAtLastScope;     // 35-38: Az/RA motor location at last Az/RA change
  uint16_t checkSum;     // 29-40: Checksum (sum^0xFF00, LSB first)
};

struct SitechXXR { // The following 21 bytes follow the XXR\r ASCII command
  static constexpr int rawSize = 21;
  int32_t elPosSet;   // 0 - 3 Alt/Dec motor destination, in motor counts
  int32_t elSpeedSet; // 4 - 7 Alt/Dec speed, in counts per servo loop
  int32_t azPosSet;    // 8 - 11 Az/RA motor destination, in motor counts
  int32_t azSpeedSet;  // 12 - 15 Az/RA motor speed, in counts per servo loop
  uint8_t useBits;     // 16 bit 0:  use the following XBits and YBits values.
  uint8_t xBits;       // 17 XBits (ignored if Byte 16 bit 0 is 0)
  uint8_t yBits;       // 18 YBits (ignored if Byte 16 bit 0 is 0)
};

struct SitechYXR { // The following 21 bytes follow the XXR\r ASCII command
  static constexpr int rawSize = 34;
  int32_t elPosSet;    // 0-3 Alt motor destination, in motor counts
  int32_t elSpeedSet;  // 4-7 Alt speed (base rate), in counts per servo loop
  int32_t azPosSet;     // 8-11 Az motor destination, in motor counts
  int32_t azSpeedSet;   // 12-15 Az speed (base rate), in counts per servo loop
  int32_t elRateAdder; // 16-19 Alt Rate Adder
  int32_t azRateAdder;  // 20-23 Az Rate Adder
  int32_t elRateAdderTime; // 24-27 Alt Rate Adder time (servo loops; 1953=1s)
  int32_t azRateAdderTime;  // 28-31 Az Rate Adder time (servo loops; 1953=1s)
};

size_t sitechEncode(const SitechXXR &xxr, uint8_t *buffer, size_t size);
size_t sitechEncode(const SitechYXR &yxr, uint8_t *buffer, size_t size);

} // namespace turret
