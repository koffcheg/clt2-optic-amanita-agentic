#pragma once

#include <cstdint>

#include <opencv2/core.hpp>

#include "dp1v2/domain/geometry.hpp"

namespace dp1v2 {

struct TileDesc {
    std::uint64_t frame_id = 0;
    int tile_id = -1;
    cv::Rect roi_with_border;
    cv::Rect valid_area;
    cv::Point origin_in_frame{0, 0};
    CoordinateSpace coordinate_space = CoordinateSpace::TileLocal;
};

} // namespace dp1v2
