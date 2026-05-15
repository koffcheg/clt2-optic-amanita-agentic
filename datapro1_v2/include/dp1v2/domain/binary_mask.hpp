#pragma once

#include <cstdint>
#include <string>

#include <opencv2/core.hpp>

#include "dp1v2/domain/geometry.hpp"
#include "dp1v2/domain/pixel.hpp"

namespace dp1v2 {

struct BinaryMask {
    std::uint64_t frame_id = 0;
    std::string source_ref;
    cv::Mat mask;
    PixelFormat pixel_format = PixelFormat::MaskU8;
    FrameGeometry geometry;
    std::uint8_t background_value = 0;
    std::uint8_t foreground_value = 255;
    CoordinateSpace coordinate_space = CoordinateSpace::FrameGlobal;
};

} // namespace dp1v2
