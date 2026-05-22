#include <gtest/gtest.h>

#include <string>

#include "dp1v2/runtime/runtime_profiling_aggregator.hpp"

namespace {

dp1v2::StageTiming stageTiming(const std::string &stage_key, const std::int64_t duration_ns)
{
    return dp1v2::StageTiming{
        .stage_key = stage_key,
        .status = dp1v2::StageStatusCode::Completed,
        .duration_ns = duration_ns,
    };
}

dp1v2::StageStatus stageStatus(
    const std::string &stage_key,
    const dp1v2::StageStatusCode status,
    const std::string &variant,
    const std::string &route)
{
    return dp1v2::StageStatus{
        .stage_key = stage_key,
        .status = status,
        .variant = variant,
        .route = route,
    };
}

dp1v2::SingleFramePipelineResult makeResult(
    const dp1v2::FrameTerminalStatus status,
    const std::int64_t frame_duration_ns,
    const std::uint32_t tile_count)
{
    dp1v2::SingleFramePipelineResult result{};
    result.lifecycle.status = status;
    result.frame.frame_id = 42;
    result.frame.camera_id = 7;
    result.frame.profiling.frame_duration_ns = frame_duration_ns;
    result.frame.profiling.cardinality.tile_count = tile_count;
    result.frame.profiling.stage_timings = {
        stageTiming("input", 100000),
        stageTiming("input_normalization", 200000),
        stageTiming("prep", 300000),
    };
    return result;
}

dp1v2::SingleFramePipelineResult makeTilesControlledFailureResult()
{
    dp1v2::SingleFramePipelineResult result{};
    result.lifecycle.status = dp1v2::FrameTerminalStatus::Failed;
    result.frame.frame_id = 42;
    result.frame.camera_id = 7;
    result.frame.profiling.frame_duration_ns = 381000;
    result.frame.profiling.cardinality.tile_count = 6;
    result.frame.profiling.stage_timings = {
        stageTiming("input", 100000),
        stageTiming("input_normalization", 200000),
        stageTiming("prep", 81000),
    };
    result.frame.stage_statuses = {
        stageStatus("prep", dp1v2::StageStatusCode::Completed, "tiles", "tiles"),
        stageStatus(
            "radiometric_correction",
            dp1v2::StageStatusCode::NotStarted,
            "inverse_median",
            "tiles"),
    };
    result.frame.diagnostics.push_back(dp1v2::DiagnosticMessage{
        .code = "prep.tiles.downstream_not_connected",
        .message = "not connected",
    });
    return result;
}

dp1v2::SingleFramePipelineResult makeFailureResultWithReason(const std::string &reason)
{
    dp1v2::SingleFramePipelineResult result{};
    result.lifecycle.status = dp1v2::FrameTerminalStatus::Failed;
    result.lifecycle.reason = reason;
    result.frame.frame_id = 7;
    result.frame.camera_id = 2;
    result.frame.profiling.cardinality.tile_count = 1;
    return result;
}

} // namespace

TEST(RuntimeProfilingAggregatorTest, AccumulatesRunAndWindowSummaries)
{
    dp1v2::RuntimeProfilingAggregator aggregator;
    aggregator.record_frame(makeResult(dp1v2::FrameTerminalStatus::Completed, 5000000, 0));
    aggregator.record_frame(makeResult(dp1v2::FrameTerminalStatus::Failed, 7000000, 6));
    aggregator.record_dropped_frame();

    const dp1v2::RuntimeProfilingSummary &summary = aggregator.run_summary();
    EXPECT_EQ(summary.frames_total, 3U);
    EXPECT_EQ(summary.completed, 1U);
    EXPECT_EQ(summary.failed, 1U);
    EXPECT_EQ(summary.dropped, 1U);
    EXPECT_EQ(summary.frame_duration_samples, 2U);
    EXPECT_EQ(summary.min_frame_duration_ns, 5000000);
    EXPECT_EQ(summary.max_frame_duration_ns, 7000000);
    EXPECT_EQ(summary.total_frame_duration_ns, 12000000);
    EXPECT_EQ(summary.tile_count_samples, 2U);
    EXPECT_EQ(summary.total_tile_count, 6U);
    EXPECT_EQ(summary.max_tile_count, 6U);

    ASSERT_EQ(summary.stages.size(), 3U);
    EXPECT_EQ(summary.stages[0].stage_key, "input");
    EXPECT_EQ(summary.stages[0].calls, 2U);
    EXPECT_EQ(summary.stages[0].total_duration_ns, 200000);

    const dp1v2::RuntimeProfilingSummary window = aggregator.consume_window_summary();
    EXPECT_EQ(window.frames_total, 3U);
    EXPECT_EQ(aggregator.window_summary().frames_total, 0U);
}

TEST(RuntimeProfilingAggregatorTest, FormatsWindowSummaryWithoutPercentiles)
{
    dp1v2::RuntimeProfilingAggregator aggregator;
    aggregator.record_frame(makeResult(dp1v2::FrameTerminalStatus::Completed, 5000000, 0));
    aggregator.record_frame(makeResult(dp1v2::FrameTerminalStatus::Completed, 7000000, 0));

    const std::string message = dp1v2::format_profiling_window_summary_log(aggregator.run_summary());
    EXPECT_NE(message.find("event=profiling_window_summary"), std::string::npos);
    EXPECT_NE(message.find("frames=2"), std::string::npos);
    EXPECT_NE(message.find("avg_frame_duration_ms=6.000"), std::string::npos);
    EXPECT_NE(message.find("min_frame_duration_ms=5.000"), std::string::npos);
    EXPECT_NE(message.find("max_frame_duration_ms=7.000"), std::string::npos);
    EXPECT_NE(message.find("stage_input_avg_ms=0.100"), std::string::npos);
    EXPECT_EQ(message.find("p95"), std::string::npos);
    EXPECT_EQ(message.find("p99"), std::string::npos);
    EXPECT_EQ(message.find("median"), std::string::npos);
}

TEST(RuntimeProfilingAggregatorTest, AverageFrameDurationIgnoresDroppedFrames)
{
    dp1v2::RuntimeProfilingAggregator aggregator;
    aggregator.record_frame(makeResult(dp1v2::FrameTerminalStatus::Completed, 5000000, 0));
    aggregator.record_frame(makeResult(dp1v2::FrameTerminalStatus::Failed, 7000000, 0));
    aggregator.record_dropped_frame();

    const std::string message = dp1v2::format_profiling_window_summary_log(aggregator.run_summary());
    EXPECT_NE(message.find("frames=3"), std::string::npos);
    EXPECT_NE(message.find("avg_frame_duration_ms=6.000"), std::string::npos);
}

TEST(RuntimeProfilingAggregatorTest, OnlyDroppedFramesYieldZeroDurations)
{
    dp1v2::RuntimeProfilingAggregator aggregator;
    aggregator.record_dropped_frame();
    aggregator.record_dropped_frame();

    const std::string message = dp1v2::format_profiling_window_summary_log(aggregator.run_summary());
    EXPECT_NE(message.find("frames=2"), std::string::npos);
    EXPECT_NE(message.find("avg_frame_duration_ms=0.000"), std::string::npos);
    EXPECT_NE(message.find("min_frame_duration_ms=0.000"), std::string::npos);
    EXPECT_NE(message.find("max_frame_duration_ms=0.000"), std::string::npos);
}

TEST(RuntimeProfilingAggregatorTest, FormatsRunSummaryWithSourceCounters)
{
    dp1v2::RuntimeProfilingAggregator aggregator;
    aggregator.record_frame(makeResult(dp1v2::FrameTerminalStatus::Completed, 5000000, 0));

    const dp1v2::RuntimeProfilingRunSummary summary{
        .profile = aggregator.run_summary(),
        .source_read_attempts = 2,
        .source_empty_reads = 1,
        .last_status = dp1v2::ProcessTerminalStatus::SourceExhausted,
        .last_reason = "source exhausted=bad",
    };

    const std::string message = dp1v2::format_profiling_run_summary_log(summary);
    EXPECT_NE(message.find("event=profiling_run_summary"), std::string::npos);
    EXPECT_NE(message.find("frames_total=1"), std::string::npos);
    EXPECT_NE(message.find("source_read_attempts=2"), std::string::npos);
    EXPECT_NE(message.find("source_empty_reads=1"), std::string::npos);
    EXPECT_NE(message.find("last_status=source_exhausted"), std::string::npos);
    EXPECT_NE(message.find("last_reason=source_exhausted_bad"), std::string::npos);
}

TEST(RuntimeProfilingAggregatorTest, GatingPreventsFrameFormatterWhenFrameReportsDisabled)
{
    dp1v2::LoggingConfig logging{};
    dp1v2::ProfilingConfig profiling{};
    logging.enabled = true;
    profiling.enabled = true;
    profiling.reports.emit_frame_reports = false;

    EXPECT_FALSE(dp1v2::should_emit_frame_profile_log(logging, profiling, true));

    profiling.reports.emit_frame_reports = true;
    EXPECT_FALSE(dp1v2::should_emit_frame_profile_log(logging, profiling, false));
    EXPECT_TRUE(dp1v2::should_emit_frame_profile_log(logging, profiling, true));
}

TEST(RuntimeProfilingAggregatorTest, GatingControlsWindowSummaryLogging)
{
    dp1v2::LoggingConfig logging{};
    dp1v2::ProfilingConfig profiling{};
    logging.enabled = true;
    profiling.enabled = true;
    profiling.reports.emit_window_summary = true;
    profiling.logging_bridge.emit_aggregated_summaries = true;
    profiling.logging_bridge.summary_every_n_frames = 0;

    EXPECT_TRUE(dp1v2::should_emit_window_profile_log(logging, profiling));

    profiling.reports.emit_window_summary = false;
    EXPECT_FALSE(dp1v2::should_emit_window_profile_log(logging, profiling));

    profiling.reports.emit_window_summary = true;
    profiling.logging_bridge.emit_aggregated_summaries = false;
    EXPECT_FALSE(dp1v2::should_emit_window_profile_log(logging, profiling));
}

TEST(RuntimeProfilingAggregatorTest, WindowSummaryDueUsesAggregationWindowFrames)
{
    dp1v2::ProfilingConfig profiling{};
    dp1v2::RuntimeProfilingSummary summary{};

    profiling.aggregation_window_frames = 3;
    summary.frames_total = 2;
    EXPECT_FALSE(dp1v2::is_window_summary_due(profiling, summary));

    summary.frames_total = 3;
    EXPECT_TRUE(dp1v2::is_window_summary_due(profiling, summary));
}

TEST(RuntimeProfilingAggregatorTest, WindowSummaryDueWithSingleFrameWindow)
{
    dp1v2::ProfilingConfig profiling{};
    dp1v2::RuntimeProfilingSummary summary{};

    profiling.aggregation_window_frames = 1;
    summary.frames_total = 1;
    EXPECT_TRUE(dp1v2::is_window_summary_due(profiling, summary));
}

TEST(RuntimeProfilingAggregatorTest, WindowSummaryDueIgnoresSummaryEveryNFrames)
{
    dp1v2::ProfilingConfig profiling{};
    dp1v2::RuntimeProfilingSummary summary{};

    profiling.aggregation_window_frames = 1;
    profiling.logging_bridge.summary_every_n_frames = 1000;
    summary.frames_total = 1;

    EXPECT_TRUE(dp1v2::is_window_summary_due(profiling, summary));
}

TEST(RuntimeProfilingAggregatorTest, FormatsControlledTilesFailureSummary)
{
    const dp1v2::SingleFramePipelineResult result = makeTilesControlledFailureResult();

    EXPECT_TRUE(dp1v2::is_controlled_tiles_frame_failure(result));
    const std::string message = dp1v2::format_frame_failed_log(result);
    EXPECT_NE(message.find("event=frame_failed"), std::string::npos);
    EXPECT_NE(message.find("frame_id=42"), std::string::npos);
    EXPECT_NE(message.find("camera_id=7"), std::string::npos);
    EXPECT_NE(message.find("controlled=true"), std::string::npos);
    EXPECT_NE(message.find("reason=prep_tiles_downstream_not_connected"), std::string::npos);
    EXPECT_NE(message.find("prep_variant=tiles"), std::string::npos);
    EXPECT_NE(message.find("tile_count=6"), std::string::npos);
    EXPECT_NE(message.find("radiometric_status=not_started"), std::string::npos);
}

TEST(RuntimeProfilingAggregatorTest, FormatsFailureSummaryWithSanitizedReason)
{
    const std::string message = dp1v2::format_frame_failed_log(
        makeFailureResultWithReason("bad reason=oops"));

    EXPECT_NE(message.find("reason=bad_reason_oops"), std::string::npos);
}
