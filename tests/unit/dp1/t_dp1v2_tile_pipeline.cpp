#include <gtest/gtest.h>

#include <algorithm>
#include <string_view>

#include "dp1v2/frame/frame_context.hpp"
#include "dp1v2/runtime/tile_pipeline.hpp"

namespace {

dp1v2::StageConfig stageConfig(
    const bool enabled,
    const std::string& variant,
    const std::string& level)
{
    return dp1v2::StageConfig{
        .enabled = enabled,
        .variant = variant,
        .level = level,
        .parameters = {},
    };
}

dp1v2::PipelineConfig makePipelineConfig()
{
    dp1v2::PipelineConfig config{};
    config.stages.radiometric = stageConfig(true, "inverse_median", "L0");
    return config;
}

dp1v2::PrepTilesParametersConfig makeTilesConfig()
{
    return dp1v2::PrepTilesParametersConfig{
        .tile_width = 2,
        .tile_height = 2,
        .overlap_x = 0,
        .overlap_y = 0,
    };
}

const dp1v2::StageStatus* findStageStatus(
    const dp1v2::FrameContext& context,
    const std::string_view stage_key)
{
    const auto status = std::find_if(
        context.stage_statuses.begin(),
        context.stage_statuses.end(),
        [stage_key](const dp1v2::StageStatus& record) {
            return record.stage_key == stage_key;
        });
    return status == context.stage_statuses.end() ? nullptr : &(*status);
}

bool hasDiagnostic(
    const dp1v2::FrameContext& context,
    const std::string_view code)
{
    return std::any_of(
        context.diagnostics.begin(),
        context.diagnostics.end(),
        [code](const dp1v2::DiagnosticMessage& diagnostic) {
            return diagnostic.code == code;
        });
}

} // namespace

TEST(TilePipelineTest, TileAndViewCountMismatchReturnsControlledFailureBeforeExecution)
{
    const dp1v2::PipelineConfig pipeline_config = makePipelineConfig();
    dp1v2::ResolvedPipelineConfig resolved_config{};
    resolved_config.prep.tiles = makeTilesConfig();

    dp1v2::PrepTilesOutput prep_output{};
    prep_output.tiles.push_back(dp1v2::TileDesc{
        .frame_id = 42,
        .tile_id = 3,
    });

    dp1v2::FrameContext context{};
    context.frame_id = 42;
    context.camera_id = 7;

    dp1v2::RadiometricStage radiometric_stage;
    dp1v2::Stage2BoundaryAdapter stage2_boundary_adapter;
    dp1v2::TileExecutor executor;
    dp1v2::TileFrameAggregator aggregator;
    dp1v2::VisualizationSink visualization_sink(dp1v2::VisualizationConfig{});
    dp1v2::TilePipeline pipeline(
        radiometric_stage,
        stage2_boundary_adapter,
        executor,
        aggregator,
        visualization_sink);

    const dp1v2::TilePipelineResult result = pipeline.process(
        dp1v2::TilePipelineArgs{
            .prep_output = prep_output,
            .frame_context = context,
            .pipeline_config = pipeline_config,
            .resolved_pipeline_config = resolved_config,
            .tiles_config = *resolved_config.prep.tiles,
            .frame_id = 42,
            .camera_id = 7,
        });

    EXPECT_EQ(result.lifecycle.status, dp1v2::FrameTerminalStatus::Failed);
    EXPECT_EQ(result.lifecycle.reason, "prep tiles and tile views size mismatch");
    EXPECT_EQ(result.execution.total_tasks, 0U);
    EXPECT_EQ(result.aggregation.total_tiles, 1U);
    EXPECT_EQ(context.profiling.cardinality.tile_count, 1U);
    EXPECT_TRUE(hasDiagnostic(context, "tile_pipeline.validation_failed"));

    const dp1v2::StageStatus* status = findStageStatus(context, "radiometric_correction");
    ASSERT_NE(status, nullptr);
    EXPECT_EQ(status->status, dp1v2::StageStatusCode::Failed);
    EXPECT_EQ(status->route, "tiles");
    EXPECT_EQ(status->reason, "prep tiles and tile views size mismatch");
}
