#include <gtest/gtest.h>

#include <algorithm>
#include <chrono>
#include <string>
#include <string_view>

#include <opencv2/core.hpp>

#include "dp1v2/frame/frame_context.hpp"
#include "dp1v2/runtime/pipeline.hpp"
#include "dp1v2/runtime/runtime_profiling_aggregator.hpp"
#include "dp1v2/runtime/runtime_threading.hpp"

namespace dp1v2 {

ResultSinkOutcome publish_result_to_sinks(const TDataRes &)
{
    return ResultSinkOutcome{
        .send_status = ResultSinkStatus::SendSkipped,
        .artifact_status = ResultSinkStatus::ArtifactSkipped,
        .reason = "test_result_sink_stub",
    };
}

} // namespace dp1v2

namespace {

dp1v2::PixelRange rangeU16()
{
    return dp1v2::PixelRange{
        .min_value = 0.0,
        .max_value = 65535.0,
        .black_level = 0.0,
        .saturation_level = 65535.0,
    };
}

dp1v2::StageConfig stageConfig(
    const bool enabled,
    const std::string &variant,
    const std::string &level)
{
    return dp1v2::StageConfig{
        .enabled = enabled,
        .variant = variant,
        .level = level,
        .parameters = {},
    };
}

dp1v2::PipelineConfig makeTilesPipelineConfig()
{
    dp1v2::PipelineConfig config{};
    config.schema_version = "1.0";
    config.profile = "test";
    config.input_route = dp1v2::InputRouteConfig{
        .pixel_format = dp1v2::PixelFormat::U16,
        .bit_depth = dp1v2::InputBitDepth::Bit16,
        .pixel_range = rangeU16(),
    };
    config.stages.input_normalization = stageConfig(true, "passthrough", "L0");
    config.stages.prep = stageConfig(true, "tiles", "L1");
    config.stages.radiometric = stageConfig(true, "inverse_median", "L0");
    return config;
}

dp1v2::PipelineConfig makeFullFramePipelineConfig()
{
    dp1v2::PipelineConfig config = makeTilesPipelineConfig();
    config.stages.prep = stageConfig(true, "full_frame", "L0");
    return config;
}

dp1v2::RadiometricResolvedConfig makeRadiometricResolvedConfig()
{
    dp1v2::RadiometricResolvedConfig config{};
    config.inverse_median = dp1v2::InverseMedianParametersConfig{};
    return config;
}

dp1v2::PrepResolvedConfig makePrepResolvedConfig()
{
    dp1v2::PrepResolvedConfig config{};
    config.tiles = dp1v2::PrepTilesParametersConfig{
        .tile_width = 2,
        .tile_height = 2,
        .overlap_x = 0,
        .overlap_y = 0,
    };
    return config;
}

dp1v2::ResolvedPipelineConfig makeResolvedPipelineConfig()
{
    dp1v2::ResolvedPipelineConfig config{};
    config.input_normalization = dp1v2::InputNormalizationResolvedConfig{
        .binning_mode = dp1v2::BinningMode::None,
        .bin_factor = 1,
    };
    config.prep = makePrepResolvedConfig();
    config.radiometric = makeRadiometricResolvedConfig();
    return config;
}

dp1v2::RawFrameEnvelope makeEnvelope()
{
    cv::Mat image(4, 5, CV_16UC1);
    image.setTo(cv::Scalar(1024));

    dp1v2::RawFrameEnvelope envelope{};
    envelope.frame = image;
    envelope.header_hint.frame_id = 42;
    envelope.header_hint.camera_id = 7;
    envelope.header_hint.bit_depth = 16;
    envelope.received_steady_ts = std::chrono::steady_clock::now();
    return envelope;
}

dp1v2::SingleFramePipelineResult processTestFrame(
    const dp1v2::RawFrameEnvelope &envelope,
    const dp1v2::PipelineConfig &pipeline_config,
    const dp1v2::ResolvedPipelineConfig &resolved_pipeline_config,
    dp1v2::InputNormalizationStage &input_normalization_stage,
    dp1v2::PrepStage &prep_stage,
    dp1v2::RadiometricStage &radiometric_stage,
    dp1v2::VisualizationSink &visualization_sink,
    const int cam_index = 7)
{
    dp1v2::FullFramePipeline full_frame_pipeline(radiometric_stage, visualization_sink);
    dp1v2::TileExecutor tile_executor;
    dp1v2::TileFrameAggregator tile_aggregator;
    dp1v2::TilePipeline tile_pipeline(
        radiometric_stage,
        tile_executor,
        tile_aggregator,
        visualization_sink);

    return dp1v2::process_single_frame(
        envelope,
        cam_index,
        pipeline_config,
        resolved_pipeline_config,
        input_normalization_stage,
        prep_stage,
        full_frame_pipeline,
        tile_pipeline);
}

const dp1v2::StageStatus *findStageStatus(
    const dp1v2::FrameContextSnapshot &frame,
    const std::string_view stage_key)
{
    const auto status = std::find_if(
        frame.stage_statuses.begin(),
        frame.stage_statuses.end(),
        [stage_key](const dp1v2::StageStatus &record) {
            return record.stage_key == stage_key;
        });
    return status == frame.stage_statuses.end() ? nullptr : &(*status);
}

const dp1v2::StageTiming *findStageTiming(
    const dp1v2::FrameContextSnapshot &frame,
    const std::string_view stage_key)
{
    const auto timing = std::find_if(
        frame.profiling.stage_timings.begin(),
        frame.profiling.stage_timings.end(),
        [stage_key](const dp1v2::StageTiming &record) {
            return record.stage_key == stage_key;
        });
    return timing == frame.profiling.stage_timings.end() ? nullptr : &(*timing);
}

bool hasArtifact(
    const dp1v2::FrameContextSnapshot &frame,
    const std::string_view id)
{
    return std::any_of(
        frame.artifacts.records.begin(),
        frame.artifacts.records.end(),
        [id](const dp1v2::FrameArtifactRef &artifact) {
            return artifact.id == id;
        });
}

bool hasDiagnostic(
    const dp1v2::FrameContextSnapshot &frame,
    const std::string_view code)
{
    return std::any_of(
        frame.diagnostics.begin(),
        frame.diagnostics.end(),
        [code](const dp1v2::DiagnosticMessage &diagnostic) {
            return diagnostic.code == code;
        });
}

} // namespace

TEST(PipelineTest, TilesPrepRouteRunsRadiometricWarmUpThroughTilePipeline)
{
    const dp1v2::PipelineConfig pipeline_config = makeTilesPipelineConfig();
    const dp1v2::ResolvedPipelineConfig resolved_pipeline_config = makeResolvedPipelineConfig();
    dp1v2::InputNormalizationStage input_normalization_stage(resolved_pipeline_config.input_normalization);
    dp1v2::PrepStage prep_stage(makePrepResolvedConfig());
    dp1v2::RadiometricStage radiometric_stage(resolved_pipeline_config.radiometric);
    dp1v2::VisualizationSink visualization_sink(dp1v2::VisualizationConfig{});

    const dp1v2::SingleFramePipelineResult result = processTestFrame(
        makeEnvelope(),
        pipeline_config,
        resolved_pipeline_config,
        input_normalization_stage,
        prep_stage,
        radiometric_stage,
        visualization_sink);

    EXPECT_EQ(result.lifecycle.status, dp1v2::FrameTerminalStatus::Completed);
    EXPECT_EQ(result.lifecycle.reason, "test_result_sink_stub");

    const dp1v2::StageStatus *prep_status = findStageStatus(result.frame, "prep");
    ASSERT_NE(prep_status, nullptr);
    EXPECT_EQ(prep_status->status, dp1v2::StageStatusCode::Completed);
    EXPECT_EQ(prep_status->variant, "tiles");
    EXPECT_EQ(prep_status->level, "L1");
    EXPECT_EQ(prep_status->route, "tiles");

    EXPECT_EQ(result.frame.profiling.cardinality.tile_count, 6U);
    EXPECT_GT(result.frame.profiling.frame_duration_ns, 0);
    EXPECT_NE(findStageTiming(result.frame, "prep"), nullptr);

    const dp1v2::StageStatus *radiometric_status =
        findStageStatus(result.frame, "radiometric_correction");
    ASSERT_NE(radiometric_status, nullptr);
    EXPECT_EQ(radiometric_status->status, dp1v2::StageStatusCode::Skipped);
    EXPECT_EQ(radiometric_status->variant, "inverse_median");
    EXPECT_EQ(radiometric_status->route, "tiles");
    EXPECT_NE(radiometric_status->reason.find("total_tiles=6"), std::string::npos);
    EXPECT_NE(radiometric_status->reason.find("skipped_tiles=6"), std::string::npos);
    EXPECT_NE(radiometric_status->reason.find("unsupported_tiles=0"), std::string::npos);
    EXPECT_NE(radiometric_status->reason.find("measurements=0"), std::string::npos);

    EXPECT_NE(findStageTiming(result.frame, "radiometric_correction"), nullptr);
    EXPECT_FALSE(hasArtifact(result.frame, "radiometric.processing_frame"));
    EXPECT_FALSE(hasDiagnostic(result.frame, "prep.tiles.downstream_not_connected"));
    EXPECT_TRUE(hasDiagnostic(result.frame, "tile_pipeline.summary"));
}


TEST(PipelineTest, TilesPrepRouteReachesRadiometricCompletedAfterWarmUp)
{
    const dp1v2::PipelineConfig pipeline_config = makeTilesPipelineConfig();
    const dp1v2::ResolvedPipelineConfig resolved_pipeline_config = makeResolvedPipelineConfig();
    dp1v2::InputNormalizationStage input_normalization_stage(resolved_pipeline_config.input_normalization);
    dp1v2::PrepStage prep_stage(makePrepResolvedConfig());
    dp1v2::RadiometricStage radiometric_stage(resolved_pipeline_config.radiometric);
    dp1v2::VisualizationSink visualization_sink(dp1v2::VisualizationConfig{});

    dp1v2::SingleFramePipelineResult result{};
    for (int frame_index = 0; frame_index < 4; ++frame_index) {
        dp1v2::RawFrameEnvelope envelope = makeEnvelope();
        envelope.header_hint.frame_id = 42 + frame_index;
        result = processTestFrame(
            envelope,
            pipeline_config,
            resolved_pipeline_config,
            input_normalization_stage,
            prep_stage,
            radiometric_stage,
            visualization_sink);
    }

    EXPECT_EQ(result.lifecycle.status, dp1v2::FrameTerminalStatus::Completed);
    EXPECT_EQ(result.lifecycle.reason, "test_result_sink_stub");

    const dp1v2::StageStatus *prep_status = findStageStatus(result.frame, "prep");
    ASSERT_NE(prep_status, nullptr);
    EXPECT_EQ(prep_status->status, dp1v2::StageStatusCode::Completed);
    EXPECT_EQ(prep_status->route, "tiles");

    const dp1v2::StageStatus *radiometric_status =
        findStageStatus(result.frame, "radiometric_correction");
    ASSERT_NE(radiometric_status, nullptr);
    EXPECT_EQ(radiometric_status->status, dp1v2::StageStatusCode::Completed);
    EXPECT_EQ(radiometric_status->variant, "inverse_median");
    EXPECT_EQ(radiometric_status->route, "tiles");

    const dp1v2::StageTiming *radiometric_timing =
        findStageTiming(result.frame, "radiometric_correction");
    ASSERT_NE(radiometric_timing, nullptr);
    EXPECT_EQ(radiometric_timing->status, dp1v2::StageStatusCode::Completed);
    EXPECT_EQ(radiometric_timing->input_format, dp1v2::PixelFormat::U16);
    EXPECT_EQ(radiometric_timing->output_format, dp1v2::PixelFormat::S32);

    EXPECT_EQ(result.frame.profiling.cardinality.tile_count, 6U);
    EXPECT_NE(radiometric_status->reason.find("total_tiles=6"), std::string::npos);
    EXPECT_EQ(radiometric_status->reason.find("tile_route_controlled_failure"), std::string::npos);
}

TEST(PipelineTest, TilesRouteFallsBackToCamIndexWhenEnvelopeCameraIdMissing)
{
    const dp1v2::PipelineConfig pipeline_config = makeTilesPipelineConfig();
    const dp1v2::ResolvedPipelineConfig resolved_pipeline_config = makeResolvedPipelineConfig();
    dp1v2::InputNormalizationStage input_normalization_stage(resolved_pipeline_config.input_normalization);
    dp1v2::PrepStage prep_stage(makePrepResolvedConfig());
    dp1v2::RadiometricStage radiometric_stage(resolved_pipeline_config.radiometric);
    dp1v2::VisualizationSink visualization_sink(dp1v2::VisualizationConfig{});

    dp1v2::RawFrameEnvelope envelope = makeEnvelope();
    envelope.header_hint.camera_id.reset();

    const dp1v2::SingleFramePipelineResult result = processTestFrame(
        envelope,
        pipeline_config,
        resolved_pipeline_config,
        input_normalization_stage,
        prep_stage,
        radiometric_stage,
        visualization_sink,
        7);

    EXPECT_EQ(result.lifecycle.status, dp1v2::FrameTerminalStatus::Completed);
    EXPECT_EQ(result.frame.camera_id, 7);
}

TEST(PipelineTest, TilesRoutePreservesSourceCameraIdWhenRuntimeCamIndexDiffers)
{
    const dp1v2::PipelineConfig pipeline_config = makeTilesPipelineConfig();
    const dp1v2::ResolvedPipelineConfig resolved_pipeline_config = makeResolvedPipelineConfig();
    dp1v2::InputNormalizationStage input_normalization_stage(resolved_pipeline_config.input_normalization);
    dp1v2::PrepStage prep_stage(makePrepResolvedConfig());
    dp1v2::RadiometricStage radiometric_stage(resolved_pipeline_config.radiometric);
    dp1v2::VisualizationSink visualization_sink(dp1v2::VisualizationConfig{});

    dp1v2::RawFrameEnvelope envelope = makeEnvelope();
    envelope.header_hint.camera_id = 11;

    const dp1v2::SingleFramePipelineResult result = processTestFrame(
        envelope,
        pipeline_config,
        resolved_pipeline_config,
        input_normalization_stage,
        prep_stage,
        radiometric_stage,
        visualization_sink,
        7);

    EXPECT_EQ(result.lifecycle.status, dp1v2::FrameTerminalStatus::Completed);
    EXPECT_EQ(result.frame.camera_id, 11);
}

TEST(PipelineTest, TileRouteAppliesOpenCvThreadLimitAtRuntimeBoundary)
{
    const int previous_threads = cv::getNumThreads();

    dp1v2::ResolvedPipelineConfig resolved_pipeline_config = makeResolvedPipelineConfig();
    ASSERT_TRUE(resolved_pipeline_config.prep.tiles.has_value());
    resolved_pipeline_config.prep.tiles->execution.opencv_num_threads = 1;

    dp1v2::apply_tile_route_opencv_thread_limit(resolved_pipeline_config);
    EXPECT_EQ(cv::getNumThreads(), 1);

    cv::setNumThreads(previous_threads);
}

TEST(PipelineTest, TileRouteOpenCvThreadLimitZeroLeavesCurrentSetting)
{
    const int previous_threads = cv::getNumThreads();
    cv::setNumThreads(2);

    dp1v2::ResolvedPipelineConfig resolved_pipeline_config = makeResolvedPipelineConfig();
    ASSERT_TRUE(resolved_pipeline_config.prep.tiles.has_value());
    resolved_pipeline_config.prep.tiles->execution.opencv_num_threads = 0;

    dp1v2::apply_tile_route_opencv_thread_limit(resolved_pipeline_config);
    EXPECT_EQ(cv::getNumThreads(), 2);

    cv::setNumThreads(previous_threads);
}


TEST(PipelineTest, FullFrameRouteUsesInputRouteForFramePacketMetadata)
{
    auto pipeline_config = makeFullFramePipelineConfig();
    pipeline_config.input_route = dp1v2::InputRouteConfig{
        .pixel_format = dp1v2::PixelFormat::U16,
        .bit_depth = dp1v2::InputBitDepth::Bit12,
        .pixel_range = dp1v2::PixelRange{
            .min_value = 0.0,
            .max_value = 4095.0,
            .black_level = 0.0,
            .saturation_level = 4095.0,
        },
    };
    pipeline_config.stages.input_normalization = stageConfig(true, "passthrough", "L0");
    pipeline_config.stages.prep = stageConfig(true, "full_frame", "L0");
    pipeline_config.stages.radiometric = stageConfig(true, "inverse_median", "L0");

    const dp1v2::ResolvedPipelineConfig resolved_pipeline_config = makeResolvedPipelineConfig();
    dp1v2::InputNormalizationStage input_normalization_stage(resolved_pipeline_config.input_normalization);
    dp1v2::PrepStage prep_stage;
    dp1v2::RadiometricStage radiometric_stage(resolved_pipeline_config.radiometric);
    dp1v2::VisualizationSink visualization_sink(dp1v2::VisualizationConfig{});

    const auto result = processTestFrame(
        makeEnvelope(),
        pipeline_config,
        resolved_pipeline_config,
        input_normalization_stage,
        prep_stage,
        radiometric_stage,
        visualization_sink);

    EXPECT_EQ(result.lifecycle.status, dp1v2::FrameTerminalStatus::Completed);

    const dp1v2::StageStatus* input_normalization_status =
        findStageStatus(result.frame, "input_normalization");
    ASSERT_NE(input_normalization_status, nullptr);
    EXPECT_EQ(input_normalization_status->status, dp1v2::StageStatusCode::Completed);

    const dp1v2::StageTiming* input_normalization_timing =
        findStageTiming(result.frame, "input_normalization");
    ASSERT_NE(input_normalization_timing, nullptr);

    EXPECT_EQ(result.lifecycle.reason.find("input_route bit_depth mismatch"), std::string::npos);
    EXPECT_EQ(result.lifecycle.reason.find("input_route pixel_range mismatch"), std::string::npos);
}

TEST(PipelineTest, FullFrameRouteRecordsProfilingCollectionWhenReportsDisabled)
{
    dp1v2::LoggingConfig logging{};
    dp1v2::ProfilingConfig profiling{};
    logging.enabled = true;
    profiling.emit_reports = false;
    profiling.reports.emit_frame_reports = true;

    const dp1v2::PipelineConfig pipeline_config = makeFullFramePipelineConfig();
    const dp1v2::ResolvedPipelineConfig resolved_pipeline_config = makeResolvedPipelineConfig();
    dp1v2::InputNormalizationStage input_normalization_stage(resolved_pipeline_config.input_normalization);
    dp1v2::PrepStage prep_stage;
    dp1v2::RadiometricStage radiometric_stage(resolved_pipeline_config.radiometric);
    dp1v2::VisualizationSink visualization_sink(dp1v2::VisualizationConfig{});

    const dp1v2::SingleFramePipelineResult result = processTestFrame(
        makeEnvelope(),
        pipeline_config,
        resolved_pipeline_config,
        input_normalization_stage,
        prep_stage,
        radiometric_stage,
        visualization_sink);

    EXPECT_FALSE(dp1v2::should_emit_frame_profile_log(logging, profiling, true));
    EXPECT_EQ(result.lifecycle.status, dp1v2::FrameTerminalStatus::Completed);
    EXPECT_GT(result.frame.profiling.frame_duration_ns, 0);
    EXPECT_NE(findStageTiming(result.frame, "input"), nullptr);
    EXPECT_NE(findStageTiming(result.frame, "input_normalization"), nullptr);
    EXPECT_NE(findStageTiming(result.frame, "prep"), nullptr);
    EXPECT_NE(findStageTiming(result.frame, "radiometric_correction"), nullptr);

    dp1v2::RuntimeProfilingAggregator aggregator;
    aggregator.record_frame(result);
    EXPECT_EQ(aggregator.run_summary().frames_total, 1U);
    EXPECT_EQ(aggregator.run_summary().frame_duration_samples, 1U);
    EXPECT_EQ(aggregator.run_summary().stages.size(), 4U);
}
