

#pragma once

#include <chrono>
#include <cstddef>
#include <limits>
#include <opencv2/opencv.hpp>

namespace cam_pro {
const int MAX_PIXEL_COUNT = std::numeric_limits<int>::max();
const int MAX_CAMERA_NAME_LENGTH = 32;
const int FRAME_VERSION = 0xCA000101;
const std::size_t DATA_ALIGNMENT = alignof(std::max_align_t);

using DateTimeClock = std::chrono::system_clock;
using DateTime = DateTimeClock::time_point;

struct FrameHeader {
  int index;         // Frame index
  int width;         // image width in pixels
  int height;        // image height in pixels
  int bytesPerPixel; // pixel format
  int bitDepth;      // Bits per pixel per channel
  // char cameraName[MAX_CAMERA_NAME_LENGTH]; //Чи ми цього потребуємо?
  DateTime exposureStart; 
  float exposureLength;   // s
  float focalLength;      // m
  float pixelWidth;       // m
  float pixelHeight;      // m
  bool turretInfoValid;
  float Azimuth;
  float Elevation;
  float AzimuthSpeed;
  float ElevationSpeed;
  int reserved[16];
  // std::size_t size() {

  /// @brief Calculates size of header including data alignment
  /// @return
  inline std::size_t size() {
    return sizeof(*this) + DATA_ALIGNMENT -
           sizeof(*this) % DATA_ALIGNMENT; // Frame data alignment
  }
};

struct Frame {
  FrameHeader header;

  /// @brief Caluclates offset of the data relative to the structure start
  /// taking into account DATA_ALIGNMENT requirement
  /// @return
  inline static std::size_t getDataOffset() {
    return sizeof(header) +
           (DATA_ALIGNMENT - sizeof(header) % DATA_ALIGNMENT) % DATA_ALIGNMENT;
    // data alignment
  };

  inline std::size_t dataSize() {
    return header.width * header.height * header.bytesPerPixel;
  }

  /// @brief Caluclates frame size (Header+Data+DataAlignment)
  inline std::size_t size() {
    return size(header.width, header.height, header.bytesPerPixel);
  };

  /// @brief Caluclates frame size (Header+Data+DataAlignment) (static method)
  static inline std::size_t size(int width, int height, int bytesPerPixel) {
    std::size_t size = getDataOffset(); // sizeof(header)+data alignment
    // add image data size
    size += width * height * bytesPerPixel;
    // add alignment of the structure as a whole
    size += (DATA_ALIGNMENT - size % DATA_ALIGNMENT) % DATA_ALIGNMENT;
    return size;
  };

  void *data() { return reinterpret_cast<char *>(this) + getDataOffset(); };

  const void *data() const {
    return reinterpret_cast<const char *>(this) + getDataOffset();
  };

  /// @brief Creates a cv::Mat instance based on frame data
  /// @return cv::Mat object
  cv::Mat asMat();
};

} // namespace cam_pro
