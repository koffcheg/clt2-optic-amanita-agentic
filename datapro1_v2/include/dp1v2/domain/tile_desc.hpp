#pragma once

#include <cstdint>

#include <opencv2/core.hpp>

#include "dp1v2/domain/geometry.hpp"

namespace dp1v2 {

// Coordinate semantics are intentionally mixed:
// - roi_with_border and origin_in_frame are in frame-global coordinates;
// - valid_area is tile-local, relative to roi_with_border.tl();
// - coordinate_space = TileLocal marks tile-local output mapping and does not
//   mean every rectangle in this descriptor is tile-local.
struct TileDesc {
    std::uint64_t frame_id = 0;
    int tile_id = -1;
    cv::Rect roi_with_border;
    cv::Rect valid_area;
    cv::Point origin_in_frame{0, 0};
    CoordinateSpace coordinate_space = CoordinateSpace::TileLocal;
};

} // namespace dp1v2
