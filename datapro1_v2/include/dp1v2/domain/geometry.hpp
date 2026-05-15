#pragma once

#include <opencv2/core.hpp>

namespace dp1v2 {

enum class CoordinateSpace {
    FrameGlobal,
    TileLocal,
    RoiLocal,
};

struct FrameGeometry {
    int width = 0;
    int height = 0;
    cv::Point origin_px{0, 0};
};

} // namespace dp1v2
