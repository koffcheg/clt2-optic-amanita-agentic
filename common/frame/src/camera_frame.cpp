
#include "camera_frame.hpp"
#include <chrono>
#include <cstddef>
#include <limits>

namespace cam_pro {

cv::Mat Frame::asMat() {
  switch (header.bytesPerPixel) {
  case 2:
    return cv::Mat(header.height, header.width, CV_16UC1, data());
  default:
    throw std::runtime_error("unknown image format");
  }
}

} // namespace cam_pro
