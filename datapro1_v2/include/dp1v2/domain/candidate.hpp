#pragma once

#include <cstdint>

#include <opencv2/core.hpp>

#include "dp1v2/domain/geometry.hpp"

namespace dp1v2 {

enum class CandidateStatus : std::uint8_t {
    Provisional,
};

struct Candidate {
    std::uint64_t candidate_id = 0;
    std::uint64_t frame_id = 0;
    int tile_id = -1;
    int component_id = -1;
    cv::Rect bbox_px;
    cv::Point2f centroid_px{0.0F, 0.0F};
    int area_px = 0;
    float score = 0.0F;
    CandidateStatus status = CandidateStatus::Provisional;
    std::uint32_t quality_flags = 0;
    CoordinateSpace coordinate_space = CoordinateSpace::TileLocal;
};

} // namespace dp1v2
