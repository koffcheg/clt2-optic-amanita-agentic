#pragma once

#include <cstdint>
#include <optional>
#include <string>

#include <opencv2/core.hpp>

#include "dp1v2/domain/geometry.hpp"
#include "dp1v2/domain/pixel.hpp"
#include "dp1v2/domain/time.hpp"

namespace dp1v2 {

struct FramePacket {
    std::uint64_t frame_id = 0;
    int camera_id = -1;
    std::string source_id;
    cv::Mat image;
    PixelFormat pixel_format = PixelFormat::U16;
    InputBitDepth bit_depth = InputBitDepth::Bit16;
    PixelRange pixel_range;
    FrameGeometry geometry;
    TimestampRef ingest_time;
    std::optional<TimestampRef> acquisition_time;
};

} // namespace dp1v2
