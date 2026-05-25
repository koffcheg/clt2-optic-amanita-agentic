#include <gtest/gtest.h>

#include <chrono>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

#include <opencv2/imgcodecs.hpp>

#include "dp1v2/frame/frame_packet_builder.hpp"
#include "dp1v2/source/source_factory.hpp"
#include "dp1v2/source/uri_file_source.hpp"

namespace {

class SourceTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        const auto unique = std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
        temp_dir_ = std::filesystem::temp_directory_path() / ("dp1v2_source_test_" + unique);
        std::filesystem::create_directories(temp_dir_);
    }

    void TearDown() override
    {
        std::error_code ec;
        std::filesystem::remove_all(temp_dir_, ec);
    }

    std::filesystem::path temp_dir_;
};

cv::Mat makeU8Image()
{
    cv::Mat image(2, 2, CV_8UC1);
    image.at<std::uint8_t>(0, 0) = 1;
    image.at<std::uint8_t>(0, 1) = 2;
    image.at<std::uint8_t>(1, 0) = 3;
    image.at<std::uint8_t>(1, 1) = 4;
    return image;
}

cv::Mat makeU16Image()
{
    cv::Mat image(2, 2, CV_16UC1);
    image.at<std::uint16_t>(0, 0) = 17;
    image.at<std::uint16_t>(0, 1) = 1024;
    image.at<std::uint16_t>(1, 0) = 4096;
    image.at<std::uint16_t>(1, 1) = 65535;
    return image;
}

dp1v2::InputRouteConfig u16InputRoute()
{
    dp1v2::InputRouteConfig route{};
    route.pixel_format = dp1v2::PixelFormat::U16;
    route.bit_depth = dp1v2::InputBitDepth::Bit16;
    route.pixel_range = dp1v2::PixelRange{.min_value = 0.0, .max_value = 65535.0, .black_level = 0.0, .saturation_level = 65535.0};
    return route;
}

} // namespace

TEST_F(SourceTest, UriFileFrameSource_WhenU8ImageRead_PreservesBitDepthAndStartsFrameIdAtZero)
{
    const std::filesystem::path path = temp_dir_ / "frame.png";
    ASSERT_TRUE(cv::imwrite(path.string(), makeU8Image()));

    dp1v2::UriFileFrameSource source(path.string());
    ASSERT_TRUE(source.is_open());

    const dp1v2::SourceReadResult result = source.read_next();

    ASSERT_EQ(result.status, dp1v2::SourceReadStatus::FrameReady) << result.reason;
    ASSERT_TRUE(result.envelope.header_hint.frame_id.has_value());
    ASSERT_TRUE(result.envelope.header_hint.bit_depth.has_value());
    EXPECT_EQ(*result.envelope.header_hint.frame_id, 0U);
    EXPECT_EQ(*result.envelope.header_hint.bit_depth, 8);
    EXPECT_EQ(result.envelope.frame.type(), CV_8UC1);
}


TEST_F(SourceTest, UriFileFrameSource_WhenU16ImageRead_PreservesCV16UC1)
{
    const std::filesystem::path path = temp_dir_ / "frame.tiff";
    ASSERT_TRUE(cv::imwrite(path.string(), makeU16Image()));

    dp1v2::UriFileFrameSource source(path.string());
    ASSERT_TRUE(source.is_open());

    const dp1v2::SourceReadResult result = source.read_next();

    ASSERT_EQ(result.status, dp1v2::SourceReadStatus::FrameReady) << result.reason;
    ASSERT_TRUE(result.envelope.header_hint.frame_id.has_value());
    ASSERT_TRUE(result.envelope.header_hint.bit_depth.has_value());
    EXPECT_EQ(*result.envelope.header_hint.frame_id, 0U);
    EXPECT_EQ(*result.envelope.header_hint.bit_depth, 16);
    ASSERT_EQ(result.envelope.frame.type(), CV_16UC1);
    EXPECT_EQ(result.envelope.frame.depth(), CV_16U);
    EXPECT_EQ(result.envelope.frame.channels(), 1);
    EXPECT_EQ(result.envelope.frame.at<std::uint16_t>(1, 1), 65535);
}


TEST_F(SourceTest, UriFileFrameSource_WhenPrintfImageSequenceRead_UsesZeroBasedImreadUnchanged)
{
    const std::filesystem::path frame0 = temp_dir_ / "C001_F000000.tiff";
    const std::filesystem::path frame1 = temp_dir_ / "C001_F000001.tiff";
    cv::Mat image0 = makeU16Image();
    cv::Mat image1 = makeU16Image();
    image1.at<std::uint16_t>(0, 0) = 12345;
    ASSERT_TRUE(cv::imwrite(frame0.string(), image0));
    ASSERT_TRUE(cv::imwrite(frame1.string(), image1));

    const std::filesystem::path pattern = temp_dir_ / "C001_F%06d.tiff";
    dp1v2::UriFileFrameSource source(pattern.string());
    ASSERT_TRUE(source.is_open());

    const dp1v2::SourceReadResult first = source.read_next();
    const dp1v2::SourceReadResult second = source.read_next();

    ASSERT_EQ(first.status, dp1v2::SourceReadStatus::FrameReady) << first.reason;
    ASSERT_EQ(second.status, dp1v2::SourceReadStatus::FrameReady) << second.reason;
    ASSERT_TRUE(first.envelope.header_hint.frame_id.has_value());
    ASSERT_TRUE(second.envelope.header_hint.frame_id.has_value());
    EXPECT_EQ(*first.envelope.header_hint.frame_id, 0U);
    EXPECT_EQ(*second.envelope.header_hint.frame_id, 1U);
    EXPECT_EQ(first.envelope.frame.type(), CV_16UC1);
    EXPECT_EQ(second.envelope.frame.type(), CV_16UC1);
    EXPECT_EQ(first.envelope.frame.at<std::uint16_t>(0, 0), 17);
    EXPECT_EQ(second.envelope.frame.at<std::uint16_t>(0, 0), 12345);
}

TEST_F(SourceTest, UriFileFrameSource_WhenLogicalCameraIdProvided_EmitsHeaderHintCameraId)
{
    const std::filesystem::path path = temp_dir_ / "frame.tiff";
    ASSERT_TRUE(cv::imwrite(path.string(), makeU16Image()));

    dp1v2::UriFileFrameSource source(path.string(), 0);
    ASSERT_TRUE(source.is_open());

    const dp1v2::SourceReadResult result = source.read_next();

    ASSERT_EQ(result.status, dp1v2::SourceReadStatus::FrameReady) << result.reason;
    ASSERT_TRUE(result.envelope.header_hint.camera_id.has_value());
    EXPECT_EQ(*result.envelope.header_hint.camera_id, 0);
}

TEST_F(SourceTest, CreateFrameSource_WhenFileSourceUsesCamIndex_EmitsHeaderHintCameraId)
{
    const std::filesystem::path path = temp_dir_ / "frame.tiff";
    ASSERT_TRUE(cv::imwrite(path.string(), makeU16Image()));

    dp1v2::SourceConfig config{};
    config.mode = dp1v2::FrameSourceMode::File;
    config.file.path = path.string();

    std::unique_ptr<dp1v2::IFrameSource> source = dp1v2::create_frame_source(config, 0);
    ASSERT_NE(source, nullptr);

    const dp1v2::SourceReadResult result = source->read_next();

    ASSERT_EQ(result.status, dp1v2::SourceReadStatus::FrameReady) << result.reason;
    ASSERT_TRUE(result.envelope.header_hint.camera_id.has_value());
    EXPECT_EQ(*result.envelope.header_hint.camera_id, 0);
}

TEST_F(SourceTest, MakeFramePacket_AfterFileSourceRead_UsesLogicalCameraId)
{
    const std::filesystem::path path = temp_dir_ / "frame.tiff";
    ASSERT_TRUE(cv::imwrite(path.string(), makeU16Image()));

    dp1v2::UriFileFrameSource source(path.string(), 0);
    ASSERT_TRUE(source.is_open());

    const dp1v2::SourceReadResult read_result = source.read_next();
    ASSERT_EQ(read_result.status, dp1v2::SourceReadStatus::FrameReady) << read_result.reason;

    const dp1v2::FramePacketBuildResult packet_result = dp1v2::make_frame_packet(
        read_result.envelope.frame,
        read_result.envelope.header_hint,
        u16InputRoute(),
        read_result.envelope.received_steady_ts);

    ASSERT_TRUE(packet_result.ok()) << dp1v2::frame_packet_error_to_cstr(packet_result.error);
    EXPECT_EQ(packet_result.packet.camera_id, 0);
}

TEST_F(SourceTest, MakeFramePacket_WhenU8FrameUsesU16InputRoute_ReturnsHeaderMatConflict)
{
    dp1v2::FrameHeaderHint hint{};
    hint.bit_depth = 8;

    const dp1v2::FramePacketBuildResult result = dp1v2::make_frame_packet(
        makeU8Image(),
        hint,
        u16InputRoute(),
        std::chrono::steady_clock::now());

    EXPECT_EQ(result.error, dp1v2::FramePacketBuildError::HeaderMatConflict);
}
