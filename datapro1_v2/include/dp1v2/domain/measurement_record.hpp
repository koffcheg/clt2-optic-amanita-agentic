#pragma once

#include <cstdint>
#include <string>

#include <opencv2/core.hpp>

#include "dp1v2/domain/geometry.hpp"
#include "dp1v2/domain/pixel.hpp"
#include "dp1v2/domain/time.hpp"

namespace dp1v2 {

struct PhotometryStats {
    float mean_intensity = 0.0F;
    float max_intensity = 0.0F;
    float stddev_intensity = 0.0F;
    PixelFormat source_format = PixelFormat::U16;
    InputBitDepth source_bit_depth = InputBitDepth::Bit16;
};

struct MeasurementRecord {
    std::uint32_t schema_version = 1;
    std::uint64_t measurement_id = 0;
    std::uint64_t frame_id = 0;
    int camera_id = -1;
    std::string source_id;
    std::uint64_t source_object_id = 0;
    std::uint64_t source_segment_id = 0;
    TimestampRef time_ref;
    cv::Point2f position_px{0.0F, 0.0F};
    cv::Rect bbox_px;
    int area_px = 0;
    PhotometryStats photometry;
    CoordinateSpace coordinate_space = CoordinateSpace::FrameGlobal;
    std::uint32_t quality_flags = 0;
};

} // namespace dp1v2
