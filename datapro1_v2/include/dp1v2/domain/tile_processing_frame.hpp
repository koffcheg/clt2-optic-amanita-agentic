#pragma once

#include <cstdint>

#include <opencv2/core.hpp>

#include "dp1v2/domain/geometry.hpp"
#include "dp1v2/domain/pixel.hpp"

namespace dp1v2 {

struct TileProcessingFrame {
    std::uint64_t frame_id = 0;
    int tile_id = -1;
    cv::Mat image;
    PixelFormat pixel_format = PixelFormat::F32;
    PixelRange value_range;
    ProcessingDomain processing_domain = ProcessingDomain::RadiometricResidual;
    RangePolicy range_policy = RangePolicy::SignedResidual;
    FrameGeometry geometry;
    cv::Point origin_in_frame{0, 0};
    cv::Rect valid_area;
    CoordinateSpace coordinate_space = CoordinateSpace::TileLocal;
};

} // namespace dp1v2
