#pragma once

#include <cstdint>
#include <string>

#include <opencv2/core.hpp>

#include "dp1v2/domain/geometry.hpp"
#include "dp1v2/domain/pixel.hpp"

namespace dp1v2 {

struct TileBinaryMask {
    std::uint64_t frame_id = 0;
    int tile_id = -1;
    std::string source_ref;
    cv::Mat mask;
    PixelFormat pixel_format = PixelFormat::MaskU8;
    FrameGeometry geometry;
    cv::Point origin_in_frame{0, 0};
    cv::Rect valid_area;
    std::uint8_t background_value = 0;
    std::uint8_t foreground_value = 255;
    CoordinateSpace coordinate_space = CoordinateSpace::TileLocal;
};

} // namespace dp1v2
