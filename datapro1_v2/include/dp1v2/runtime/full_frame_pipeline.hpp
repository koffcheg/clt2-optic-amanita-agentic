#pragma once

#include "dp1v2/config/config.hpp"
#include "dp1v2/domain/canonical_frame.hpp"
#include "dp1v2/domain/frame_context.hpp"
#include "dp1v2/result/result_sink.hpp"
#include "dp1v2/runtime/runtime.hpp"
#include "dp1v2/stages/radiometric_stage.hpp"
#include "dp1v2/visualization/visualization_sink.hpp"

namespace dp1v2 {

class Stage2BoundaryAdapter;

struct FullFramePipelineArgs {
    const CanonicalFrame &frame;
    FrameContext &frame_context;
    const PipelineConfig &pipeline_config;
};

struct FullFramePipelineResult {
    FrameLifecycleResult lifecycle;
    ResultSinkOutcome sink;
};

class FullFramePipeline final {
public:
    FullFramePipeline(
        RadiometricStage &radiometric_stage,
        Stage2BoundaryAdapter &stage2_boundary_adapter,
        VisualizationSink &visualization_sink);

    FullFramePipelineResult process(const FullFramePipelineArgs &args);

private:
    RadiometricStage &radiometric_stage_;
    Stage2BoundaryAdapter &stage2_boundary_adapter_;
    VisualizationSink &visualization_sink_;
};

} // namespace dp1v2
