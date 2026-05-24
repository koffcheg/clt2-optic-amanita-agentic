#include <gtest/gtest.h>

#include <cstdint>

#include <opencv2/core.hpp>

#include "dp1v2/frame/frame_context.hpp"
#include "dp1v2/stages/input_normalization_stage.hpp"

namespace {

dp1v2::StageConfig averageBinningStageConfig()
{
    return dp1v2::StageConfig{.enabled = true, .variant = "average_binning", .level = "L0", .parameters = {}};
}

dp1v2::InputRouteConfig routeU8()
{
    return dp1v2::InputRouteConfig{
        .pixel_format = dp1v2::PixelFormat::U8,
        .bit_depth = dp1v2::InputBitDepth::Bit8,
        .pixel_range = dp1v2::PixelRange{.min_value = 0.0, .max_value = 255.0, .black_level = 0.0, .saturation_level = 255.0},
    };
}

dp1v2::InputRouteConfig routeU16()
{
    return dp1v2::InputRouteConfig{
        .pixel_format = dp1v2::PixelFormat::U16,
        .bit_depth = dp1v2::InputBitDepth::Bit16,
        .pixel_range = dp1v2::PixelRange{.min_value = 0.0, .max_value = 65535.0, .black_level = 0.0, .saturation_level = 65535.0},
    };
}

dp1v2::FramePacket makePacket(
    const cv::Mat &image,
    const dp1v2::PixelFormat pixel_format,
    const dp1v2::InputBitDepth bit_depth,
    const dp1v2::PixelRange &pixel_range)
{
    dp1v2::FramePacket packet{};
    packet.frame_id = 100;
    packet.camera_id = 3;
    packet.source_id = "visual-unit";
    packet.image = image;
    packet.pixel_format = pixel_format;
    packet.bit_depth = bit_depth;
    packet.pixel_range = pixel_range;
    packet.geometry = dp1v2::FrameGeometry{.width = image.cols, .height = image.rows};
    return packet;
}

} // namespace

TEST(AverageBinningVisualUnitTest, U8_2x2_RoundedValue)
{
    const cv::Mat image = (cv::Mat_<std::uint8_t>(2, 2) << 1, 2, 3, 4);
    const auto packet = makePacket(image, dp1v2::PixelFormat::U8, dp1v2::InputBitDepth::Bit8, routeU8().pixel_range);
    auto context = dp1v2::build_frame_context(packet, packet.camera_id);

    const dp1v2::InputNormalizationStage stage({.binning_mode = dp1v2::BinningMode::Average, .bin_factor = 2});
    const auto outcome = stage.process({.frame = packet}, context, {.input_route = routeU8(), .stage = averageBinningStageConfig()});

    ASSERT_EQ(outcome.status, dp1v2::StageExecutionStatus::Completed);
    EXPECT_EQ(outcome.output.frame.image.at<std::uint8_t>(0, 0), 3);
}

TEST(AverageBinningVisualUnitTest, U16_2x2_RoundedValue)
{
    const cv::Mat image = (cv::Mat_<std::uint16_t>(2, 2) << 10, 10, 10, 11);
    const auto packet = makePacket(image, dp1v2::PixelFormat::U16, dp1v2::InputBitDepth::Bit16, routeU16().pixel_range);
    auto context = dp1v2::build_frame_context(packet, packet.camera_id);

    const dp1v2::InputNormalizationStage stage({.binning_mode = dp1v2::BinningMode::Average, .bin_factor = 2});
    const auto outcome = stage.process({.frame = packet}, context, {.input_route = routeU16(), .stage = averageBinningStageConfig()});

    ASSERT_EQ(outcome.status, dp1v2::StageExecutionStatus::Completed);
    EXPECT_EQ(outcome.output.frame.image.at<std::uint16_t>(0, 0), 10);
}

TEST(AverageBinningVisualUnitTest, U16_4x4_RoundedValue)
{
    cv::Mat image(4, 4, CV_16UC1);
    image.setTo(cv::Scalar(1000));
    image.at<std::uint16_t>(0, 0) = 1008;
    const auto packet = makePacket(image, dp1v2::PixelFormat::U16, dp1v2::InputBitDepth::Bit16, routeU16().pixel_range);
    auto context = dp1v2::build_frame_context(packet, packet.camera_id);

    const dp1v2::InputNormalizationStage stage({.binning_mode = dp1v2::BinningMode::Average, .bin_factor = 4});
    const auto outcome = stage.process({.frame = packet}, context, {.input_route = routeU16(), .stage = averageBinningStageConfig()});

    ASSERT_EQ(outcome.status, dp1v2::StageExecutionStatus::Completed);
    EXPECT_EQ(outcome.output.frame.image.at<std::uint16_t>(0, 0), 1001);
}

TEST(AverageBinningVisualUnitTest, U16_MaxValue_OverflowSafety)
{
    cv::Mat image(4, 4, CV_16UC1);
    image.setTo(cv::Scalar(65535));
    const auto packet = makePacket(image, dp1v2::PixelFormat::U16, dp1v2::InputBitDepth::Bit16, routeU16().pixel_range);
    auto context = dp1v2::build_frame_context(packet, packet.camera_id);

    const dp1v2::InputNormalizationStage stage({.binning_mode = dp1v2::BinningMode::Average, .bin_factor = 4});
    const auto outcome = stage.process({.frame = packet}, context, {.input_route = routeU16(), .stage = averageBinningStageConfig()});

    ASSERT_EQ(outcome.status, dp1v2::StageExecutionStatus::Completed);
    EXPECT_EQ(outcome.output.frame.image.at<std::uint16_t>(0, 0), 65535);
}
