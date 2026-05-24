#pragma once

#include <cstdint>

#include "dp1v2/config/config.hpp"
#include "dp1v2/domain/frame_context.hpp"
#include "dp1v2/result/result_sink.hpp"
#include "dp1v2/runtime/runtime.hpp"
#include "dp1v2/runtime/tile_executor.hpp"
#include "dp1v2/runtime/tile_frame_aggregator.hpp"
#include "dp1v2/runtime/tile_processor.hpp"
#include "dp1v2/stages/prep_stage.hpp"
#include "dp1v2/stages/radiometric_stage.hpp"
#include "dp1v2/visualization/visualization_sink.hpp"

namespace dp1v2 {

struct TilePipelineArgs {
    const PrepTilesOutput& prep_output;
    FrameContext& frame_context;

    const PipelineConfig& pipeline_config;
    const ResolvedPipelineConfig& resolved_pipeline_config;
    const PrepTilesParametersConfig& tiles_config;

    std::uint64_t frame_id = 0;
    int camera_id = -1;
};

struct TilePipelineResult {
    FrameLifecycleResult lifecycle;
    ResultSinkOutcome sink;
    TileExecutionSummary execution;
    TileFrameAggregationSummary aggregation;
};

class TilePipeline final {
public:
    TilePipeline(
        RadiometricStage& radiometric_stage,
        TileExecutor& executor,
        TileFrameAggregator& aggregator,
        VisualizationSink& visualization_sink);

    TilePipelineResult process(const TilePipelineArgs& args);

private:
    RadiometricStage& radiometric_stage_;
    TileExecutor& executor_;
    TileFrameAggregator& aggregator_;
    VisualizationSink& visualization_sink_;
};

} // namespace dp1v2
