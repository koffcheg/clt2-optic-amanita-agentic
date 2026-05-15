#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "dp1v2/domain/geometry.hpp"
#include "dp1v2/domain/pixel.hpp"
#include "dp1v2/domain/time.hpp"
#include "dp1v2/domain/status.hpp"

namespace dp1v2 {

struct PipelineConfig;

struct FrameContext {
    std::uint64_t frame_id = 0;
    int camera_id = -1;
    std::string source_id;
    std::string pipeline_run_id;
    const PipelineConfig *config_ref = nullptr;
    int worker_count = 0;
    PixelFormat input_format = PixelFormat::U16;
    InputBitDepth input_bit_depth = InputBitDepth::Bit16;
    FrameGeometry geometry;
    TimestampRef acquisition_time;
    std::vector<StageStatus> stage_statuses;
    std::vector<DiagnosticMessage> diagnostics;
};

} // namespace dp1v2
