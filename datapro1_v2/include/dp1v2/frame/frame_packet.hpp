#pragma once

#include <chrono>
#include <cstdint>
#include <optional>

#include <opencv2/core.hpp>

namespace dp1v2 {

enum class PixelType {
    MONO8,
    MONO10,
    MONO12,
    MONO14,
    MONO16,
};

enum class PixelTypeResolvePath {
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

struct PixelTypeResolution {
    PixelType pixel_type = PixelType::MONO16;
    PixelTypeResolvePath path = PixelTypeResolvePath::FromMatDepth;
    bool exact_bit_depth = false;
};

struct FramePacket {
    cv::Mat frame;
    PixelType pixel_type = PixelType::MONO16;
    std::uint64_t frame_id = 0;
    int camera_id = -1;
    std::chrono::steady_clock::time_point ingest_steady_ts{};
    std::optional<std::chrono::system_clock::time_point> exposure_start;
    std::optional<float> exposure_length_sec;
};

struct FramePacketBuildResult {
    FramePacket packet{};
    FramePacketBuildError error = FramePacketBuildError::None;
    PixelTypeResolvePath resolve_path = PixelTypeResolvePath::FromMatDepth;
    bool exact_bit_depth = false;

    [[nodiscard]] bool ok() const {
        return error == FramePacketBuildError::None;
    }
};

inline std::optional<PixelType> pixel_type_from_bit_depth(const int bit_depth) {
    switch (bit_depth) {
        case 8:
            return PixelType::MONO8;
        case 10:
            return PixelType::MONO10;
        case 12:
            return PixelType::MONO12;
        case 14:
            return PixelType::MONO14;
        case 16:
            return PixelType::MONO16;
        default:
            return std::nullopt;
    }
}

inline std::optional<PixelType> pixel_type_from_mat_depth(const cv::Mat &frame) {
    if (frame.depth() == CV_8U)
        return PixelType::MONO8;

    if (frame.depth() == CV_16U)
        return PixelType::MONO16;

    return std::nullopt;
}

inline std::optional<PixelTypeResolution> resolve_pixel_type(const cv::Mat &frame, const FrameHeaderHint &hint) {
    if (hint.bit_depth.has_value()) {
        const auto resolved = pixel_type_from_bit_depth(*hint.bit_depth);
        if (resolved.has_value()) {
            return PixelTypeResolution{.pixel_type = *resolved, .path = PixelTypeResolvePath::FromHeader, .exact_bit_depth = true};
        }
    }

    const auto resolved_from_mat = pixel_type_from_mat_depth(frame);
    if (!resolved_from_mat.has_value())
        return std::nullopt;

    return PixelTypeResolution{.pixel_type = *resolved_from_mat, .path = PixelTypeResolvePath::FromMatDepth, .exact_bit_depth = false};
}

inline bool is_header_mat_compatible(const PixelType header_pixel_type, const PixelType mat_depth_type) {
    if (mat_depth_type == PixelType::MONO8)
        return header_pixel_type == PixelType::MONO8;

    if (mat_depth_type == PixelType::MONO16) {
        return header_pixel_type == PixelType::MONO10
               || header_pixel_type == PixelType::MONO12
               || header_pixel_type == PixelType::MONO14
               || header_pixel_type == PixelType::MONO16;
    }

    return false;
}

inline const char *frame_packet_error_to_cstr(const FramePacketBuildError error) {
    switch (error) {
        case FramePacketBuildError::None:
            return "none";
        case FramePacketBuildError::EmptyFrame:
            return "empty_frame";
        case FramePacketBuildError::InvalidChannelCount:
            return "invalid_channel_count";
        case FramePacketBuildError::UnsupportedBitDepthHint:
            return "unsupported_bit_depth_hint";
        case FramePacketBuildError::UnsupportedMatDepth:
            return "unsupported_mat_depth";
        case FramePacketBuildError::HeaderMatConflict:
            return "header_mat_conflict";
        default:
            return "unknown";
    }
}

inline FramePacketBuildResult make_frame_packet(const cv::Mat &frame,
                                                const FrameHeaderHint &hint,
                                                const std::chrono::steady_clock::time_point ingest_steady_ts) {
    if (frame.empty()) {
        return FramePacketBuildResult{.error = FramePacketBuildError::EmptyFrame};
    }

    if (frame.channels() != 1) {
        return FramePacketBuildResult{.error = FramePacketBuildError::InvalidChannelCount};
    }

    const auto mat_depth_type = pixel_type_from_mat_depth(frame);
    if (!mat_depth_type.has_value()) {
        return FramePacketBuildResult{.error = FramePacketBuildError::UnsupportedMatDepth};
    }

    if (hint.bit_depth.has_value()) {
        const auto header_pixel_type = pixel_type_from_bit_depth(*hint.bit_depth);
        if (!header_pixel_type.has_value()) {
            return FramePacketBuildResult{.error = FramePacketBuildError::UnsupportedBitDepthHint};
        }

        if (!is_header_mat_compatible(*header_pixel_type, *mat_depth_type)) {
            return FramePacketBuildResult{.error = FramePacketBuildError::HeaderMatConflict};
        }
    }

    const auto pixel_type_resolution = resolve_pixel_type(frame, hint);
    if (!pixel_type_resolution.has_value()) {
        return FramePacketBuildResult{.error = FramePacketBuildError::UnsupportedMatDepth};
    }

    FramePacket packet{};
    packet.frame = frame;
    packet.pixel_type = pixel_type_resolution->pixel_type;
    packet.frame_id = hint.frame_id.value_or(0);
    packet.camera_id = hint.camera_id.value_or(-1);
    packet.ingest_steady_ts = ingest_steady_ts;
    packet.exposure_start = hint.exposure_start;
    packet.exposure_length_sec = hint.exposure_length_sec;

    return FramePacketBuildResult{
        .packet = std::move(packet),
        .error = FramePacketBuildError::None,
        .resolve_path = pixel_type_resolution->path,
        .exact_bit_depth = pixel_type_resolution->exact_bit_depth,
    };
};

} // namespace dp1v2
