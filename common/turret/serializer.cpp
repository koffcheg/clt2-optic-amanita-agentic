

#include "serializer.hpp"
#include <bit>

namespace turret {

Serializer::Serializer(Data &data) : m_data(data){};

void Serializer::write(int8_t x) { m_data.push_back(x); }

void Serializer::write(int32_t x) {
  m_data.push_back((x >> 8 * 3) & 0xFF);
  m_data.push_back((x >> 8 * 2) & 0xFF);
  m_data.push_back((x >> 8 * 1) & 0xFF);
  m_data.push_back((x >> 8 * 0) & 0xFF);
}

void Serializer::write(int64_t x) {
  m_data.push_back((x >> 8 * 7) & 0xFF);
  m_data.push_back((x >> 8 * 6) & 0xFF);
  m_data.push_back((x >> 8 * 5) & 0xFF);
  m_data.push_back((x >> 8 * 4) & 0xFF);
  m_data.push_back((x >> 8 * 3) & 0xFF);
  m_data.push_back((x >> 8 * 2) & 0xFF);
  m_data.push_back((x >> 8 * 1) & 0xFF);
  m_data.push_back((x >> 8 * 0) & 0xFF);
}

template <typename T> void Serializer::writeFD(T x) {
  char *p = reinterpret_cast<char *>(&x);
  // todo: check float endianness
  if (std::endian::native == std::endian::big) {
    for (size_t i = 0; i < sizeof(x); i++) {
      m_data.push_back(*p++);
    }
  } else {
    p += sizeof(x);
    for (size_t i = 0; i < sizeof(x); i++) {
      m_data.push_back(*--p);
    }
  }
}

void Serializer::write(float x) { writeFD(x); }

void Serializer::write(double x) { writeFD(x); }

void Serializer::write(bool x) { m_data.push_back(x ? 1 : 0); }

void Serializer::write(DateTime x) {
  int64_t Dt =
      duration_cast<std::chrono::nanoseconds>(x.time_since_epoch()).count();
  write(Dt);
}

void Serializer::write(const void *x, size_t size) {
  const unsigned char *p = static_cast<const unsigned char *>(x);
  for (size_t i = 0; i < size; i++) {
    m_data.push_back(*p++);
  }
}

// Deserialiazer
Deserializer::Deserializer(const Data &data) : m_data(data), m_idx(0){};

int8_t Deserializer::readInt8() {
  int8_t x = 0;
  read(x);
  return x;
}

int32_t Deserializer::readInt32() {
  int32_t x = 0;
  read(x);
  return x;
}

int64_t Deserializer::readInt64() {
  int64_t x = 0;
  read(x);
  return x;
}

template <typename T> void Deserializer::readFD(T &x) {
  char *p = reinterpret_cast<char *>(&x);
  // todo: check float endianness
  if (std::endian::native == std::endian::big) {
    for (size_t i = 0; i < sizeof(x); i++) {
      *p++ = m_data.at(m_idx++);
    }
  } else {
    p += sizeof(x);
    for (size_t i = 0; i < sizeof(x); i++) {
      *--p = m_data.at(m_idx++);
    }
  }
}

float Deserializer::readFloat() {
  float x = 0;
  readFD(x);
  return x;
}

double Deserializer::readDouble() {
  double x = 0;
  readFD(x);
  return x;
}

bool Deserializer::readBool() { return m_data.at(m_idx++) == 0 ? false : true; }

DateTime Deserializer::readTime() {
  return DateTimeClock::time_point(std::chrono::nanoseconds(readInt64()));
}

void Deserializer::read(void *x, size_t size) {
  unsigned char *p = static_cast<unsigned char *>(x);
  for (size_t i = 0; i < size; i++) {
    *p++ = m_data.at(m_idx++);
  }
}
void Deserializer::read(int8_t &x) { x = m_data.at(m_idx++); }

void Deserializer::read(int32_t &x) {
  x = 0;
  x |= m_data.at(m_idx++) << 8 * 3;
  x |= m_data.at(m_idx++) << 8 * 2;
  x |= m_data.at(m_idx++) << 8 * 1;
  x |= m_data.at(m_idx++) << 8 * 0;
}

void Deserializer::read(int64_t &x) {
  x = 0;
  x |= (int64_t)m_data.at(m_idx++) << 8 * 7;
  x |= (int64_t)m_data.at(m_idx++) << 8 * 6;
  x |= (int64_t)m_data.at(m_idx++) << 8 * 5;
  x |= (int64_t)m_data.at(m_idx++) << 8 * 4;
  x |= (int64_t)m_data.at(m_idx++) << 8 * 3;
  x |= (int64_t)m_data.at(m_idx++) << 8 * 2;
  x |= (int64_t)m_data.at(m_idx++) << 8 * 1;
  x |= (int64_t)m_data.at(m_idx++) << 8 * 0;
}
void Deserializer::read(float &x) { readFD(x); }
void Deserializer::read(double &x) { readFD(x); }
void Deserializer::read(bool &x) { x = m_data.at(m_idx++) == 0 ? false : true; }
void Deserializer::read(DateTime &x) { x = readTime(); }

} // namespace turret