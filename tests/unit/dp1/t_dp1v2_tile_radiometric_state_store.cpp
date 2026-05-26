#include <gtest/gtest.h>

#include <cstdint>
#include <vector>

#include <opencv2/core.hpp>

#include "dp1v2/domain/canonical_frame.hpp"
#include "dp1v2/domain/frame_context.hpp"
#include "dp1v2/stages/prep_stage.hpp"
#include "dp1v2/stages/radiometric_stage.hpp"
#include "dp1v2/stages/tile_radiometric_state_store.hpp"

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
        .variant = "inverse_median",
        .level = "L1",
        .parameters = {},
    };
}

dp1v2::RadiometricResolvedConfig resolvedRadiometricConfig()
{
    dp1v2::RadiometricResolvedConfig config{};
    config.inverse_median = dp1v2::InverseMedianParametersConfig{};
    config.inverse_median->mode = dp1v2::InverseMedianMode::FixedK3;
    config.inverse_median->stride = 1;
    config.inverse_median->output_dynamic_range_mode =
        dp1v2::InverseMedianOutputMode::RawSigned;
    return config;
}

dp1v2::TileRawView makeTileView(
    cv::Mat& frame,
    const std::uint64_t frame_id,
    const int camera_id,
    const int tile_id,
    const cv::Rect roi)
{
    return dp1v2::TileRawView{
        .frame_id = frame_id,
        .camera_id = camera_id,
        .tile_id = tile_id,
        .image = frame(roi),
        .pixel_format = dp1v2::PixelFormat::U16,
        .bit_depth = dp1v2::InputBitDepth::Bit16,
        .pixel_range = rangeU16(),
        .geometry = dp1v2::FrameGeometry{
            .width = roi.width,
            .height = roi.height,
            .origin_px = roi.tl(),
        },
        .origin_in_frame = roi.tl(),
        .valid_area = cv::Rect(0, 0, roi.width, roi.height),
        .coordinate_space = dp1v2::CoordinateSpace::TileLocal,
    };
}

dp1v2::PrepTilesOutput makePrepOutputForTwoTiles(
    cv::Mat& frame,
    const std::uint64_t frame_id,
    const std::uint16_t left_value,
    const std::uint16_t right_value)
{
    frame = cv::Mat(1, 2, CV_16UC1);
    frame.at<std::uint16_t>(0, 0) = left_value;
    frame.at<std::uint16_t>(0, 1) = right_value;

    dp1v2::PrepTilesOutput output{};
    output.tiles = {
        dp1v2::TileDesc{
            .frame_id = frame_id,
            .tile_id = 0,
            .roi_with_border = cv::Rect(0, 0, 1, 1),
            .valid_area = cv::Rect(0, 0, 1, 1),
            .origin_in_frame = cv::Point(0, 0),
        },
        dp1v2::TileDesc{
            .frame_id = frame_id,
            .tile_id = 1,
            .roi_with_border = cv::Rect(1, 0, 1, 1),
            .valid_area = cv::Rect(0, 0, 1, 1),
            .origin_in_frame = cv::Point(1, 0),
        },
    };
    output.tile_views = {
        makeTileView(frame, frame_id, 7, 0, cv::Rect(0, 0, 1, 1)),
        makeTileView(frame, frame_id, 7, 1, cv::Rect(1, 0, 1, 1)),
    };
    return output;
}

dp1v2::StageOutcome<dp1v2::RadiometricTileOutput> processTile(
    dp1v2::RadiometricStage& stage,
    const dp1v2::TileRawView& tile)
{
    dp1v2::TileContext tile_context{};
    tile_context.frame_id = tile.frame_id;
    tile_context.tile_id = tile.tile_id;
    return stage.process(
        dp1v2::RadiometricTileInput{.tile = tile},
        tile_context,
        radiometricConfig());
}

dp1v2::CanonicalFrame makeCanonicalFrame(cv::Mat& image, const std::uint64_t frame_id)
{
    dp1v2::CanonicalFrame frame{};
    frame.frame_id = frame_id;
    frame.camera_id = 7;
    frame.image = image;
    frame.pixel_format = dp1v2::PixelFormat::U16;
    frame.bit_depth = dp1v2::InputBitDepth::Bit16;
    frame.pixel_range = rangeU16();
    frame.geometry = dp1v2::FrameGeometry{.width = image.cols, .height = image.rows};
    frame.coordinate_space = dp1v2::CoordinateSpace::FrameGlobal;
    return frame;
}

} // namespace

TEST(TileRadiometricStateStoreTest, TwoTilesDoNotShareInverseMedianHistory)
{
    cv::Mat frame1;
    dp1v2::PrepTilesOutput prep1 = makePrepOutputForTwoTiles(frame1, 1, 10, 100);
    cv::Mat frame2;
    dp1v2::PrepTilesOutput prep2 = makePrepOutputForTwoTiles(frame2, 2, 20, 200);
    cv::Mat frame3;
    dp1v2::PrepTilesOutput prep3 = makePrepOutputForTwoTiles(frame3, 3, 30, 300);

    dp1v2::RadiometricStage stage(resolvedRadiometricConfig());

    ASSERT_FALSE(stage.prepareTileStates(prep1, radiometricConfig(), cv::Size(2, 1), 1));
    EXPECT_EQ(processTile(stage, prep1.tile_views[0]).status, dp1v2::StageExecutionStatus::Skipped);
    EXPECT_EQ(processTile(stage, prep1.tile_views[1]).status, dp1v2::StageExecutionStatus::Skipped);

    ASSERT_FALSE(stage.prepareTileStates(prep2, radiometricConfig(), cv::Size(2, 1), 1));
    EXPECT_EQ(processTile(stage, prep2.tile_views[0]).status, dp1v2::StageExecutionStatus::Skipped);
    EXPECT_EQ(processTile(stage, prep2.tile_views[1]).status, dp1v2::StageExecutionStatus::Skipped);

    ASSERT_FALSE(stage.prepareTileStates(prep3, radiometricConfig(), cv::Size(2, 1), 1));
    const auto left = processTile(stage, prep3.tile_views[0]);
    const auto right = processTile(stage, prep3.tile_views[1]);

    ASSERT_EQ(left.status, dp1v2::StageExecutionStatus::Completed);
    ASSERT_EQ(right.status, dp1v2::StageExecutionStatus::Completed);
    ASSERT_FALSE(left.output.frame.image.empty());
    ASSERT_FALSE(right.output.frame.image.empty());
    EXPECT_EQ(left.output.frame.image.type(), CV_32FC1);
    EXPECT_EQ(right.output.frame.image.type(), CV_32FC1);
    EXPECT_EQ(left.output.frame.pixel_format, dp1v2::PixelFormat::F32);
    EXPECT_EQ(right.output.frame.pixel_format, dp1v2::PixelFormat::F32);
    EXPECT_FLOAT_EQ(left.output.frame.image.at<float>(0, 0), 10.0F);
    EXPECT_FLOAT_EQ(right.output.frame.image.at<float>(0, 0), 100.0F);
}

TEST(TileRadiometricStateStoreTest, TileStatePersistsAcrossFramesForSameTile)
{
    dp1v2::RadiometricStage stage(resolvedRadiometricConfig());

    cv::Mat frame1;
    dp1v2::PrepTilesOutput prep1 = makePrepOutputForTwoTiles(frame1, 1, 10, 100);
    ASSERT_FALSE(stage.prepareTileStates(prep1, radiometricConfig(), cv::Size(2, 1), 1));
    EXPECT_EQ(processTile(stage, prep1.tile_views[0]).status, dp1v2::StageExecutionStatus::Skipped);

    cv::Mat frame2;
    dp1v2::PrepTilesOutput prep2 = makePrepOutputForTwoTiles(frame2, 2, 20, 100);
    ASSERT_FALSE(stage.prepareTileStates(prep2, radiometricConfig(), cv::Size(2, 1), 1));
    EXPECT_EQ(processTile(stage, prep2.tile_views[0]).status, dp1v2::StageExecutionStatus::Skipped);

    cv::Mat frame3;
    dp1v2::PrepTilesOutput prep3 = makePrepOutputForTwoTiles(frame3, 3, 30, 100);
    ASSERT_FALSE(stage.prepareTileStates(prep3, radiometricConfig(), cv::Size(2, 1), 1));
    EXPECT_EQ(processTile(stage, prep3.tile_views[0]).status, dp1v2::StageExecutionStatus::Completed);
}

TEST(TileRadiometricStateStoreTest, TileStateResetsWhenSignatureChanges)
{
    dp1v2::RadiometricStage stage(resolvedRadiometricConfig());

    cv::Mat frame1;
    dp1v2::PrepTilesOutput prep1 = makePrepOutputForTwoTiles(frame1, 1, 10, 100);
    ASSERT_FALSE(stage.prepareTileStates(prep1, radiometricConfig(), cv::Size(2, 1), 1));
    EXPECT_EQ(processTile(stage, prep1.tile_views[0]).status, dp1v2::StageExecutionStatus::Skipped);

    cv::Mat frame2;
    dp1v2::PrepTilesOutput prep2 = makePrepOutputForTwoTiles(frame2, 2, 20, 100);
    ASSERT_FALSE(stage.prepareTileStates(prep2, radiometricConfig(), cv::Size(2, 1), 1));
    EXPECT_EQ(processTile(stage, prep2.tile_views[0]).status, dp1v2::StageExecutionStatus::Skipped);

    cv::Mat changed_frame(1, 1, CV_16UC1);
    changed_frame.at<std::uint16_t>(0, 0) = 30;
    dp1v2::PrepTilesOutput changed{};
    changed.tiles.push_back(dp1v2::TileDesc{
        .frame_id = 3,
        .tile_id = 0,
        .roi_with_border = cv::Rect(0, 0, 1, 1),
        .valid_area = cv::Rect(0, 0, 1, 1),
        .origin_in_frame = cv::Point(0, 0),
    });
    changed.tile_views.push_back(makeTileView(changed_frame, 3, 7, 0, cv::Rect(0, 0, 1, 1)));

    ASSERT_FALSE(stage.prepareTileStates(changed, radiometricConfig(), cv::Size(1, 1), 1));
    EXPECT_EQ(processTile(stage, changed.tile_views[0]).status, dp1v2::StageExecutionStatus::Skipped);
}

TEST(TileRadiometricStateStoreTest, MissingPreparedStateDoesNotInsertInWorkerPath)
{
    cv::Mat frame;
    dp1v2::PrepTilesOutput prep = makePrepOutputForTwoTiles(frame, 1, 10, 100);
    dp1v2::RadiometricStage stage(resolvedRadiometricConfig());

    const auto outcome = processTile(stage, prep.tile_views[0]);

    EXPECT_EQ(outcome.status, dp1v2::StageExecutionStatus::Failed);
    EXPECT_EQ(outcome.reason, "tile radiometric state was not prepared for the requested tile");
}

TEST(TileRadiometricStateStoreTest, FullFrameAndTileStatesAreIndependent)
{
    dp1v2::RadiometricStage stage(resolvedRadiometricConfig());
    dp1v2::FrameContext context{};

    cv::Mat full1(1, 1, CV_16UC1);
    full1.at<std::uint16_t>(0, 0) = 10;
    cv::Mat full2(1, 1, CV_16UC1);
    full2.at<std::uint16_t>(0, 0) = 20;
    cv::Mat full3(1, 1, CV_16UC1);
    full3.at<std::uint16_t>(0, 0) = 30;

    EXPECT_EQ(
        stage.process(
            dp1v2::RadiometricFullFrameInput{.frame = makeCanonicalFrame(full1, 1)},
            context,
            radiometricConfig()).status,
        dp1v2::StageExecutionStatus::Skipped);
    EXPECT_EQ(
        stage.process(
            dp1v2::RadiometricFullFrameInput{.frame = makeCanonicalFrame(full2, 2)},
            context,
            radiometricConfig()).status,
        dp1v2::StageExecutionStatus::Skipped);
    EXPECT_EQ(
        stage.process(
            dp1v2::RadiometricFullFrameInput{.frame = makeCanonicalFrame(full3, 3)},
            context,
            radiometricConfig()).status,
        dp1v2::StageExecutionStatus::Completed);

    cv::Mat tile_frame;
    dp1v2::PrepTilesOutput prep = makePrepOutputForTwoTiles(tile_frame, 4, 40, 100);
    ASSERT_FALSE(stage.prepareTileStates(prep, radiometricConfig(), cv::Size(2, 1), 1));
    EXPECT_EQ(processTile(stage, prep.tile_views[0]).status, dp1v2::StageExecutionStatus::Skipped);
}

TEST(TileRadiometricStateStoreTest, PerTileWarmUpIsIndependent)
{
    dp1v2::RadiometricStage stage(resolvedRadiometricConfig());

    cv::Mat frame1;
    dp1v2::PrepTilesOutput prep1 = makePrepOutputForTwoTiles(frame1, 1, 10, 100);
    ASSERT_FALSE(stage.prepareTileStates(prep1, radiometricConfig(), cv::Size(2, 1), 1));
    EXPECT_EQ(processTile(stage, prep1.tile_views[0]).status, dp1v2::StageExecutionStatus::Skipped);

    cv::Mat frame2;
    dp1v2::PrepTilesOutput prep2 = makePrepOutputForTwoTiles(frame2, 2, 20, 200);
    ASSERT_FALSE(stage.prepareTileStates(prep2, radiometricConfig(), cv::Size(2, 1), 1));
    EXPECT_EQ(processTile(stage, prep2.tile_views[0]).status, dp1v2::StageExecutionStatus::Skipped);

    cv::Mat frame3;
    dp1v2::PrepTilesOutput prep3 = makePrepOutputForTwoTiles(frame3, 3, 30, 300);
    ASSERT_FALSE(stage.prepareTileStates(prep3, radiometricConfig(), cv::Size(2, 1), 1));

    EXPECT_EQ(processTile(stage, prep3.tile_views[0]).status, dp1v2::StageExecutionStatus::Completed);
    EXPECT_EQ(processTile(stage, prep3.tile_views[1]).status, dp1v2::StageExecutionStatus::Skipped);
}

TEST(TileRadiometricStateStoreTest, StoreLookupDoesNotCreateMissingState)
{
    dp1v2::TileRadiometricStateStore store;

    EXPECT_THROW(
        store.getTileState(dp1v2::TileRadiometricStateKey{.camera_id = 7, .tile_id = 0}),
        std::logic_error);
    EXPECT_EQ(store.size(), 0U);
}
