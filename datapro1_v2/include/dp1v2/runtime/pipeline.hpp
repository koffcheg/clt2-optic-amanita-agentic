#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "dp1v2/config/config.hpp"
#include "dp1v2/domain/frame_context.hpp"
#include "dp1v2/result/result_sink.hpp"
#include "dp1v2/runtime/full_frame_pipeline.hpp"
#include "dp1v2/runtime/runtime.hpp"
#include "dp1v2/runtime/tile_pipeline.hpp"
#include "dp1v2/source/source.hpp"
#include "dp1v2/stages/input_normalization_stage.hpp"
#include "dp1v2/stages/prep_stage.hpp"

namespace dp1v2 {

struct FrameContextSnapshot {
    std::uint64_t frame_id = 0;
    int camera_id = -1;
    std::string source_id;
    std::vector<StageStatus> stage_statuses;
    FrameProfiling profiling;
    std::vector<DiagnosticMessage> diagnostics;
    FrameArtifactRegistry artifacts;
};

struct SingleFramePipelineResult {
    FrameLifecycleResult lifecycle;
    ResultSinkOutcome sink;
    FrameContextSnapshot frame;
};

SingleFramePipelineResult process_single_frame(
    const RawFrameEnvelope &envelope,
    int cam_index,
    const PipelineConfig &pipeline_config,
    const ResolvedPipelineConfig &resolved_pipeline_config,
    const InputNormalizationStage &input_normalization_stage,
    PrepStage &prep_stage,
    FullFramePipeline &full_frame_pipeline,
    TilePipeline &tile_pipeline);

} // namespace dp1v2
