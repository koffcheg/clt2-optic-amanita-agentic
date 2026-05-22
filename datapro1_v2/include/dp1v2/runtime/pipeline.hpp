#pragma once

#include <string>
#include <vector>

#include "dp1v2/config/config.hpp"
#include "dp1v2/domain/frame_context.hpp"
#include "dp1v2/result/result_sink.hpp"
#include "dp1v2/runtime/runtime.hpp"
#include "dp1v2/source/source.hpp"
#include "dp1v2/stages/input_normalization_stage.hpp"
#include "dp1v2/stages/prep_stage.hpp"
#include "dp1v2/stages/radiometric_stage.hpp"
#include "dp1v2/visualization/visualization_sink.hpp"

namespace dp1v2 {

struct FrameContextSnapshot {
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
    const InputNormalizationStage &input_normalization_stage,
    PrepStage &prep_stage,
    RadiometricStage &radiometric_stage,
    VisualizationSink &visualization_sink);

} // namespace dp1v2
