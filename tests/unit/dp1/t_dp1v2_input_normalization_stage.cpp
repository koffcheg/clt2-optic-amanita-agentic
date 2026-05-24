#include <gtest/gtest.h>

#include <cstdint>

#include <opencv2/core.hpp>

#include "dp1v2/frame/frame_context.hpp"
#include "dp1v2/stages/input_normalization_stage.hpp"

namespace {

dp1v2::PixelRange rangeU8()
{
    return dp1v2::PixelRange{.min_value = 0.0, .max_value = 255.0, .black_level = 0.0, .saturation_level = 255.0};
}

dp1v2::PixelRange rangeU16()
{
    return dp1v2::PixelRange{.min_value = 0.0, .max_value = 65535.0, .black_level = 0.0, .saturation_level = 65535.0};
}

dp1v2::StageConfig enabledStageWithVariant(const std::string& variant)
{
    return dp1v2::StageConfig{.enabled = true, .variant = variant, .level = "L0", .parameters = {}};
}

dp1v2::InputRouteConfig routeU8()
{
    return dp1v2::InputRouteConfig{
        .pixel_format = dp1v2::PixelFormat::U8,
        .bit_depth = dp1v2::InputBitDepth::Bit8,
        .pixel_range = rangeU8(),
    };
}

dp1v2::InputRouteConfig routeU16()
{
    return dp1v2::InputRouteConfig{
        .pixel_format = dp1v2::PixelFormat::U16,
        .bit_depth = dp1v2::InputBitDepth::Bit16,
        .pixel_range = rangeU16(),
    };
}

dp1v2::FramePacket makePacket(
    const cv::Mat& image,
    const dp1v2::PixelFormat pixel_format,
    const dp1v2::InputBitDepth bit_depth,
    const dp1v2::PixelRange& pixel_range)
{
    dp1v2::FramePacket packet{};
    packet.frame_id = 42;
    packet.camera_id = 7;
    packet.source_id = "stage0-source";
    packet.image = image;
    packet.pixel_format = pixel_format;
    packet.bit_depth = bit_depth;
    packet.pixel_range = pixel_range;
    packet.geometry = dp1v2::FrameGeometry{.width = image.cols, .height = image.rows};
    return packet;
}

} // namespace

TEST(InputNormalizationStageTest, Process_PassthroughVariant_RemainsPassThrough)
{
    cv::Mat image(2, 3, CV_8UC1);
    image.setTo(cv::Scalar(17));
    const dp1v2::FramePacket packet = makePacket(image, dp1v2::PixelFormat::U8, dp1v2::InputBitDepth::Bit8, rangeU8());
    dp1v2::FrameContext context = dp1v2::build_frame_context(packet, packet.camera_id);

    const dp1v2::InputNormalizationStage stage(dp1v2::InputNormalizationResolvedConfig{
        .binning_mode = dp1v2::BinningMode::None,
        .bin_factor = 1,
    });
    const auto outcome = stage.process(
        dp1v2::InputNormalizationInput{.frame = packet},
        context,
        dp1v2::InputNormalizationConfig{.input_route = routeU8(), .stage = enabledStageWithVariant("passthrough")});

    ASSERT_EQ(outcome.status, dp1v2::StageExecutionStatus::Completed);
    EXPECT_EQ(outcome.output.frame.image.data, packet.image.data);
    EXPECT_FALSE(outcome.output.frame.normalization.binned);
    EXPECT_EQ(outcome.output.frame.normalization.binning_mode, dp1v2::BinningMode::None);
}

TEST(InputNormalizationStageTest, Process_AverageBinningVariant_Completes)
{
    cv::Mat image = (cv::Mat_<std::uint8_t>(2, 2) << 1, 2, 3, 4);
    const dp1v2::FramePacket packet = makePacket(image, dp1v2::PixelFormat::U8, dp1v2::InputBitDepth::Bit8, rangeU8());
    dp1v2::FrameContext context = dp1v2::build_frame_context(packet, packet.camera_id);

    const dp1v2::InputNormalizationStage stage(dp1v2::InputNormalizationResolvedConfig{
        .binning_mode = dp1v2::BinningMode::Average,
        .bin_factor = 2,
    });
    const auto outcome = stage.process(
        dp1v2::InputNormalizationInput{.frame = packet},
        context,
        dp1v2::InputNormalizationConfig{.input_route = routeU8(), .stage = enabledStageWithVariant("average_binning")});

    ASSERT_EQ(outcome.status, dp1v2::StageExecutionStatus::Completed);
    EXPECT_TRUE(outcome.output.frame.normalization.binned);
    EXPECT_EQ(outcome.output.frame.normalization.binning_mode, dp1v2::BinningMode::Average);
    EXPECT_EQ(outcome.output.frame.image.at<std::uint8_t>(0, 0), 3);
}

TEST(InputNormalizationStageTest, Process_RejectsVariantResolvedConfigMismatch)
{
    cv::Mat image = (cv::Mat_<std::uint8_t>(2, 2) << 1, 2, 3, 4);
    const dp1v2::FramePacket packet = makePacket(image, dp1v2::PixelFormat::U8, dp1v2::InputBitDepth::Bit8, rangeU8());
    dp1v2::FrameContext context = dp1v2::build_frame_context(packet, packet.camera_id);

    const dp1v2::InputNormalizationStage stage(dp1v2::InputNormalizationResolvedConfig{
        .binning_mode = dp1v2::BinningMode::Average,
        .bin_factor = 2,
    });
    const auto outcome = stage.process(
        dp1v2::InputNormalizationInput{.frame = packet},
        context,
        dp1v2::InputNormalizationConfig{.input_route = routeU8(), .stage = enabledStageWithVariant("passthrough")});

    EXPECT_EQ(outcome.status, dp1v2::StageExecutionStatus::Unsupported);
    EXPECT_EQ(outcome.reason, "passthrough variant requires binning_mode none and kbin=1");
}

TEST(InputNormalizationStageTest, Process_AverageBinningU16_2x2_UsesRoundedMean)
{
    cv::Mat image = (cv::Mat_<std::uint16_t>(2, 2) << 1000, 1000, 1000, 1001);
    const dp1v2::FramePacket packet = makePacket(image, dp1v2::PixelFormat::U16, dp1v2::InputBitDepth::Bit16, rangeU16());
    dp1v2::FrameContext context = dp1v2::build_frame_context(packet, packet.camera_id);

    const dp1v2::InputNormalizationStage stage({.binning_mode = dp1v2::BinningMode::Average, .bin_factor = 2});
    const auto outcome = stage.process(
        {.frame = packet}, context, {.input_route = routeU16(), .stage = enabledStageWithVariant("average_binning")});

    ASSERT_EQ(outcome.status, dp1v2::StageExecutionStatus::Completed);
    EXPECT_EQ(outcome.output.frame.image.at<std::uint16_t>(0, 0), 1000);
}

TEST(InputNormalizationStageTest, Process_AverageBinningU16_4x4_UsesRoundedMean)
{
    cv::Mat image(4, 4, CV_16UC1);
    image.setTo(cv::Scalar(1000));
    image.at<std::uint16_t>(0, 0) = 1008;
    const dp1v2::FramePacket packet = makePacket(image, dp1v2::PixelFormat::U16, dp1v2::InputBitDepth::Bit16, rangeU16());
    dp1v2::FrameContext context = dp1v2::build_frame_context(packet, packet.camera_id);

    const dp1v2::InputNormalizationStage stage({.binning_mode = dp1v2::BinningMode::Average, .bin_factor = 4});
    const auto outcome = stage.process(
        {.frame = packet}, context, {.input_route = routeU16(), .stage = enabledStageWithVariant("average_binning")});

    ASSERT_EQ(outcome.status, dp1v2::StageExecutionStatus::Completed);
    EXPECT_EQ(outcome.output.frame.image.at<std::uint16_t>(0, 0), 1001);
}

TEST(InputNormalizationStageTest, Process_AverageBinningU16_MaxValueDoesNotOverflow)
{
    cv::Mat image(4, 4, CV_16UC1);
    image.setTo(cv::Scalar(65535));
    const dp1v2::FramePacket packet = makePacket(image, dp1v2::PixelFormat::U16, dp1v2::InputBitDepth::Bit16, rangeU16());
    dp1v2::FrameContext context = dp1v2::build_frame_context(packet, packet.camera_id);

    const dp1v2::InputNormalizationStage stage({.binning_mode = dp1v2::BinningMode::Average, .bin_factor = 4});
    const auto outcome = stage.process(
        {.frame = packet}, context, {.input_route = routeU16(), .stage = enabledStageWithVariant("average_binning")});

    ASSERT_EQ(outcome.status, dp1v2::StageExecutionStatus::Completed);
    EXPECT_EQ(outcome.output.frame.image.at<std::uint16_t>(0, 0), 65535);
}

TEST(InputNormalizationStageTest, Process_AverageBinningRejectsNonDivisibleGeometry)
{
    cv::Mat image(3, 4, CV_16UC1);
    image.setTo(cv::Scalar(128));
    const dp1v2::FramePacket packet = makePacket(image, dp1v2::PixelFormat::U16, dp1v2::InputBitDepth::Bit16, rangeU16());
    dp1v2::FrameContext context = dp1v2::build_frame_context(packet, packet.camera_id);

    const dp1v2::InputNormalizationStage stage({.binning_mode = dp1v2::BinningMode::Average, .bin_factor = 2});
    const auto outcome = stage.process(
        {.frame = packet}, context, {.input_route = routeU16(), .stage = enabledStageWithVariant("average_binning")});

    EXPECT_EQ(outcome.status, dp1v2::StageExecutionStatus::Unsupported);
    EXPECT_EQ(outcome.reason, "average binning requires frame geometry divisible by kbin");
}

TEST(InputNormalizationStageTest, Process_AverageBinningOutputDoesNotAliasInput)
{
    cv::Mat image(4, 4, CV_8UC1);
    image.setTo(cv::Scalar(24));
    const dp1v2::FramePacket packet = makePacket(image, dp1v2::PixelFormat::U8, dp1v2::InputBitDepth::Bit8, rangeU8());
    dp1v2::FrameContext context = dp1v2::build_frame_context(packet, packet.camera_id);

    const dp1v2::InputNormalizationStage stage({.binning_mode = dp1v2::BinningMode::Average, .bin_factor = 2});
    const auto outcome = stage.process(
        {.frame = packet}, context, {.input_route = routeU8(), .stage = enabledStageWithVariant("average_binning")});

    ASSERT_EQ(outcome.status, dp1v2::StageExecutionStatus::Completed);
    EXPECT_NE(outcome.output.frame.image.data, packet.image.data);
}

TEST(InputNormalizationStageTest, Process_RejectsInputRouteMetadataMismatch)
{
    cv::Mat image(2, 2, CV_16UC1);
    image.setTo(cv::Scalar(1024));
    const dp1v2::FramePacket packet = makePacket(image, dp1v2::PixelFormat::U16, dp1v2::InputBitDepth::Bit12, rangeU16());
    dp1v2::FrameContext context = dp1v2::build_frame_context(packet, packet.camera_id);

    auto mismatched_route = routeU16();
    mismatched_route.bit_depth = dp1v2::InputBitDepth::Bit16;
    const dp1v2::InputNormalizationStage stage({.binning_mode = dp1v2::BinningMode::None, .bin_factor = 1});
    const auto outcome = stage.process(
        {.frame = packet}, context, {.input_route = mismatched_route, .stage = enabledStageWithVariant("passthrough")});

    EXPECT_EQ(outcome.status, dp1v2::StageExecutionStatus::Unsupported);
    EXPECT_EQ(outcome.reason, "input_route bit_depth mismatch");
}

TEST(InputNormalizationStageTest, Process_RejectsInputRouteCarrierMismatch)
{
    cv::Mat image(2, 2, CV_8UC1);
    image.setTo(cv::Scalar(42));
    const dp1v2::FramePacket packet = makePacket(image, dp1v2::PixelFormat::U16, dp1v2::InputBitDepth::Bit16, rangeU16());
    dp1v2::FrameContext context = dp1v2::build_frame_context(packet, packet.camera_id);

    const dp1v2::InputNormalizationStage stage({.binning_mode = dp1v2::BinningMode::None, .bin_factor = 1});
    const auto outcome = stage.process(
        {.frame = packet}, context, {.input_route = routeU16(), .stage = enabledStageWithVariant("passthrough")});

    EXPECT_EQ(outcome.status, dp1v2::StageExecutionStatus::Unsupported);
    EXPECT_EQ(outcome.reason, "input_route carrier depth mismatch");
}
