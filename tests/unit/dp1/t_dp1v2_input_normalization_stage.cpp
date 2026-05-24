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

dp1v2::StageConfig enabledPassthroughStage()
{
    return dp1v2::StageConfig{
        .enabled = true,
        .variant = "passthrough",
        .level = "L0",
        .parameters = {},
    };
}

dp1v2::InputRouteConfig route(
    const dp1v2::PixelFormat pixel_format,
    const dp1v2::InputBitDepth bit_depth,
    const dp1v2::PixelRange pixel_range)
{
    return dp1v2::InputRouteConfig{
        .pixel_format = pixel_format,
        .bit_depth = bit_depth,
        .pixel_range = pixel_range,
    };
}

dp1v2::FramePacket makePacket(
    const cv::Mat &image,
    const dp1v2::PixelFormat pixel_format,
    const dp1v2::InputBitDepth bit_depth,
    const dp1v2::PixelRange pixel_range)
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

dp1v2::StageOutcome<dp1v2::InputNormalizationOutput> processPacket(
    const dp1v2::FramePacket &packet,
    const dp1v2::InputRouteConfig &input_route)
{
    dp1v2::FrameContext context = dp1v2::build_frame_context(packet, packet.camera_id);
    const dp1v2::InputNormalizationStage stage;
    return stage.process(
        dp1v2::InputNormalizationInput{.frame = packet},
        context,
        dp1v2::InputNormalizationConfig{.input_route = input_route, .stage = enabledPassthroughStage()});
}

} // namespace

TEST(InputNormalizationStageTest, Process_WhenU8RouteMatches_EmitsCanonicalFrame)
{
    cv::Mat image(2, 3, CV_8UC1);
    image.setTo(cv::Scalar(17));
    const dp1v2::FramePacket packet = makePacket(image, dp1v2::PixelFormat::U8, dp1v2::InputBitDepth::Bit8, rangeU8());

    const auto outcome = processPacket(packet, route(dp1v2::PixelFormat::U8, dp1v2::InputBitDepth::Bit8, rangeU8()));

    ASSERT_EQ(outcome.status, dp1v2::StageExecutionStatus::Completed);
    EXPECT_EQ(outcome.output.frame.frame_id, packet.frame_id);
    EXPECT_EQ(outcome.output.frame.pixel_format, dp1v2::PixelFormat::U8);
    EXPECT_EQ(outcome.output.frame.bit_depth, dp1v2::InputBitDepth::Bit8);
    EXPECT_EQ(outcome.output.frame.image.type(), CV_8UC1);
    EXPECT_EQ(outcome.output.frame.image.data, packet.image.data);
    EXPECT_EQ(outcome.output.frame.image_ownership, dp1v2::CanonicalPayloadOwnership::BorrowedReadOnly);
    EXPECT_EQ(outcome.output.frame.parent_artifact_id, "raw_frame");
}

TEST(InputNormalizationStageTest, Process_WhenU16RouteMatches_EmitsCanonicalFrame)
{
    cv::Mat image(2, 3, CV_16UC1);
    image.setTo(cv::Scalar(1024));
    const dp1v2::FramePacket packet = makePacket(image, dp1v2::PixelFormat::U16, dp1v2::InputBitDepth::Bit16, rangeU16());

    const auto outcome = processPacket(packet, route(dp1v2::PixelFormat::U16, dp1v2::InputBitDepth::Bit16, rangeU16()));

    ASSERT_EQ(outcome.status, dp1v2::StageExecutionStatus::Completed);
    EXPECT_EQ(outcome.output.frame.pixel_format, dp1v2::PixelFormat::U16);
    EXPECT_EQ(outcome.output.frame.bit_depth, dp1v2::InputBitDepth::Bit16);
    EXPECT_EQ(outcome.output.frame.image.type(), CV_16UC1);
    EXPECT_EQ(outcome.output.frame.geometry.width, image.cols);
    EXPECT_EQ(outcome.output.frame.geometry.height, image.rows);
}

TEST(InputNormalizationStageTest, Process_WhenPixelFormatMismatches_ReturnsUnsupported)
{
    cv::Mat image(2, 3, CV_8UC1);
    const dp1v2::FramePacket packet = makePacket(image, dp1v2::PixelFormat::U8, dp1v2::InputBitDepth::Bit8, rangeU8());

    const auto outcome = processPacket(packet, route(dp1v2::PixelFormat::U16, dp1v2::InputBitDepth::Bit16, rangeU16()));

    EXPECT_EQ(outcome.status, dp1v2::StageExecutionStatus::Unsupported);
    EXPECT_EQ(outcome.reason, "input_route pixel_format mismatch");
}

TEST(InputNormalizationStageTest, Process_WhenBitDepthMismatches_ReturnsUnsupported)
{
    cv::Mat image(2, 3, CV_16UC1);
    const dp1v2::FramePacket packet = makePacket(image, dp1v2::PixelFormat::U16, dp1v2::InputBitDepth::Bit12, rangeU16());

    const auto outcome = processPacket(packet, route(dp1v2::PixelFormat::U16, dp1v2::InputBitDepth::Bit16, rangeU16()));

    EXPECT_EQ(outcome.status, dp1v2::StageExecutionStatus::Unsupported);
    EXPECT_EQ(outcome.reason, "input_route bit_depth mismatch");
}

TEST(InputNormalizationStageTest, Process_WhenPixelRangeMismatches_ReturnsUnsupported)
{
    cv::Mat image(2, 3, CV_8UC1);
    const dp1v2::PixelRange packet_range = rangeU8();
    dp1v2::PixelRange route_range = rangeU8();
    route_range.saturation_level = 200.0;
    const dp1v2::FramePacket packet = makePacket(image, dp1v2::PixelFormat::U8, dp1v2::InputBitDepth::Bit8, packet_range);

    const auto outcome = processPacket(packet, route(dp1v2::PixelFormat::U8, dp1v2::InputBitDepth::Bit8, route_range));

    EXPECT_EQ(outcome.status, dp1v2::StageExecutionStatus::Unsupported);
    EXPECT_EQ(outcome.reason, "input_route pixel_range mismatch");
}

TEST(InputNormalizationStageTest, Process_WhenCarrierDepthMismatches_ReturnsUnsupported)
{
    cv::Mat image(2, 3, CV_8UC1);
    const dp1v2::FramePacket packet = makePacket(image, dp1v2::PixelFormat::U16, dp1v2::InputBitDepth::Bit16, rangeU16());

    const auto outcome = processPacket(packet, route(dp1v2::PixelFormat::U16, dp1v2::InputBitDepth::Bit16, rangeU16()));

    EXPECT_EQ(outcome.status, dp1v2::StageExecutionStatus::Unsupported);
    EXPECT_EQ(outcome.reason, "input_route carrier depth mismatch");
}

TEST(InputNormalizationStageTest, Process_WhenSuccessful_CanonicalFrameCanBeRegisteredAsArtifact)
{
    cv::Mat image(2, 3, CV_16UC1);
    const dp1v2::FramePacket packet = makePacket(image, dp1v2::PixelFormat::U16, dp1v2::InputBitDepth::Bit16, rangeU16());
    dp1v2::FrameContext context = dp1v2::build_frame_context(packet, packet.camera_id);
    const dp1v2::InputNormalizationStage stage;

    const auto outcome = stage.process(
        dp1v2::InputNormalizationInput{.frame = packet},
        context,
        dp1v2::InputNormalizationConfig{
            .input_route = route(dp1v2::PixelFormat::U16, dp1v2::InputBitDepth::Bit16, rangeU16()),
            .stage = enabledPassthroughStage(),
            
        });
    ASSERT_EQ(outcome.status, dp1v2::StageExecutionStatus::Completed);

    const dp1v2::FrameArtifactRef artifact = dp1v2::register_canonical_frame_artifact(context, outcome.output.frame);

    EXPECT_EQ(artifact.id, "canonical_frame");
    EXPECT_EQ(artifact.kind, dp1v2::FrameArtifactKind::CanonicalFrame);
    EXPECT_EQ(artifact.producer_stage, "input_normalization");
    EXPECT_EQ(artifact.parent_artifact_id, "raw_frame");
    EXPECT_EQ(artifact.ownership, dp1v2::FrameArtifactOwnership::BorrowedReadOnly);
}

TEST(InputNormalizationStageTest, Process_WhenSuccessful_CanonicalFrameProvenanceIsPassthroughNoBinning)
{
    cv::Mat image(2, 3, CV_16UC1);
    const dp1v2::FramePacket packet = makePacket(image, dp1v2::PixelFormat::U16, dp1v2::InputBitDepth::Bit16, rangeU16());

    const auto outcome = processPacket(packet, route(dp1v2::PixelFormat::U16, dp1v2::InputBitDepth::Bit16, rangeU16()));

    ASSERT_EQ(outcome.status, dp1v2::StageExecutionStatus::Completed);
    EXPECT_EQ(outcome.output.frame.normalization.source, dp1v2::NormalizationSource::Stage0);
    EXPECT_FALSE(outcome.output.frame.normalization.copied);
    EXPECT_FALSE(outcome.output.frame.normalization.converted);
    EXPECT_FALSE(outcome.output.frame.normalization.binned);
    EXPECT_EQ(outcome.output.frame.normalization.bin_factor_x, 1);
    EXPECT_EQ(outcome.output.frame.normalization.bin_factor_y, 1);
    EXPECT_EQ(outcome.output.frame.normalization.binning_mode, dp1v2::BinningMode::None);
}


TEST(InputNormalizationStageTest, Process_AverageKbin2_U8ValuesAndMetadata)
{
    cv::Mat image = (cv::Mat_<std::uint8_t>(2, 2) << 1, 2, 3, 4);
    auto packet = makePacket(image, dp1v2::PixelFormat::U8, dp1v2::InputBitDepth::Bit8, rangeU8());
    dp1v2::FrameContext context = dp1v2::build_frame_context(packet, packet.camera_id);
    const dp1v2::InputNormalizationStage stage;
    auto out = stage.process({.frame=packet}, context, {.input_route=route(dp1v2::PixelFormat::U8, dp1v2::InputBitDepth::Bit8, rangeU8()), .stage=enabledPassthroughStage(), });
    ASSERT_EQ(out.status, dp1v2::StageExecutionStatus::Completed);
    EXPECT_EQ(out.output.frame.image.at<std::uint8_t>(0,0), 3);
    EXPECT_TRUE(out.output.frame.normalization.binned);
    EXPECT_EQ(out.output.frame.normalization.binning_mode, dp1v2::BinningMode::Average);
}

TEST(InputNormalizationStageTest, Process_AverageKbin4_U16ValuesAndGeometry)
{
    cv::Mat image(4,4,CV_16UC1); image.setTo(cv::Scalar(16));
    auto packet = makePacket(image, dp1v2::PixelFormat::U16, dp1v2::InputBitDepth::Bit16, rangeU16());
    dp1v2::FrameContext context = dp1v2::build_frame_context(packet, packet.camera_id);
    const dp1v2::InputNormalizationStage stage;
    auto out = stage.process({.frame=packet}, context, {.input_route=route(dp1v2::PixelFormat::U16, dp1v2::InputBitDepth::Bit16, rangeU16()), .stage=enabledPassthroughStage(), });
    ASSERT_EQ(out.status, dp1v2::StageExecutionStatus::Completed);
    EXPECT_EQ(out.output.frame.geometry.width, 1);
    EXPECT_EQ(out.output.frame.geometry.height, 1);
    EXPECT_EQ(out.output.frame.image.at<std::uint16_t>(0,0), 16);
}

TEST(InputNormalizationStageTest, Process_AverageBinningRejectsNonDivisibleGeometry)
{
    cv::Mat image(3,3,CV_8UC1); image.setTo(cv::Scalar(1));
    auto packet = makePacket(image, dp1v2::PixelFormat::U8, dp1v2::InputBitDepth::Bit8, rangeU8());
    dp1v2::FrameContext context = dp1v2::build_frame_context(packet, packet.camera_id);
    const dp1v2::InputNormalizationStage stage;
    auto out = stage.process({.frame=packet}, context, {.input_route=route(dp1v2::PixelFormat::U8, dp1v2::InputBitDepth::Bit8, rangeU8()), .stage=enabledPassthroughStage(), });
    EXPECT_EQ(out.status, dp1v2::StageExecutionStatus::Unsupported);
}


TEST(InputNormalizationStageTest, Process_AverageKbin1_RemainsPassThroughNoBinning)
{
    cv::Mat image(4, 4, CV_16UC1);
    image.setTo(cv::Scalar(32));
    const dp1v2::FramePacket packet = makePacket(image, dp1v2::PixelFormat::U16, dp1v2::InputBitDepth::Bit16, rangeU16());
    dp1v2::FrameContext context = dp1v2::build_frame_context(packet, packet.camera_id);
    const dp1v2::InputNormalizationStage stage;

    const auto outcome = stage.process(
        dp1v2::InputNormalizationInput{.frame = packet},
        context,
        dp1v2::InputNormalizationConfig{
            .input_route = route(dp1v2::PixelFormat::U16, dp1v2::InputBitDepth::Bit16, rangeU16()),
            .stage = enabledPassthroughStage(),
            
        });

    ASSERT_EQ(outcome.status, dp1v2::StageExecutionStatus::Completed);
    EXPECT_EQ(outcome.output.frame.geometry.width, packet.geometry.width);
    EXPECT_EQ(outcome.output.frame.geometry.height, packet.geometry.height);
    EXPECT_EQ(outcome.output.frame.image.type(), packet.image.type());
    EXPECT_FALSE(outcome.output.frame.normalization.binned);
    EXPECT_EQ(outcome.output.frame.normalization.binning_mode, dp1v2::BinningMode::None);
}
