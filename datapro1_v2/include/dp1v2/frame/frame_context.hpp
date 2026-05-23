#pragma once

#include "dp1v2/domain/frame_context.hpp"
#include "dp1v2/domain/canonical_frame.hpp"
#include "dp1v2/domain/frame_packet.hpp"
#include "dp1v2/domain/processing_frame.hpp"

#include <chrono>
#include <cstdint>
#include <string_view>

namespace dp1v2 {

struct StageStatusUpdate {
    std::string_view stage_key;
    StageStatusCode status = StageStatusCode::NotStarted;
    std::string_view variant;
    std::string_view level;
    std::string_view route;
    std::string_view reason;
};

FrameContext build_frame_context(const FramePacket &packet, int cam_index);
FrameArtifactRef register_raw_frame_artifact(FrameContext &context, const FramePacket &packet);
FrameArtifactRef register_canonical_frame_artifact(FrameContext &context, const CanonicalFrame &frame);
const FrameArtifactRef *find_frame_artifact_by_id(const FrameContext &context, std::string_view id);
const FrameArtifactRef *find_frame_artifact_by_stage(
    const FrameContext &context,
    std::string_view producer_stage);
FrameArtifactRef register_radiometric_processing_artifact(
    FrameContext &context,
    const ProcessingFrame &frame);
StageTiming record_stage_timing(
    FrameContext &context,
    std::string_view stage_key,
    StageStatusCode status,
    std::string_view variant,
    std::string_view level,
    PixelFormat input_format,
    PixelFormat output_format,
    std::chrono::steady_clock::time_point start_time,
    std::chrono::steady_clock::time_point end_time,
    std::string_view reason = {});
StageStatus record_stage_status(FrameContext &context, const StageStatusUpdate &update);
DiagnosticMessage record_diagnostic(
    FrameContext &context,
    std::string_view code,
    std::string_view message);
void set_frame_tile_count(FrameContext &context, std::uint32_t tile_count);

} // namespace dp1v2
