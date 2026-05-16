#pragma once

#include <string>

#include "dp1v2/config/config.hpp"
#include "dp1v2/result/result_sink.hpp"
#include "dp1v2/runtime/runtime.hpp"
#include "dp1v2/source/source.hpp"
#include "dp1v2/stages/radiometric_stage.hpp"

namespace dp1v2 {

struct SingleFramePipelineResult {
    FrameLifecycleResult lifecycle;
    ResultSinkOutcome sink;
    std::string evidence_path;
};

SingleFramePipelineResult process_single_frame(
    const RawFrameEnvelope &envelope,
    int cam_index,
    const PipelineConfig &pipeline_config,
    RadiometricStage &radiometric_stage);

} // namespace dp1v2
