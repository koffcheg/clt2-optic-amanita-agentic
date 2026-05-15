#pragma once

#include <cstdint>

#include <opencv2/core.hpp>

#include "dp1v2/domain/geometry.hpp"

namespace dp1v2 {

enum class ObjectValidationStatus : std::uint8_t {
    Accepted,
    Rejected,
};

enum class ObjectSourceKind : std::uint8_t {
    Candidate,
    Segment,
};

struct ValidatedObject {
    std::uint64_t object_id = 0;
    std::uint64_t frame_id = 0;
    int camera_id = -1;
    int tile_id = -1;
    ObjectSourceKind source_kind = ObjectSourceKind::Segment;
    std::uint64_t source_candidate_id = 0;
    std::uint64_t source_segment_id = 0;
    cv::Rect bbox_px;
    cv::Point2f centroid_px{0.0F, 0.0F};
    int area_px = 0;
    float detection_score = 0.0F;
    float validation_score = 0.0F;
    ObjectValidationStatus status = ObjectValidationStatus::Accepted;
    std::uint32_t quality_flags = 0;
    std::uint32_t reject_flags = 0;
    CoordinateSpace coordinate_space = CoordinateSpace::TileLocal;
};

} // namespace dp1v2
