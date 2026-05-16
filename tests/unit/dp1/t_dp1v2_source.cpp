#include <gtest/gtest.h>

#include <chrono>
#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

#include <opencv2/imgcodecs.hpp>

#include "dp1v2/frame/frame_packet_builder.hpp"
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
