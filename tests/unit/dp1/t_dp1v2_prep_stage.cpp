#include <gtest/gtest.h>

#include <opencv2/core.hpp>

#include "dp1v2/domain/frame_context.hpp"
#include "dp1v2/stages/prep_stage.hpp"

namespace {

dp1v2::StageConfig fullFrameConfig(const bool enabled = true)
{
    return dp1v2::StageConfig{
        .enabled = enabled,
        .variant = "full_frame",
        .level = "L0",
        .parameters = {},
    };
}

dp1v2::StageConfig tilesConfig(const bool enabled = true)
{
    return dp1v2::StageConfig{
        .enabled = enabled,
        .variant = "tiles",
        .level = "L1",
        .parameters = {},
    };
}

dp1v2::PrepResolvedConfig resolvedTilesConfig()
{
    dp1v2::PrepResolvedConfig config{};
    config.tiles = dp1v2::PrepTilesParametersConfig{
        .tile_width = 256,
        .tile_height = 256,
        .overlap_x = 16,
        .overlap_y = 16,
    };
    return config;
}

dp1v2::PixelRange rangeU16()
{
    return dp1v2::PixelRange{
        .min_value = 0.0,
        .max_value = 65535.0,
        .black_level = 0.0,
        .saturation_level = 65535.0,
    };
}

dp1v2::CanonicalFrame makeCanonicalFrame(const cv::Mat &image)
{
    dp1v2::CanonicalFrame frame{};
    frame.frame_id = 42;
    frame.camera_id = 7;
    frame.source_id = "prep-stage-source";
    frame.image = image;
    frame.image_ownership = dp1v2::CanonicalPayloadOwnership::BorrowedReadOnly;
    frame.pixel_format = dp1v2::PixelFormat::U16;
    frame.bit_depth = dp1v2::InputBitDepth::Bit16;
    frame.pixel_range = rangeU16();
    frame.geometry = dp1v2::FrameGeometry{.width = image.cols, .height = image.rows};
    frame.coordinate_space = dp1v2::CoordinateSpace::FrameGlobal;
    return frame;
}

dp1v2::StageOutcome<dp1v2::PrepFullFrameOutput> processFrame(
    const dp1v2::CanonicalFrame &frame,
    const dp1v2::StageConfig &config)
{
    dp1v2::FrameContext context{};
    dp1v2::PrepStage stage;
    return stage.process(dp1v2::PrepFullFrameInput{.frame = frame}, context, config);
}

dp1v2::StageOutcome<dp1v2::PrepTilesOutput> processTiles(
    const dp1v2::CanonicalFrame &frame,
    dp1v2::PrepStage &stage,
    const dp1v2::StageConfig &config)
{
    dp1v2::FrameContext context{};
    return stage.process(dp1v2::PrepTilesInput{.frame = frame}, context, config);
}

} // namespace

TEST(PrepStageTest, FullFrameCompletedReturnsSameCanonicalFrame)
{
    cv::Mat image(2, 3, CV_16UC1);
    image.setTo(cv::Scalar(1024));
    const dp1v2::CanonicalFrame frame = makeCanonicalFrame(image);

    const auto outcome = processFrame(frame, fullFrameConfig());

    ASSERT_EQ(outcome.status, dp1v2::StageExecutionStatus::Completed);
    ASSERT_NE(outcome.output.frame, nullptr);
    EXPECT_EQ(outcome.output.frame, &frame);
    EXPECT_EQ(outcome.output.frame->image.data, frame.image.data);
}

TEST(PrepStageTest, FullFrameDisabledReturnsDisabled)
{
    cv::Mat image(2, 3, CV_16UC1);
    const dp1v2::CanonicalFrame frame = makeCanonicalFrame(image);

    const auto outcome = processFrame(frame, fullFrameConfig(false));

    EXPECT_EQ(outcome.status, dp1v2::StageExecutionStatus::Disabled);
}

TEST(PrepStageTest, FullFrameRejectsNonFullFrameVariant)
{
    cv::Mat image(2, 3, CV_16UC1);
    const dp1v2::CanonicalFrame frame = makeCanonicalFrame(image);

    for (const char *variant : {"tiles", "roi", "adaptive_roi"}) {
        dp1v2::StageConfig config = fullFrameConfig();
        config.variant = variant;

        const auto outcome = processFrame(frame, config);

        EXPECT_EQ(outcome.status, dp1v2::StageExecutionStatus::Unsupported) << variant;
    }
}

TEST(PrepStageTest, FullFrameRejectsInvalidCanonicalFrame)
{
    const dp1v2::CanonicalFrame empty_frame = makeCanonicalFrame(cv::Mat{});
    EXPECT_EQ(
        processFrame(empty_frame, fullFrameConfig()).status,
        dp1v2::StageExecutionStatus::Failed);

    cv::Mat image(2, 3, CV_16UC1);
    dp1v2::CanonicalFrame geometry_mismatch_frame = makeCanonicalFrame(image);
    geometry_mismatch_frame.geometry.width = image.cols + 1;

    EXPECT_EQ(
        processFrame(geometry_mismatch_frame, fullFrameConfig()).status,
        dp1v2::StageExecutionStatus::Failed);
}
TEST(PrepStageTest, CapabilitiesExposeTilesOnlyWhenResolvedConfigExists)
{
    const dp1v2::PrepStage default_stage;
    const auto default_capabilities = default_stage.capabilities();
    EXPECT_TRUE(default_capabilities.supports_full_frame);
    EXPECT_FALSE(default_capabilities.supports_tiles);

    const dp1v2::PrepStage configured_stage(resolvedTilesConfig());
    const auto configured_capabilities = configured_stage.capabilities();
    EXPECT_TRUE(configured_capabilities.supports_full_frame);
    EXPECT_TRUE(configured_capabilities.supports_tiles);
}

TEST(PrepStageTest, TilesRouteFailsWhenResolvedConfigIsMissing)
{
    cv::Mat image(2, 3, CV_16UC1);
    const dp1v2::CanonicalFrame frame = makeCanonicalFrame(image);
    dp1v2::PrepStage stage;

    const auto outcome = processTiles(frame, stage, tilesConfig());

    EXPECT_EQ(outcome.status, dp1v2::StageExecutionStatus::Failed);
    EXPECT_EQ(outcome.reason, "prep tiles resolved configuration is missing");
}

TEST(PrepStageTest, TilesRouteWithResolvedConfigRemainsExplicitlyUnsupportedUntilImplemented)
{
    cv::Mat image(2, 3, CV_16UC1);
    const dp1v2::CanonicalFrame frame = makeCanonicalFrame(image);
    dp1v2::PrepStage stage(resolvedTilesConfig());

    const auto outcome = processTiles(frame, stage, tilesConfig());

    EXPECT_EQ(outcome.status, dp1v2::StageExecutionStatus::Unsupported);
    EXPECT_EQ(outcome.reason, "prep tiles route is not implemented");
}
