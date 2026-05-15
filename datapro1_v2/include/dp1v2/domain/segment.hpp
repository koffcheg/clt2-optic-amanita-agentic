#pragma once

#include <cstdint>
#include <optional>
#include <vector>

#include <opencv2/core.hpp>

#include "dp1v2/domain/geometry.hpp"

namespace dp1v2 {

struct Segment {
    std::uint64_t segment_id = 0;
    std::uint64_t frame_id = 0;
    int tile_id = -1;
    std::uint64_t candidate_id = 0;
    cv::Rect bbox_px;
    int area_px = 0;
    std::optional<std::vector<cv::Point>> contour_px;
    std::uint32_t quality_flags = 0;
    CoordinateSpace coordinate_space = CoordinateSpace::TileLocal;
};

} // namespace dp1v2
