#include <gtest/gtest.h>

#include <cstddef>
#include <stdexcept>
#include <vector>

#include <opencv2/core.hpp>

#include "dp1v2/config/config.hpp"
#include "dp1v2/domain/tile_desc.hpp"
#include "dp1v2/domain/tile_raw_view.hpp"
#include "dp1v2/runtime/tile_executor.hpp"
#include "dp1v2/stages/radiometric_stage.hpp"

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

dp1v2::StageConfig radiometricConfig()
{
    return dp1v2::StageConfig{
        .enabled = true,
        .variant = "unsupported_tile_test_variant",
        .level = "L0",
        .parameters = {},
    };
}

dp1v2::PipelineConfig pipelineConfig()
{
    dp1v2::PipelineConfig config{};
    config.stages.radiometric = radiometricConfig();
    return config;
}

struct TileFixture {
    cv::Mat image;
    std::vector<dp1v2::TileDesc> descs;
    std::vector<dp1v2::TileRawView> views;
    std::vector<dp1v2::TileTask> tasks;
};

TileFixture makeTileFixture(const std::size_t tile_count)
{
    TileFixture fixture{};
    fixture.image = cv::Mat(1, static_cast<int>(tile_count), CV_16UC1, cv::Scalar(1024));
    fixture.descs.resize(tile_count);
    fixture.views.resize(tile_count);
    fixture.tasks.resize(tile_count);

    for (std::size_t index = 0; index < tile_count; ++index) {
        const int tile_id = static_cast<int>(index);
        fixture.descs[index] = dp1v2::TileDesc{
            .frame_id = 42,
            .tile_id = tile_id,
            .roi_with_border = cv::Rect(tile_id, 0, 1, 1),
            .valid_area = cv::Rect(0, 0, 1, 1),
            .origin_in_frame = cv::Point(tile_id, 0),
        };
        fixture.views[index] = dp1v2::TileRawView{
            .frame_id = 42,
            .camera_id = 7,
            .tile_id = tile_id,
            .image = fixture.image(cv::Rect(tile_id, 0, 1, 1)),
            .pixel_format = dp1v2::PixelFormat::U16,
            .bit_depth = dp1v2::InputBitDepth::Bit16,
            .pixel_range = rangeU16(),
            .geometry = dp1v2::FrameGeometry{.width = 1, .height = 1},
            .origin_in_frame = cv::Point(tile_id, 0),
            .valid_area = cv::Rect(0, 0, 1, 1),
        };
        fixture.tasks[index] = dp1v2::TileTask{
            .task_index = index,
            .frame_id = 42,
            .camera_id = 7,
            .desc = &fixture.descs[index],
            .raw_view = &fixture.views[index],
        };
    }

    return fixture;
}

std::vector<dp1v2::TileResult> makeSentinelResults(const std::size_t count)
{
    std::vector<dp1v2::TileResult> results(count);
    for (dp1v2::TileResult& result : results) {
        result.status = dp1v2::TileResultStatus::Disabled;
        result.tile_id = -100;
    }
    return results;
}

} // namespace

TEST(TileExecutorTest, ExecuteFillsAllTaskSlots)
{
    TileFixture fixture = makeTileFixture(4);
    std::vector<dp1v2::TileResult> results = makeSentinelResults(fixture.tasks.size());
    dp1v2::RadiometricStage radiometric_stage;
    const dp1v2::PipelineConfig config = pipelineConfig();
    const dp1v2::TileProcessor processor(radiometric_stage, config);

    const dp1v2::TileExecutionSummary summary = dp1v2::TileExecutor{}.execute(
        fixture.tasks,
        results,
        processor,
        dp1v2::TileExecutionConfig{});

    EXPECT_EQ(summary.total_tasks, fixture.tasks.size());
    EXPECT_EQ(summary.completed_tasks, 0U);
    EXPECT_EQ(summary.failed_tasks, 0U);
    EXPECT_EQ(summary.unsupported_tasks, fixture.tasks.size());

    for (std::size_t index = 0; index < results.size(); ++index) {
        EXPECT_EQ(results[index].tile_id, static_cast<int>(index));
        EXPECT_EQ(results[index].frame_id, 42U);
        EXPECT_EQ(results[index].camera_id, 7);
        EXPECT_EQ(results[index].status, dp1v2::TileResultStatus::Unsupported);
    }
}

TEST(TileExecutorTest, NumThreadsOneUsesExecutorPath)
{
    TileFixture fixture = makeTileFixture(2);
    std::vector<dp1v2::TileResult> results = makeSentinelResults(fixture.tasks.size());
    dp1v2::RadiometricStage radiometric_stage;
    const dp1v2::PipelineConfig config = pipelineConfig();
    const dp1v2::TileProcessor processor(radiometric_stage, config);

    dp1v2::TileExecutionConfig execution_config{};
    execution_config.num_threads = 1;

    const dp1v2::TileExecutionSummary summary = dp1v2::TileExecutor{}.execute(
        fixture.tasks,
        results,
        processor,
        execution_config);

    EXPECT_EQ(summary.total_tasks, 2U);
    EXPECT_EQ(summary.unsupported_tasks, 2U);
    EXPECT_EQ(results[0].tile_id, 0);
    EXPECT_EQ(results[1].tile_id, 1);
}

TEST(TileExecutorTest, FailedTileResultDoesNotBreakExecutor)
{
    TileFixture fixture = makeTileFixture(3);
    fixture.tasks[1].raw_view = nullptr;
    std::vector<dp1v2::TileResult> results = makeSentinelResults(fixture.tasks.size());
    dp1v2::RadiometricStage radiometric_stage;
    const dp1v2::PipelineConfig config = pipelineConfig();
    const dp1v2::TileProcessor processor(radiometric_stage, config);

    const dp1v2::TileExecutionSummary summary = dp1v2::TileExecutor{}.execute(
        fixture.tasks,
        results,
        processor,
        dp1v2::TileExecutionConfig{});

    EXPECT_EQ(summary.total_tasks, 3U);
    EXPECT_EQ(summary.failed_tasks, 1U);
    EXPECT_EQ(summary.unsupported_tasks, 2U);
    EXPECT_EQ(results[0].status, dp1v2::TileResultStatus::Unsupported);
    EXPECT_EQ(results[1].status, dp1v2::TileResultStatus::Failed);
    EXPECT_EQ(results[1].error_code, "invalid_tile_task");
    EXPECT_EQ(results[2].status, dp1v2::TileResultStatus::Unsupported);
}

TEST(TileExecutorTest, RejectsResultsVectorWithWrongSize)
{
    TileFixture fixture = makeTileFixture(2);
    std::vector<dp1v2::TileResult> results(1);
    dp1v2::RadiometricStage radiometric_stage;
    const dp1v2::PipelineConfig config = pipelineConfig();
    const dp1v2::TileProcessor processor(radiometric_stage, config);

    EXPECT_THROW(
        dp1v2::TileExecutor{}.execute(
            fixture.tasks,
            results,
            processor,
            dp1v2::TileExecutionConfig{}),
        std::logic_error);
}
