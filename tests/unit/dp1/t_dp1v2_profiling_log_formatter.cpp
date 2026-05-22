#include <gtest/gtest.h>

#include <string>

#include "dp1v2/runtime/profiling_log_formatter.hpp"

namespace {

bool contains(const std::string &text, const std::string &needle)
{
    return text.find(needle) != std::string::npos;
}

dp1v2::StageTiming stageTiming(
    const std::string &stage_key,
    const dp1v2::StageStatusCode status,
    const std::int64_t duration_ns,
    const std::string &variant,
    const std::string &level,
    const dp1v2::PixelFormat input_format,
    const dp1v2::PixelFormat output_format)
{
    return dp1v2::StageTiming{
        .stage_key = stage_key,
        .status = status,
        .variant = variant,
        .level = level,
        .input_format = input_format,
        .output_format = output_format,
        .duration_ns = duration_ns,
    };
}

dp1v2::StageStatus stageStatus(
    const std::string &stage_key,
    const dp1v2::StageStatusCode status,
    const std::string &variant,
    const std::string &level,
    const std::string &route,
    const std::string &reason = {})
{
    return dp1v2::StageStatus{
        .stage_key = stage_key,
        .status = status,
        .variant = variant,
        .level = level,
        .route = route,
        .reason = reason,
    };
}

dp1v2::SingleFramePipelineResult makeFullFrameResult()
{
    dp1v2::SingleFramePipelineResult result{};
    result.lifecycle.status = dp1v2::FrameTerminalStatus::Completed;
    result.frame.frame_id = 123;
    result.frame.camera_id = 0;
    result.frame.profiling.frame_duration_ns = 5842000;
    result.frame.profiling.stage_timings = {
        stageTiming("input",
                    dp1v2::StageStatusCode::Completed,
                    18000,
                    {},
                    {},
                    dp1v2::PixelFormat::U8,
                    dp1v2::PixelFormat::U8),
        stageTiming("input_normalization",
                    dp1v2::StageStatusCode::Completed,
                    71000,
                    "passthrough",
                    "L0",
                    dp1v2::PixelFormat::U8,
                    dp1v2::PixelFormat::U8),
        stageTiming("prep",
                    dp1v2::StageStatusCode::Completed,
                    9000,
                    "full_frame",
                    "L0",
                    dp1v2::PixelFormat::U8,
                    dp1v2::PixelFormat::U8),
        stageTiming("radiometric_correction",
                    dp1v2::StageStatusCode::Completed,
                    5521000,
                    "inverse_median",
                    "L1",
                    dp1v2::PixelFormat::U8,
                    dp1v2::PixelFormat::S16),
    };
    return result;
}

dp1v2::SingleFramePipelineResult makeTilesControlledFailureResult()
{
    dp1v2::SingleFramePipelineResult result{};
    result.lifecycle.status = dp1v2::FrameTerminalStatus::Failed;
    result.lifecycle.reason = "prep tiles layout is built, but downstream tile pipeline is not connected yet";
    result.frame.frame_id = 42;
    result.frame.camera_id = 7;
    result.frame.profiling.frame_duration_ns = 381000;
    result.frame.profiling.cardinality.tile_count = 6;
    result.frame.profiling.stage_timings = {
        stageTiming("input",
                    dp1v2::StageStatusCode::Completed,
                    11000,
                    {},
                    {},
                    dp1v2::PixelFormat::U16,
                    dp1v2::PixelFormat::U16),
        stageTiming("input_normalization",
                    dp1v2::StageStatusCode::Completed,
                    44000,
                    "passthrough",
                    "L0",
                    dp1v2::PixelFormat::U16,
                    dp1v2::PixelFormat::U16),
        stageTiming("prep",
                    dp1v2::StageStatusCode::Completed,
                    126000,
                    "tiles",
                    "L1",
                    dp1v2::PixelFormat::U16,
                    dp1v2::PixelFormat::U16),
    };
    result.frame.stage_statuses = {
        stageStatus("prep",
                    dp1v2::StageStatusCode::Completed,
                    "tiles",
                    "L1",
                    "tiles"),
        stageStatus("radiometric_correction",
                    dp1v2::StageStatusCode::NotStarted,
                    "inverse_median",
                    "L0",
                    "tiles",
                    result.lifecycle.reason),
    };
    result.frame.diagnostics = {
        dp1v2::DiagnosticMessage{
            .code = "prep.tiles.downstream_not_connected",
            .message = result.lifecycle.reason,
        },
    };
    return result;
}

} // namespace

TEST(ProfilingLogFormatterTest, FullFrameSummaryContainsFrameProfileAndStages)
{
    const std::string message = dp1v2::format_frame_profile_log(makeFullFrameResult());

    EXPECT_TRUE(contains(message, "event=frame_profile"));
    EXPECT_TRUE(contains(message, "frame_id=123"));
    EXPECT_TRUE(contains(message, "camera_id=0"));
    EXPECT_TRUE(contains(message, "status=completed"));
    EXPECT_TRUE(contains(message, "total_duration_ms=5.842"));
    EXPECT_TRUE(contains(message, "stage_count=4"));
    EXPECT_TRUE(contains(message, "stages=\""));
    EXPECT_TRUE(contains(message, "radiometric_correction"));
}

TEST(ProfilingLogFormatterTest, TilesSummaryContainsControlledFailureAndRoute)
{
    const std::string message = dp1v2::format_frame_profile_log(makeTilesControlledFailureResult());

    EXPECT_TRUE(contains(message, "event=frame_profile"));
    EXPECT_TRUE(contains(message, "status=failed"));
    EXPECT_TRUE(contains(message, "reason=prep_tiles_downstream_not_connected"));
    EXPECT_TRUE(contains(message, "controlled=true"));
    EXPECT_TRUE(contains(message, "tile_count=6"));
    EXPECT_TRUE(contains(message, "stage_count=3"));
    EXPECT_TRUE(contains(message, "radiometric_correction:not_started:tiles"));
}

TEST(ProfilingLogFormatterTest, EmptyStageTimingsDoNotCrash)
{
    dp1v2::SingleFramePipelineResult result{};
    result.lifecycle.status = dp1v2::FrameTerminalStatus::Completed;
    result.frame.frame_id = 9;
    result.frame.camera_id = 3;
    result.frame.profiling.frame_duration_ns = 1000;

    const std::string message = dp1v2::format_frame_profile_log(result);

    EXPECT_TRUE(contains(message, "event=frame_profile"));
    EXPECT_TRUE(contains(message, "frame_id=9"));
    EXPECT_TRUE(contains(message, "camera_id=3"));
    EXPECT_TRUE(contains(message, "stage_count=0"));
    EXPECT_TRUE(contains(message, "stages=\"\""));
}
