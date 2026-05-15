#pragma once

#include <cstdint>
#include <vector>

#include <opencv2/core.hpp>

#include "dp1v2/domain/candidate.hpp"
#include "dp1v2/domain/measurement_record.hpp"
#include "dp1v2/domain/segment.hpp"
#include "dp1v2/domain/validated_object.hpp"

namespace dp1v2 {

struct TileResult {
    std::uint64_t frame_id = 0;
    int camera_id = -1;
    int tile_id = -1;
    cv::Rect valid_area;
    std::vector<Candidate> candidates;
    std::vector<Segment> segments;
    std::vector<ValidatedObject> objects;
    std::vector<MeasurementRecord> measurements;
};

} // namespace dp1v2
