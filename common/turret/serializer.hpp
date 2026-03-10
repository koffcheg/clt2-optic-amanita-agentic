
#pragma once

#include "turretTypes.hpp"
#include <functional>
#include <vector>

namespace turret {

using Data = std::vector<unsigned char>;

class Serializer {
public:
  explicit Serializer(Data &data);
  void write(int8_t x);
  void write(int32_t x);
  void write(int64_t x);
  void write(float x);
  void write(double x);
  void write(bool x);
  void write(DateTime x);
  void write(const void *x, size_t size);

private:
  template <typename T> void writeFD(T x);
  Data &m_data;
};

class Deserializer {
public:
  explicit Deserializer(const Data &data);
  int8_t readInt8();
  int32_t readInt32();
  int64_t readInt64();
  float readFloat();
  double readDouble();
  bool readBool();
  DateTime readTime();
  void read(void *x, size_t size);

  void read(int8_t &x);
  void read(int32_t &x);
  void read(int64_t &x);
  void read(float &x);
  void read(double &x);
  void read(bool &x);
  void read(DateTime &x);
  void read(const void *x, size_t size);

private:
  template <typename T> void readFD(T &x);

  const Data &m_data;
  int m_idx;
};

using Data = std::vector<unsigned char>;
using SlipDecoderCallback = std::function<void(const Data &decodedData)>;

} // namespace turret