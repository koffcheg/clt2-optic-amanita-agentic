
#pragma once

#include <chrono>
#include <math.h>
#include <memory>
#include <stdexcept>
#include <string>

namespace u {

constexpr double DEG = M_PI / 180.0;

constexpr long double operator""_deg(long double val) { return val * DEG; }

inline double deg2rad(double d) { return d * DEG; }

inline double rad2deg(double r) { return r / DEG; }

/// @brief Returns x coerced to the range min..max
/// @tparam T  data type
/// @param x  input value
/// @param min minimum limit
/// @param max maximim limit
/// @return x if min<x<max, else min or max
template <typename T> T coerce(T x, T min, T max) {
  if (x < min) {
    return min;
  }
  if (x > max) {
    return max;
  }
  return x;
}

/// @brief Returns x coerced to the range ±maxAbs
/// @tparam T  data type
/// @param x  input value
/// @param maxAbs maximim limit
/// @return x if min<x<max, else min or max
template <typename T> T coerceAbs(T x, T maxAbs) {
  if (x < -maxAbs) {
    return -maxAbs;
  }
  if (x > maxAbs) {
    return maxAbs;
  }
  return x;
}

template <typename... Args>
std::string stringFormat(const std::string &format, Args... args) {
  int size = snprintf(nullptr, 0, format.c_str(), args...) + 1;
  //+1 for extra '\0'
  if (size <= 0) {
    throw std::runtime_error("Error during formatting.");
  }
  std::unique_ptr<char[]> buf(new char[size]);
  snprintf(buf.get(), size, format.c_str(), args...);
  return std::string(buf.get(),
                     buf.get() + size - 1); // -1 removes '\0' inside
}

template <typename... Args>
std::string stringFormatNoExcept(const std::string &format, Args... args) {
  try {
    return stringFormat(format, std::forward<Args>(args)...);
  } catch (const std::exception &e) {
    return e.what();
  }
}

// returns the size of array at compile time.
template <std::size_t N, class T> constexpr std::size_t countof(T (&)[N]) {
  return N;
}

inline double timeDiff(std::chrono::system_clock::time_point t0,
                       std::chrono::system_clock::time_point t1) {
  return std::chrono::duration<double>(t1 - t0).count();
}

inline double timeDiff(std::chrono::steady_clock::time_point t0,
                       std::chrono::steady_clock::time_point t1) {
  return std::chrono::duration<double>(t1 - t0).count();
}

inline double toSeconds(std::chrono::system_clock::duration t) {
  return std::chrono::duration<double>(t).count();
}

inline double stoddef(const std::string &str, double defaultValue) {
  try {
    return std::stod(str);
  } catch (const std::invalid_argument &e) {
    // Handle the case where the string cannot be converted to a double
    return defaultValue;
  } catch (const std::out_of_range &e) {
    // Handle the case where the string represents a value out of the range of a
    // double
    return defaultValue;
  }
}

} // namespace u