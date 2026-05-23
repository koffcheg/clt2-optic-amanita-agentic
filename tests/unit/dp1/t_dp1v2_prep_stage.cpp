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

dp1v2::PrepResolvedConfig resolvedTilesConfig(
    const int tile_width = 256,
    const int tile_height = 256,
    const int overlap_x = 16,
    const int overlap_y = 16)
{
    dp1v2::PrepResolvedConfig config{};
    config.tiles = dp1v2::PrepTilesParametersConfig{
        .tile_width = tile_width,
        .tile_height = tile_height,
        .overlap_x = overlap_x,
        .overlap_y = overlap_y,
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

void expectRect(const cv::Rect &actual, const cv::Rect &expected)
{
    EXPECT_EQ(actual.x, expected.x);
    EXPECT_EQ(actual.y, expected.y);
    EXPECT_EQ(actual.width, expected.width);
    EXPECT_EQ(actual.height, expected.height);
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

TEST(PrepStageTest, TilesBuildsGridWithoutOverlap)
{
    cv::Mat image(4, 5, CV_16UC1);
    image.setTo(cv::Scalar(1024));
    const dp1v2::CanonicalFrame frame = makeCanonicalFrame(image);
    dp1v2::PrepStage stage(resolvedTilesConfig(2, 2, 0, 0));

    const auto outcome = processTiles(frame, stage, tilesConfig());

    ASSERT_EQ(outcome.status, dp1v2::StageExecutionStatus::Completed);
    ASSERT_EQ(outcome.output.tiles.size(), 6U);
    ASSERT_EQ(outcome.output.tile_views.size(), outcome.output.tiles.size());

    expectRect(outcome.output.tiles.front().roi_with_border, cv::Rect(0, 0, 2, 2));
    expectRect(outcome.output.tiles.front().valid_area, cv::Rect(0, 0, 2, 2));
    EXPECT_EQ(outcome.output.tiles.front().tile_id, 0);
    EXPECT_EQ(outcome.output.tiles.front().origin_in_frame, cv::Point(0, 0));

    expectRect(outcome.output.tiles.back().roi_with_border, cv::Rect(4, 2, 1, 2));
    expectRect(outcome.output.tiles.back().valid_area, cv::Rect(0, 0, 1, 2));
    EXPECT_EQ(outcome.output.tiles.back().tile_id, 5);
    EXPECT_EQ(outcome.output.tiles.back().origin_in_frame, cv::Point(4, 2));
}

TEST(PrepStageTest, TilesBuildsGridWithOverlapAndClipping)
{
    cv::Mat image(6, 6, CV_16UC1);
    image.setTo(cv::Scalar(1024));
    const dp1v2::CanonicalFrame frame = makeCanonicalFrame(image);
    dp1v2::PrepStage stage(resolvedTilesConfig(3, 3, 1, 1));

    const auto outcome = processTiles(frame, stage, tilesConfig());

    ASSERT_EQ(outcome.status, dp1v2::StageExecutionStatus::Completed);
    ASSERT_EQ(outcome.output.tiles.size(), 4U);
    ASSERT_EQ(outcome.output.tile_views.size(), outcome.output.tiles.size());

    expectRect(outcome.output.tiles[0].roi_with_border, cv::Rect(0, 0, 4, 4));
    expectRect(outcome.output.tiles[0].valid_area, cv::Rect(0, 0, 3, 3));
    expectRect(outcome.output.tiles[1].roi_with_border, cv::Rect(2, 0, 4, 4));
    expectRect(outcome.output.tiles[1].valid_area, cv::Rect(1, 0, 3, 3));
    expectRect(outcome.output.tiles[2].roi_with_border, cv::Rect(0, 2, 4, 4));
    expectRect(outcome.output.tiles[2].valid_area, cv::Rect(0, 1, 3, 3));
    expectRect(outcome.output.tiles[3].roi_with_border, cv::Rect(2, 2, 4, 4));
    expectRect(outcome.output.tiles[3].valid_area, cv::Rect(1, 1, 3, 3));

    for (const dp1v2::TileDesc &desc : outcome.output.tiles) {
        EXPECT_EQ(desc.frame_id, frame.frame_id);
        EXPECT_EQ(desc.origin_in_frame, desc.roi_with_border.tl());
        EXPECT_EQ(desc.coordinate_space, dp1v2::CoordinateSpace::TileLocal);
    }
}

TEST(PrepStageTest, TileViewsAreNonOwningRoiViews)
{
    cv::Mat image(4, 5, CV_16UC1);
    image.setTo(cv::Scalar(1024));
    const dp1v2::CanonicalFrame frame = makeCanonicalFrame(image);
    dp1v2::PrepStage stage(resolvedTilesConfig(3, 2, 1, 1));

    const auto outcome = processTiles(frame, stage, tilesConfig());

    ASSERT_EQ(outcome.status, dp1v2::StageExecutionStatus::Completed);
    ASSERT_EQ(outcome.output.tiles.size(), 4U);
    ASSERT_EQ(outcome.output.tile_views.size(), outcome.output.tiles.size());

    for (std::size_t index = 0; index < outcome.output.tiles.size(); ++index) {
        const dp1v2::TileDesc &desc = outcome.output.tiles[index];
        const dp1v2::TileRawView &view = outcome.output.tile_views[index];

        EXPECT_EQ(desc.frame_id, frame.frame_id);
        EXPECT_EQ(desc.tile_id, static_cast<int>(index));
        EXPECT_EQ(desc.origin_in_frame, desc.roi_with_border.tl());
        EXPECT_EQ(desc.coordinate_space, dp1v2::CoordinateSpace::TileLocal);

        EXPECT_EQ(view.frame_id, frame.frame_id);
        EXPECT_EQ(view.camera_id, frame.camera_id);
        EXPECT_EQ(view.tile_id, desc.tile_id);
        EXPECT_EQ(view.pixel_format, frame.pixel_format);
        EXPECT_EQ(view.bit_depth, frame.bit_depth);
        EXPECT_EQ(view.pixel_range.min_value, frame.pixel_range.min_value);
        EXPECT_EQ(view.pixel_range.max_value, frame.pixel_range.max_value);
        EXPECT_EQ(view.pixel_range.black_level, frame.pixel_range.black_level);
        EXPECT_EQ(view.pixel_range.saturation_level, frame.pixel_range.saturation_level);
        EXPECT_EQ(view.geometry.width, desc.roi_with_border.width);
        EXPECT_EQ(view.geometry.height, desc.roi_with_border.height);
        EXPECT_EQ(view.geometry.origin_px, desc.roi_with_border.tl());
        EXPECT_EQ(view.origin_in_frame, desc.origin_in_frame);
        expectRect(view.valid_area, desc.valid_area);
        EXPECT_EQ(view.coordinate_space, dp1v2::CoordinateSpace::TileLocal);

        EXPECT_EQ(view.image.rows, desc.roi_with_border.height);
        EXPECT_EQ(view.image.cols, desc.roi_with_border.width);
        EXPECT_EQ(view.image.datastart, frame.image.datastart);
        EXPECT_EQ(view.image.dataend, frame.image.dataend);
        EXPECT_EQ(view.image.data, frame.image.ptr(desc.roi_with_border.y, desc.roi_with_border.x));
    }
}

TEST(PrepStageTest, TilesRejectInvalidCanonicalFrame)
{
    dp1v2::PrepStage stage(resolvedTilesConfig(2, 2, 0, 0));

    const dp1v2::CanonicalFrame empty_frame = makeCanonicalFrame(cv::Mat{});
    EXPECT_EQ(
        processTiles(empty_frame, stage, tilesConfig()).status,
        dp1v2::StageExecutionStatus::Failed);

    cv::Mat image(2, 3, CV_16UC1);
    dp1v2::CanonicalFrame geometry_mismatch_frame = makeCanonicalFrame(image);
    geometry_mismatch_frame.geometry.width = image.cols + 1;
    EXPECT_EQ(
        processTiles(geometry_mismatch_frame, stage, tilesConfig()).status,
        dp1v2::StageExecutionStatus::Failed);

    cv::Mat multichannel_image(2, 3, CV_16UC3);
    const dp1v2::CanonicalFrame multichannel_frame = makeCanonicalFrame(multichannel_image);
    EXPECT_EQ(
        processTiles(multichannel_frame, stage, tilesConfig()).status,
        dp1v2::StageExecutionStatus::Failed);

    dp1v2::CanonicalFrame tile_local_frame = makeCanonicalFrame(image);
    tile_local_frame.coordinate_space = dp1v2::CoordinateSpace::TileLocal;
    EXPECT_EQ(
        processTiles(tile_local_frame, stage, tilesConfig()).status,
        dp1v2::StageExecutionStatus::Failed);
}

TEST(PrepStageTest, TilesRejectsNonTilesVariant)
{
    cv::Mat image(2, 3, CV_16UC1);
    const dp1v2::CanonicalFrame frame = makeCanonicalFrame(image);
    dp1v2::PrepStage stage(resolvedTilesConfig(2, 2, 0, 0));

    for (const char *variant : {"full_frame", "roi", "adaptive_roi"}) {
        dp1v2::StageConfig config = tilesConfig();
        config.variant = variant;

        const auto outcome = processTiles(frame, stage, config);

        EXPECT_EQ(outcome.status, dp1v2::StageExecutionStatus::Unsupported) << variant;
    }
}

TEST(PrepStageTest, FullFrameAcceptsBinnedCanonicalFrame)
{
    cv::Mat image(2, 2, CV_16UC1);
    image.setTo(cv::Scalar(256));
    dp1v2::CanonicalFrame frame = makeCanonicalFrame(image);
    frame.image_ownership = dp1v2::CanonicalPayloadOwnership::OwnedBinned;
    frame.normalization.binned = true;
    frame.normalization.bin_factor_x = 2;
    frame.normalization.bin_factor_y = 2;
    frame.normalization.binning_mode = dp1v2::BinningMode::Average;

    const auto outcome = processFrame(frame, fullFrameConfig());

    ASSERT_EQ(outcome.status, dp1v2::StageExecutionStatus::Completed);
    ASSERT_NE(outcome.output.frame, nullptr);
    EXPECT_EQ(outcome.output.frame, &frame);
}

TEST(PrepStageTest, TilesBuildsLayoutFromBinnedCanonicalFrame)
{
    cv::Mat image(4, 4, CV_16UC1);
    image.setTo(cv::Scalar(256));
    dp1v2::CanonicalFrame frame = makeCanonicalFrame(image);
    frame.image_ownership = dp1v2::CanonicalPayloadOwnership::OwnedBinned;
    frame.normalization.binned = true;
    frame.normalization.bin_factor_x = 2;
    frame.normalization.bin_factor_y = 2;
    frame.normalization.binning_mode = dp1v2::BinningMode::Average;

    dp1v2::PrepStage stage(resolvedTilesConfig(2, 2, 0, 0));
    const auto outcome = processTiles(frame, stage, tilesConfig());

    ASSERT_EQ(outcome.status, dp1v2::StageExecutionStatus::Completed);
    EXPECT_FALSE(outcome.output.tiles.empty());
    EXPECT_EQ(outcome.output.tiles.size(), outcome.output.tile_views.size());
}
