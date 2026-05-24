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

dp1v2::StageConfig enabledStageWithVariant(const std::string& variant)
{
    return dp1v2::StageConfig{
        .enabled = true,
        .variant = variant,
        .level = "L0",
        .parameters = {},
    };
}

dp1v2::InputRouteConfig routeU8()
{
    return dp1v2::InputRouteConfig{
        .pixel_format = dp1v2::PixelFormat::U8,
        .bit_depth = dp1v2::InputBitDepth::Bit8,
        .pixel_range = rangeU8(),
    };
}

dp1v2::FramePacket makePacket(const cv::Mat& image)
{
    dp1v2::FramePacket packet{};
    packet.frame_id = 42;
    packet.camera_id = 7;
    packet.source_id = "stage0-source";
    packet.image = image;
    packet.pixel_format = dp1v2::PixelFormat::U8;
    packet.bit_depth = dp1v2::InputBitDepth::Bit8;
    packet.pixel_range = rangeU8();
    packet.geometry = dp1v2::FrameGeometry{.width = image.cols, .height = image.rows};
    return packet;
}

} // namespace

TEST(InputNormalizationStageTest, Process_PassthroughVariant_RemainsPassThrough)
{
    cv::Mat image(2, 3, CV_8UC1);
    image.setTo(cv::Scalar(17));
    const dp1v2::FramePacket packet = makePacket(image);
    dp1v2::FrameContext context = dp1v2::build_frame_context(packet, packet.camera_id);

    const dp1v2::InputNormalizationStage stage(dp1v2::InputNormalizationResolvedConfig{
        .binning_mode = dp1v2::BinningMode::None,
        .bin_factor = 1,
    });
    const auto outcome = stage.process(
        dp1v2::InputNormalizationInput{.frame = packet},
        context,
        dp1v2::InputNormalizationConfig{
            .input_route = routeU8(),
            .stage = enabledStageWithVariant("passthrough"),
        });

    ASSERT_EQ(outcome.status, dp1v2::StageExecutionStatus::Completed);
    EXPECT_EQ(outcome.output.frame.image.data, packet.image.data);
    EXPECT_FALSE(outcome.output.frame.normalization.binned);
    EXPECT_EQ(outcome.output.frame.normalization.binning_mode, dp1v2::BinningMode::None);
}

TEST(InputNormalizationStageTest, Process_AverageBinningVariant_Completes)
{
    cv::Mat image = (cv::Mat_<std::uint8_t>(2, 2) << 1, 2, 3, 4);
    const dp1v2::FramePacket packet = makePacket(image);
    dp1v2::FrameContext context = dp1v2::build_frame_context(packet, packet.camera_id);

    const dp1v2::InputNormalizationStage stage(dp1v2::InputNormalizationResolvedConfig{
        .binning_mode = dp1v2::BinningMode::Average,
        .bin_factor = 2,
    });
    const auto outcome = stage.process(
        dp1v2::InputNormalizationInput{.frame = packet},
        context,
        dp1v2::InputNormalizationConfig{
            .input_route = routeU8(),
            .stage = enabledStageWithVariant("average_binning"),
        });

    ASSERT_EQ(outcome.status, dp1v2::StageExecutionStatus::Completed);
    EXPECT_TRUE(outcome.output.frame.normalization.binned);
    EXPECT_EQ(outcome.output.frame.normalization.binning_mode, dp1v2::BinningMode::Average);
    EXPECT_EQ(outcome.output.frame.image.at<std::uint8_t>(0, 0), 3);
}

TEST(InputNormalizationStageTest, Process_RejectsVariantResolvedConfigMismatch)
{
    cv::Mat image = (cv::Mat_<std::uint8_t>(2, 2) << 1, 2, 3, 4);
    const dp1v2::FramePacket packet = makePacket(image);
    dp1v2::FrameContext context = dp1v2::build_frame_context(packet, packet.camera_id);

    const dp1v2::InputNormalizationStage stage(dp1v2::InputNormalizationResolvedConfig{
        .binning_mode = dp1v2::BinningMode::Average,
        .bin_factor = 2,
    });
    const auto outcome = stage.process(
        dp1v2::InputNormalizationInput{.frame = packet},
        context,
        dp1v2::InputNormalizationConfig{
            .input_route = routeU8(),
            .stage = enabledStageWithVariant("passthrough"),
        });

    EXPECT_EQ(outcome.status, dp1v2::StageExecutionStatus::Unsupported);
    EXPECT_EQ(outcome.reason, "passthrough variant requires binning_mode none and kbin=1");
}
