#include <gtest/gtest.h>

#include <cstdint>
#include <stdexcept>

#include <opencv2/core.hpp>

#include "dp1v2/runtime/stage2_boundary_adapter.hpp"

namespace {

dp1v2::PixelRange rawRange()
{
    return dp1v2::PixelRange{
        .min_value = 100.0,
        .max_value = 1100.0,
        .black_level = 100.0,
        .saturation_level = 1100.0,
    };
}

dp1v2::PixelRange rawRange12Bit()
{
    return dp1v2::PixelRange{
        .min_value = 0.0,
        .max_value = 4095.0,
        .black_level = 0.0,
        .saturation_level = 4095.0,
    };
}

dp1v2::CanonicalFrame makeCanonicalFrame(
    const cv::Mat& image,
    const dp1v2::PixelFormat format,
    const dp1v2::PixelRange& range = rawRange())
{
    dp1v2::CanonicalFrame frame{};
    frame.frame_id = 42;
    frame.camera_id = 7;
    frame.source_id = "stage2-boundary-source";
    frame.image = image;
    frame.image_ownership = dp1v2::CanonicalPayloadOwnership::BorrowedReadOnly;
    frame.pixel_format = format;
    frame.bit_depth = format == dp1v2::PixelFormat::U8
        ? dp1v2::InputBitDepth::Bit8
        : dp1v2::InputBitDepth::Bit16;
    frame.pixel_range = range;
    frame.geometry = dp1v2::FrameGeometry{.width = image.cols, .height = image.rows};
    frame.coordinate_space = dp1v2::CoordinateSpace::FrameGlobal;
    return frame;
}

dp1v2::TileRawView makeTileRawView(
    const cv::Mat& image,
    const dp1v2::PixelFormat format,
    const dp1v2::PixelRange& range = rawRange())
{
    dp1v2::TileRawView tile{};
    tile.frame_id = 42;
    tile.camera_id = 7;
    tile.tile_id = 3;
    tile.image = image;
    tile.pixel_format = format;
    tile.bit_depth = format == dp1v2::PixelFormat::U8
        ? dp1v2::InputBitDepth::Bit8
        : dp1v2::InputBitDepth::Bit16;
    tile.pixel_range = range;
    tile.geometry = dp1v2::FrameGeometry{.width = image.cols, .height = image.rows};
    tile.origin_in_frame = cv::Point{4, 6};
    tile.valid_area = cv::Rect{1, 1, image.cols - 1, image.rows - 1};
    tile.coordinate_space = dp1v2::CoordinateSpace::TileLocal;
    return tile;
}

dp1v2::StageOutcome<dp1v2::RadiometricFullFrameOutput> fullFrameOutcome(
    const dp1v2::StageExecutionStatus status)
{
    return dp1v2::StageOutcome<dp1v2::RadiometricFullFrameOutput>{
        .status = status,
        .output = {},
        .reason = {},
    };
}

dp1v2::StageOutcome<dp1v2::RadiometricTileOutput> tileOutcome(
    const dp1v2::StageExecutionStatus status)
{
    return dp1v2::StageOutcome<dp1v2::RadiometricTileOutput>{
        .status = status,
        .output = {},
        .reason = {},
    };
}

} // namespace

TEST(Stage2BoundaryAdapterTest, BoundaryKeyIsStable)
{
    const dp1v2::Stage2BoundaryAdapter adapter;

    EXPECT_EQ(adapter.boundaryKey(), "stage2.processing_frame");
}

TEST(Stage2BoundaryAdapterTest, CompletedFullFrameSelectsRadiometricOutput)
{
    cv::Mat input_image(2, 3, CV_8UC1);
    const dp1v2::CanonicalFrame input = makeCanonicalFrame(input_image, dp1v2::PixelFormat::U8);

    cv::Mat radiometric_image(2, 3, CV_32FC1);
    dp1v2::StageOutcome<dp1v2::RadiometricFullFrameOutput> outcome =
        fullFrameOutcome(dp1v2::StageExecutionStatus::Completed);
    outcome.output.frame.frame_id = 99;
    outcome.output.frame.image = radiometric_image;
    outcome.output.frame.pixel_format = dp1v2::PixelFormat::F32;
    outcome.output.frame.processing_domain = dp1v2::ProcessingDomain::RadiometricResidual;

    dp1v2::Stage2BoundaryWorkspace workspace{};
    const dp1v2::Stage2BoundaryAdapter adapter;
    const dp1v2::Stage2FullFrameSelection selection =
        adapter.selectFullFrameOutput(input, outcome, workspace);

    EXPECT_FALSE(selection.is_bypass);
    EXPECT_EQ(selection.source, dp1v2::Stage2BoundarySource::RadiometricOutput);
    EXPECT_EQ(selection.frame.frame_id, 99U);
    EXPECT_EQ(selection.frame.image.data, radiometric_image.data);
    EXPECT_EQ(selection.frame.pixel_format, dp1v2::PixelFormat::F32);
    EXPECT_EQ(selection.frame.processing_domain, dp1v2::ProcessingDomain::RadiometricResidual);
}

TEST(Stage2BoundaryAdapterTest, DisabledFullFrameU8BypassUsesShallowImage)
{
    cv::Mat input_image(2, 3, CV_8UC1);
    input_image.setTo(cv::Scalar{17});
    const dp1v2::CanonicalFrame input = makeCanonicalFrame(input_image, dp1v2::PixelFormat::U8);

    dp1v2::Stage2BoundaryWorkspace workspace{};
    const dp1v2::Stage2BoundaryAdapter adapter;
    const dp1v2::Stage2FullFrameSelection selection = adapter.selectFullFrameOutput(
        input,
        fullFrameOutcome(dp1v2::StageExecutionStatus::Disabled),
        workspace);

    EXPECT_TRUE(selection.is_bypass);
    EXPECT_EQ(selection.source, dp1v2::Stage2BoundarySource::RawBypassFromDisabledRadiometric);
    EXPECT_EQ(selection.frame.image.data, input.image.data);
    EXPECT_TRUE(workspace.full_frame_bypass_u8.empty());
    EXPECT_EQ(selection.frame.pixel_format, dp1v2::PixelFormat::U8);
    EXPECT_EQ(selection.frame.processing_domain, dp1v2::ProcessingDomain::RawIntensity);
    EXPECT_EQ(selection.frame.range_policy, dp1v2::RangePolicy::RawSensorRange);
    EXPECT_DOUBLE_EQ(selection.frame.value_range.min_value, input.pixel_range.min_value);
    EXPECT_DOUBLE_EQ(selection.frame.value_range.max_value, input.pixel_range.max_value);
}

TEST(Stage2BoundaryAdapterTest, DisabledFullFrameU16BypassScalesIntoWorkspace)
{
    cv::Mat input_image(1, 3, CV_16UC1);
    input_image.at<std::uint16_t>(0, 0) = 100;
    input_image.at<std::uint16_t>(0, 1) = 600;
    input_image.at<std::uint16_t>(0, 2) = 1100;
    const dp1v2::CanonicalFrame input = makeCanonicalFrame(input_image, dp1v2::PixelFormat::U16);

    dp1v2::Stage2BoundaryWorkspace workspace{};
    const dp1v2::Stage2BoundaryAdapter adapter;
    const dp1v2::Stage2FullFrameSelection selection = adapter.selectFullFrameOutput(
        input,
        fullFrameOutcome(dp1v2::StageExecutionStatus::Disabled),
        workspace);

    EXPECT_TRUE(selection.is_bypass);
    EXPECT_EQ(selection.source, dp1v2::Stage2BoundarySource::RawBypassFromDisabledRadiometric);
    EXPECT_EQ(selection.frame.image.data, workspace.full_frame_bypass_u8.data);
    EXPECT_NE(selection.frame.image.data, input.image.data);
    ASSERT_EQ(selection.frame.image.type(), CV_8UC1);
    EXPECT_EQ(selection.frame.image.at<std::uint8_t>(0, 0), 0U);
    EXPECT_EQ(selection.frame.image.at<std::uint8_t>(0, 1), 128U);
    EXPECT_EQ(selection.frame.image.at<std::uint8_t>(0, 2), 255U);
    EXPECT_EQ(selection.frame.pixel_format, dp1v2::PixelFormat::U8);
    EXPECT_EQ(selection.frame.processing_domain, dp1v2::ProcessingDomain::RawIntensity);
}

TEST(Stage2BoundaryAdapterTest, SkippedFullFrameU16BypassScalesIntoWorkspace)
{
    cv::Mat input_image(1, 3, CV_16UC1);
    input_image.at<std::uint16_t>(0, 0) = 100;
    input_image.at<std::uint16_t>(0, 1) = 600;
    input_image.at<std::uint16_t>(0, 2) = 1100;
    const dp1v2::CanonicalFrame input = makeCanonicalFrame(input_image, dp1v2::PixelFormat::U16);

    dp1v2::Stage2BoundaryWorkspace workspace{};
    const dp1v2::Stage2BoundaryAdapter adapter;
    const dp1v2::Stage2FullFrameSelection selection = adapter.selectFullFrameOutput(
        input,
        fullFrameOutcome(dp1v2::StageExecutionStatus::Skipped),
        workspace);

    EXPECT_TRUE(selection.is_bypass);
    EXPECT_EQ(selection.source, dp1v2::Stage2BoundarySource::RawBypassFromSkippedRadiometric);
    EXPECT_EQ(selection.frame.image.data, workspace.full_frame_bypass_u8.data);
    EXPECT_NE(selection.frame.image.data, input.image.data);
    ASSERT_EQ(selection.frame.image.type(), CV_8UC1);
    EXPECT_EQ(selection.frame.image.at<std::uint8_t>(0, 0), 0U);
    EXPECT_EQ(selection.frame.image.at<std::uint8_t>(0, 1), 128U);
    EXPECT_EQ(selection.frame.image.at<std::uint8_t>(0, 2), 255U);
    EXPECT_EQ(selection.frame.processing_domain, dp1v2::ProcessingDomain::RawIntensity);
}

TEST(Stage2BoundaryAdapterTest, FullFrameU16BypassScalesUsingPixelRange)
{
    cv::Mat input_image(1, 3, CV_16UC1);
    input_image.at<std::uint16_t>(0, 0) = 0;
    input_image.at<std::uint16_t>(0, 1) = 2048;
    input_image.at<std::uint16_t>(0, 2) = 4095;
    const dp1v2::CanonicalFrame input = makeCanonicalFrame(
        input_image,
        dp1v2::PixelFormat::U16,
        rawRange12Bit());

    dp1v2::Stage2BoundaryWorkspace workspace{};
    const dp1v2::Stage2BoundaryAdapter adapter;
    const dp1v2::Stage2FullFrameSelection selection = adapter.selectFullFrameOutput(
        input,
        fullFrameOutcome(dp1v2::StageExecutionStatus::Disabled),
        workspace);

    ASSERT_EQ(selection.frame.image.type(), CV_8UC1);
    EXPECT_EQ(selection.frame.image.at<std::uint8_t>(0, 0), 0U);
    EXPECT_NEAR(static_cast<int>(selection.frame.image.at<std::uint8_t>(0, 1)), 128, 1);
    EXPECT_EQ(selection.frame.image.at<std::uint8_t>(0, 2), 255U);
}

TEST(Stage2BoundaryAdapterTest, DisabledTileU8BypassUsesShallowImage)
{
    cv::Mat input_image(3, 3, CV_8UC1);
    input_image.setTo(cv::Scalar{23});
    const dp1v2::TileRawView input = makeTileRawView(input_image, dp1v2::PixelFormat::U8);

    dp1v2::Stage2BoundaryWorkspace workspace{};
    workspace.tile_bypass_u8_by_task.resize(1);
    const dp1v2::Stage2BoundaryAdapter adapter;
    const dp1v2::Stage2TileSelection selection = adapter.selectTileOutput(
        0,
        input,
        tileOutcome(dp1v2::StageExecutionStatus::Disabled),
        workspace);

    EXPECT_TRUE(selection.is_bypass);
    EXPECT_EQ(selection.frame.image.data, input.image.data);
    EXPECT_TRUE(workspace.tile_bypass_u8_by_task[0].empty());
    EXPECT_EQ(selection.frame.tile_id, input.tile_id);
    EXPECT_EQ(selection.frame.origin_in_frame, input.origin_in_frame);
    EXPECT_EQ(selection.frame.valid_area, input.valid_area);
    EXPECT_EQ(selection.frame.processing_domain, dp1v2::ProcessingDomain::RawIntensity);
}

TEST(Stage2BoundaryAdapterTest, SkippedTileU16BypassScalesIntoTaskWorkspace)
{
    cv::Mat input_image(1, 2, CV_16UC1);
    input_image.at<std::uint16_t>(0, 0) = 100;
    input_image.at<std::uint16_t>(0, 1) = 1100;
    const dp1v2::TileRawView input = makeTileRawView(input_image, dp1v2::PixelFormat::U16);

    dp1v2::Stage2BoundaryWorkspace workspace{};
    workspace.tile_bypass_u8_by_task.resize(2);
    const dp1v2::Stage2BoundaryAdapter adapter;
    const dp1v2::Stage2TileSelection selection = adapter.selectTileOutput(
        1,
        input,
        tileOutcome(dp1v2::StageExecutionStatus::Skipped),
        workspace);

    EXPECT_TRUE(selection.is_bypass);
    EXPECT_EQ(selection.frame.image.data, workspace.tile_bypass_u8_by_task[1].data);
    ASSERT_EQ(selection.frame.image.type(), CV_8UC1);
    EXPECT_EQ(selection.frame.image.at<std::uint8_t>(0, 0), 0U);
    EXPECT_EQ(selection.frame.image.at<std::uint8_t>(0, 1), 255U);
    EXPECT_TRUE(workspace.tile_bypass_u8_by_task[0].empty());
}

TEST(Stage2BoundaryAdapterTest, ValidTileTaskIndexWritesOnlySelectedWorkspaceBuffer)
{
    cv::Mat input_image(1, 2, CV_16UC1);
    input_image.at<std::uint16_t>(0, 0) = 100;
    input_image.at<std::uint16_t>(0, 1) = 1100;
    const dp1v2::TileRawView input = makeTileRawView(input_image, dp1v2::PixelFormat::U16);

    dp1v2::Stage2BoundaryWorkspace workspace{};
    workspace.tile_bypass_u8_by_task.resize(3);
    const dp1v2::Stage2BoundaryAdapter adapter;
    const dp1v2::Stage2TileSelection selection = adapter.selectTileOutput(
        2,
        input,
        tileOutcome(dp1v2::StageExecutionStatus::Disabled),
        workspace);

    EXPECT_TRUE(selection.is_bypass);
    EXPECT_TRUE(workspace.tile_bypass_u8_by_task[0].empty());
    EXPECT_TRUE(workspace.tile_bypass_u8_by_task[1].empty());
    ASSERT_FALSE(workspace.tile_bypass_u8_by_task[2].empty());
    EXPECT_EQ(selection.frame.image.data, workspace.tile_bypass_u8_by_task[2].data);
    EXPECT_EQ(selection.frame.pixel_format, dp1v2::PixelFormat::U8);
    EXPECT_EQ(selection.frame.processing_domain, dp1v2::ProcessingDomain::RawIntensity);
}

TEST(Stage2BoundaryAdapterTest, FailedAndUnsupportedOutcomesThrow)
{
    cv::Mat input_image(1, 1, CV_8UC1);
    const dp1v2::CanonicalFrame frame = makeCanonicalFrame(input_image, dp1v2::PixelFormat::U8);
    const dp1v2::TileRawView tile = makeTileRawView(input_image, dp1v2::PixelFormat::U8);

    dp1v2::Stage2BoundaryWorkspace workspace{};
    workspace.tile_bypass_u8_by_task.resize(1);
    const dp1v2::Stage2BoundaryAdapter adapter;

    EXPECT_THROW(
        adapter.selectFullFrameOutput(
            frame,
            fullFrameOutcome(dp1v2::StageExecutionStatus::Failed),
            workspace),
        std::logic_error);
    EXPECT_THROW(
        adapter.selectFullFrameOutput(
            frame,
            fullFrameOutcome(dp1v2::StageExecutionStatus::Unsupported),
            workspace),
        std::logic_error);
    EXPECT_THROW(
        adapter.selectTileOutput(
            0,
            tile,
            tileOutcome(dp1v2::StageExecutionStatus::Failed),
            workspace),
        std::logic_error);
    EXPECT_THROW(
        adapter.selectTileOutput(
            0,
            tile,
            tileOutcome(dp1v2::StageExecutionStatus::Unsupported),
            workspace),
        std::logic_error);
}

TEST(Stage2BoundaryAdapterTest, TileBypassOutOfRangeTaskIndexThrowsOutOfRange)
{
    cv::Mat input_image(1, 1, CV_16UC1);
    const dp1v2::TileRawView tile = makeTileRawView(input_image, dp1v2::PixelFormat::U16);

    dp1v2::Stage2BoundaryWorkspace workspace{};
    const dp1v2::Stage2BoundaryAdapter adapter;

    EXPECT_THROW(
        adapter.selectTileOutput(
            0,
            tile,
            tileOutcome(dp1v2::StageExecutionStatus::Skipped),
            workspace),
        std::out_of_range);
}
