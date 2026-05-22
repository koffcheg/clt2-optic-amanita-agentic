#include <gtest/gtest.h>

#include <algorithm>
#include <chrono>
#include <string>
#include <string_view>

#include <opencv2/core.hpp>

#include "dp1v2/frame/frame_context.hpp"
#include "dp1v2/runtime/pipeline.hpp"

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

constexpr std::string_view kTilesDownstreamNotConnectedReason =
    "prep tiles layout is built, but downstream tile pipeline is not connected yet";

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

TEST(PipelineTest, TilesPrepRouteBuildsLayoutThenStopsBeforeRadiometric)
{
    const dp1v2::PipelineConfig pipeline_config = makeTilesPipelineConfig();
    const dp1v2::InputNormalizationStage input_normalization_stage;
    dp1v2::PrepStage prep_stage(makePrepResolvedConfig());
    dp1v2::RadiometricStage radiometric_stage;
    dp1v2::VisualizationSink visualization_sink(dp1v2::VisualizationConfig{});

    const dp1v2::SingleFramePipelineResult result = dp1v2::process_single_frame(
        makeEnvelope(),
        7,
        pipeline_config,
        input_normalization_stage,
        prep_stage,
        radiometric_stage,
        visualization_sink);

    EXPECT_EQ(result.lifecycle.status, dp1v2::FrameTerminalStatus::Failed);
    EXPECT_EQ(result.lifecycle.reason, kTilesDownstreamNotConnectedReason);
    EXPECT_NE(result.sink.send_status, dp1v2::ResultSinkStatus::Accepted);
    EXPECT_NE(result.sink.artifact_status, dp1v2::ResultSinkStatus::Accepted);

    const dp1v2::StageStatus *prep_status = findStageStatus(result.frame, "prep");
    ASSERT_NE(prep_status, nullptr);
    EXPECT_EQ(prep_status->status, dp1v2::StageStatusCode::Completed);
    EXPECT_EQ(prep_status->variant, "tiles");
    EXPECT_EQ(prep_status->level, "L1");
    EXPECT_EQ(prep_status->route, "tiles");

    EXPECT_GT(result.frame.profiling.cardinality.tile_count, 0U);
    EXPECT_GT(result.frame.profiling.frame_duration_ns, 0);

    const dp1v2::StageStatus *radiometric_status =
        findStageStatus(result.frame, "radiometric_correction");
    ASSERT_NE(radiometric_status, nullptr);
    EXPECT_EQ(radiometric_status->status, dp1v2::StageStatusCode::NotStarted);
    EXPECT_EQ(radiometric_status->variant, "inverse_median");
    EXPECT_EQ(radiometric_status->route, "tiles");
    EXPECT_EQ(radiometric_status->reason, kTilesDownstreamNotConnectedReason);

    EXPECT_EQ(findStageTiming(result.frame, "radiometric_correction"), nullptr);
    EXPECT_FALSE(hasArtifact(result.frame, "radiometric.processing_frame"));
    EXPECT_TRUE(hasDiagnostic(result.frame, "prep.tiles.downstream_not_connected"));
}

TEST(PipelineTest, FullFrameRouteRecordsFrameDuration)
{
    const dp1v2::PipelineConfig pipeline_config = makeFullFramePipelineConfig();
    const dp1v2::InputNormalizationStage input_normalization_stage;
    dp1v2::PrepStage prep_stage;
    dp1v2::RadiometricStage radiometric_stage(makeRadiometricResolvedConfig());
    dp1v2::VisualizationSink visualization_sink(dp1v2::VisualizationConfig{});

    const dp1v2::SingleFramePipelineResult result = dp1v2::process_single_frame(
        makeEnvelope(),
        7,
        pipeline_config,
        input_normalization_stage,
        prep_stage,
        radiometric_stage,
        visualization_sink);

    EXPECT_EQ(result.lifecycle.status, dp1v2::FrameTerminalStatus::Completed);
    EXPECT_GT(result.frame.profiling.frame_duration_ns, 0);
    EXPECT_NE(findStageTiming(result.frame, "input"), nullptr);
    EXPECT_NE(findStageTiming(result.frame, "input_normalization"), nullptr);
    EXPECT_NE(findStageTiming(result.frame, "prep"), nullptr);
    EXPECT_NE(findStageTiming(result.frame, "radiometric_correction"), nullptr);
}
