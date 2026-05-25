#include <gtest/gtest.h>

#include <string>
#include <vector>

#include "dp1v2/runtime/tile_frame_aggregator.hpp"

namespace {

dp1v2::TileResult makeTileResult(const dp1v2::TileResultStatus status)
{
    dp1v2::TileResult result{};
    result.status = status;
    return result;
}

dp1v2::TileExecutionConfig executionConfig(
    const dp1v2::TileFrameStatusPolicy frame_status_policy)
{
    dp1v2::TileExecutionConfig config{};
    config.frame_status_policy = frame_status_policy;
    return config;
}

dp1v2::TileAggregationConfig measurementsOnlyConfig()
{
    dp1v2::TileAggregationConfig config{};
    config.mode = dp1v2::TileAggregationMode::MeasurementsOnly;
    return config;
}

bool contains(const std::string& value, const std::string& expected)
{
    return value.find(expected) != std::string::npos;
}

} // namespace

TEST(TileFrameAggregatorTest, AllCompletedTilesProduceCompletedSummary)
{
    const std::vector<dp1v2::TileResult> results{
        makeTileResult(dp1v2::TileResultStatus::Completed),
        makeTileResult(dp1v2::TileResultStatus::Completed),
        makeTileResult(dp1v2::TileResultStatus::Completed),
    };

    const dp1v2::TileFrameAggregationSummary summary =
        dp1v2::TileFrameAggregator{}.aggregate(
            results,
            measurementsOnlyConfig(),
            executionConfig(dp1v2::TileFrameStatusPolicy::FailedIfAnyRequiredTileFailed));

    EXPECT_EQ(summary.total_tiles, 3U);
    EXPECT_EQ(summary.completed_tile_count, 3U);
    EXPECT_EQ(summary.failed_tile_count, 0U);
    EXPECT_EQ(summary.unsupported_tile_count, 0U);
    EXPECT_EQ(summary.terminal_status, dp1v2::FrameTerminalStatus::Completed);
    EXPECT_TRUE(summary.reason.empty());
}

TEST(TileFrameAggregatorTest, AnyFailedTileFailsFrameUnderStrictPolicy)
{
    const std::vector<dp1v2::TileResult> results{
        makeTileResult(dp1v2::TileResultStatus::Completed),
        makeTileResult(dp1v2::TileResultStatus::Failed),
        makeTileResult(dp1v2::TileResultStatus::Completed),
    };

    const dp1v2::TileFrameAggregationSummary summary =
        dp1v2::TileFrameAggregator{}.aggregate(
            results,
            measurementsOnlyConfig(),
            executionConfig(dp1v2::TileFrameStatusPolicy::FailedIfAnyRequiredTileFailed));

    EXPECT_EQ(summary.total_tiles, 3U);
    EXPECT_EQ(summary.completed_tile_count, 2U);
    EXPECT_EQ(summary.failed_tile_count, 1U);
    EXPECT_EQ(summary.terminal_status, dp1v2::FrameTerminalStatus::Failed);
    EXPECT_TRUE(contains(summary.reason, "tile_frame_aggregation_failed"));
    EXPECT_TRUE(contains(summary.reason, "failed_tiles=1"));
}

TEST(TileFrameAggregatorTest, PartialFailurePolicyCompletesWithReasonWhenSomeTilesCompleted)
{
    const std::vector<dp1v2::TileResult> results{
        makeTileResult(dp1v2::TileResultStatus::Completed),
        makeTileResult(dp1v2::TileResultStatus::Failed),
    };

    const dp1v2::TileFrameAggregationSummary summary =
        dp1v2::TileFrameAggregator{}.aggregate(
            results,
            measurementsOnlyConfig(),
            executionConfig(dp1v2::TileFrameStatusPolicy::PartialIfSomeTilesFailed));

    EXPECT_EQ(summary.completed_tile_count, 1U);
    EXPECT_EQ(summary.failed_tile_count, 1U);
    EXPECT_EQ(summary.terminal_status, dp1v2::FrameTerminalStatus::Completed);
    EXPECT_TRUE(contains(summary.reason, "tile_frame_aggregation_partial"));
}

TEST(TileFrameAggregatorTest, UnsupportedTilesAreCountedAndFailFrame)
{
    const std::vector<dp1v2::TileResult> results{
        makeTileResult(dp1v2::TileResultStatus::Completed),
        makeTileResult(dp1v2::TileResultStatus::Unsupported),
        makeTileResult(dp1v2::TileResultStatus::Skipped),
        makeTileResult(dp1v2::TileResultStatus::Disabled),
    };

    const dp1v2::TileFrameAggregationSummary summary =
        dp1v2::TileFrameAggregator{}.aggregate(
            results,
            measurementsOnlyConfig(),
            executionConfig(dp1v2::TileFrameStatusPolicy::PartialIfSomeTilesFailed));

    EXPECT_EQ(summary.total_tiles, 4U);
    EXPECT_EQ(summary.completed_tile_count, 1U);
    EXPECT_EQ(summary.unsupported_tile_count, 1U);
    EXPECT_EQ(summary.skipped_tile_count, 1U);
    EXPECT_EQ(summary.disabled_tile_count, 1U);
    EXPECT_EQ(summary.terminal_status, dp1v2::FrameTerminalStatus::Failed);
    EXPECT_TRUE(contains(summary.reason, "tile_frame_aggregation_unsupported"));
}

TEST(TileFrameAggregatorTest, AggregatesStructuredOutputCounts)
{
    std::vector<dp1v2::TileResult> results{
        makeTileResult(dp1v2::TileResultStatus::Completed),
        makeTileResult(dp1v2::TileResultStatus::Completed),
    };
    results[0].candidates.resize(2);
    results[0].segments.resize(3);
    results[0].objects.resize(1);
    results[0].measurements.resize(4);
    results[1].candidates.resize(5);
    results[1].segments.resize(7);
    results[1].objects.resize(11);
    results[1].measurements.resize(13);

    const dp1v2::TileFrameAggregationSummary summary =
        dp1v2::TileFrameAggregator{}.aggregate(
            results,
            measurementsOnlyConfig(),
            executionConfig(dp1v2::TileFrameStatusPolicy::FailedIfAnyRequiredTileFailed));

    EXPECT_EQ(summary.candidate_count, 7U);
    EXPECT_EQ(summary.segment_count, 10U);
    EXPECT_EQ(summary.validated_object_count, 12U);
    EXPECT_EQ(summary.measurement_count, 17U);
    EXPECT_EQ(summary.terminal_status, dp1v2::FrameTerminalStatus::Completed);
}

TEST(TileFrameAggregatorTest, EmptyTileResultVectorProducesCompletedZeroSummary)
{
    const std::vector<dp1v2::TileResult> results;

    const dp1v2::TileFrameAggregationSummary summary =
        dp1v2::TileFrameAggregator{}.aggregate(
            results,
            measurementsOnlyConfig(),
            executionConfig(dp1v2::TileFrameStatusPolicy::FailedIfAnyRequiredTileFailed));

    EXPECT_EQ(summary.total_tiles, 0U);
    EXPECT_EQ(summary.completed_tile_count, 0U);
    EXPECT_EQ(summary.failed_tile_count, 0U);
    EXPECT_EQ(summary.candidate_count, 0U);
    EXPECT_EQ(summary.measurement_count, 0U);
    EXPECT_EQ(summary.terminal_status, dp1v2::FrameTerminalStatus::Completed);
    EXPECT_TRUE(summary.reason.empty());
}

TEST(TileFrameAggregatorTest, DebugMergeModeIsControlledUnsupported)
{
    const std::vector<dp1v2::TileResult> results{
        makeTileResult(dp1v2::TileResultStatus::Completed),
    };
    dp1v2::TileAggregationConfig aggregation_config = measurementsOnlyConfig();
    aggregation_config.mode = dp1v2::TileAggregationMode::MeasurementsAndDebugMerge;

    const dp1v2::TileFrameAggregationSummary summary =
        dp1v2::TileFrameAggregator{}.aggregate(
            results,
            aggregation_config,
            executionConfig(dp1v2::TileFrameStatusPolicy::FailedIfAnyRequiredTileFailed));

    EXPECT_EQ(summary.completed_tile_count, 1U);
    EXPECT_EQ(summary.terminal_status, dp1v2::FrameTerminalStatus::Failed);
    EXPECT_EQ(summary.reason, "tile debug merge is not implemented");
}
