#pragma once

#include <cstdint>
#include <optional>

#include <opencv2/core.hpp>

#include "dp1v2/domain/geometry.hpp"
#include "dp1v2/domain/pixel.hpp"

namespace dp1v2 {

struct ProcessingFrame {
    std::uint64_t frame_id = 0;
    std::optional<std::uint64_t> source_frame_id;
    cv::Mat image;
    PixelFormat pixel_format = PixelFormat::F32;
    PixelRange value_range;
    ProcessingDomain processing_domain = ProcessingDomain::RadiometricResidual;
    RangePolicy range_policy = RangePolicy::SignedResidual;
    FrameGeometry geometry;
    CoordinateSpace coordinate_space = CoordinateSpace::FrameGlobal;
};

} // namespace dp1v2
