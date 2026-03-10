
#include "sitech.hpp"
#include <cstring>
#include <stdexcept>

namespace turret {

static uint16_t calculateChecksum(const unsigned char *data, size_t size) {
  uint16_t cs = 0;
  while (size--) {
    cs += *data++;
  }
  cs ^= 0xFF00; // invert high byte
  return cs;
}

SitechDeserializer::SitechDeserializer(const unsigned char *data, size_t size)
    : m_data(data), m_size(size), m_checksumOk(false) {

  if (size > 2) {
    uint16_t cs = data[size - 2] | (data[size - 1] << 8);
    m_checksumOk = calculateChecksum(data, size - 2) == cs;
  }
}

void SitechDeserializer::checkSize(size_t size) {
  if (m_size < size) {
    throw std::runtime_error("SitechDeserializer: no data");
  }
}

template <typename T> T SitechDeserializer::read() {
  checkSize(sizeof(T));
  T r = 0;
  for (size_t i = 0; i < sizeof(T); i++) {
    r |= (*m_data++) << (8 * i);
  }
  m_size -= sizeof(r);
  return r;
}

void SitechDeserializer::read(void *x, size_t size) {
  checkSize(size);
  std::memcpy(x, m_data, size);
  m_data += size;
  m_size -= size;
}

template <typename T> void SitechDeserializer::read(T &x) { x = read<T>(); }

SitechSerializer::SitechSerializer(unsigned char *data, size_t size)
    : m_data(data), m_size(size), m_written(0) {}

void SitechSerializer::write(const void *x, size_t size) {
  checkSize(size);
  std::memcpy(m_data, x, size);
  m_written += size;
  m_data += size;
  m_size -= size;
}

template <typename T> void SitechSerializer::write(T x) {
  checkSize(sizeof(x));
  for (size_t i = 0; i < sizeof(x); i++) {
    *m_data++ = (x >> (8 * i)) & 0xFF;
  }
  m_written += sizeof(x);
  m_size -= sizeof(x);
}

void SitechSerializer::checkSize(size_t size) {
  if (m_size < size) {
    throw std::runtime_error("SitechSerializer: no enough space");
  }
}

// SitechStatus
bool SitechStatus::decode(const unsigned char *buff, std::size_t size) {
  if (size != rawSize) {
    return false; // todo: error code / exception?
  }
  SitechDeserializer sd(buff, size);
  address = sd.read<uint8_t>() - 0xA8;
  sd.read(el);
  sd.read(az);
  sd.read(altScope);
  sd.read(azScope);
  sd.read(keypadStatus.asInt);
  sd.read(xbits.asInt);
  sd.read(ybits.asInt);
  sd.read(extraBits.asInt);
  sd.read(analog1);
  sd.read(analog2);
  sd.read(milliseconds);
  sd.read(temperature);
  sd.read(azWormPhase);
  sd.read(altAtLastScope);
  sd.read(azAtLastScope);
  sd.read(checkSum);
  return sd.dataLeft() == 0 &&
         (checkSum == calculateChecksum(buff, size - sizeof(checkSum)));
}

size_t sitechEncode(const SitechXXR &xxr, uint8_t *buffer, size_t size) {
  if (size < xxr.rawSize) {
    return 0;
  }
  SitechSerializer ss(buffer, size);
  ss.write(xxr.elPosSet);
  ss.write(xxr.elSpeedSet);
  ss.write(xxr.azPosSet);
  ss.write(xxr.azSpeedSet);
  ss.write(xxr.useBits);
  ss.write(xxr.xBits);
  ss.write(xxr.yBits);
  ss.write(calculateChecksum(buffer, ss.written()));
  return ss.written();
}

size_t sitechEncode(const SitechYXR &yxr, uint8_t *buffer, size_t size) {
  if (size < yxr.rawSize) {
    return 0;
  }
  SitechSerializer ss(buffer, size);
  ss.write(yxr.elPosSet);
  ss.write(yxr.elSpeedSet);
  ss.write(yxr.azPosSet);
  ss.write(yxr.azSpeedSet);
  ss.write(yxr.elRateAdder);
  ss.write(yxr.azRateAdder);
  ss.write(yxr.elRateAdderTime);
  ss.write(yxr.azRateAdderTime);
  ss.write(calculateChecksum(buffer, ss.written()));
  return ss.written();
}

} // namespace turret
