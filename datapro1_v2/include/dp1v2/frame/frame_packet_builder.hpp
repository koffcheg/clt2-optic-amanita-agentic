#pragma once

#include <chrono>
#include <cstdint>
#include <optional>

#include <opencv2/core.hpp>

#include "dp1v2/config/config.hpp"
#include "dp1v2/domain/frame_packet.hpp"

namespace dp1v2 {

enum class PixelRouteResolvePath {
    FromHeader,
    FromMatDepth,
};

enum class FramePacketBuildError {
    None,
    EmptyFrame,
    InvalidChannelCount,
    UnsupportedBitDepthHint,
    UnsupportedMatDepth,
    HeaderMatConflict,
};

struct FrameHeaderHint {
    std::optional<int> bit_depth;
    std::optional<int> bytes_per_pixel;
    std::optional<std::uint64_t> frame_id;
    std::optional<int> camera_id;
    std::optional<std::chrono::system_clock::time_point> exposure_start;
    std::optional<float> exposure_length_sec;
};

struct FramePacketBuildResult {
    FramePacket packet{};
    FramePacketBuildError error = FramePacketBuildError::None;
    PixelRouteResolvePath resolve_path = PixelRouteResolvePath::FromMatDepth;
    bool exact_bit_depth = false;

    [[nodiscard]] bool ok() const;
};

const char *frame_packet_error_to_cstr(FramePacketBuildError error);

FramePacketBuildResult make_frame_packet(
    const cv::Mat &image,
    const FrameHeaderHint &hint,
    std::chrono::steady_clock::time_point ingest_steady_ts);

FramePacketBuildResult make_frame_packet(
    const cv::Mat &image,
    const FrameHeaderHint &hint,
    const InputRouteConfig &input_route,
    std::chrono::steady_clock::time_point ingest_steady_ts);

} // namespace dp1v2
