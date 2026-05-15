#include "dp1v2/frame/frame_packet_builder.hpp"

#include <utility>

namespace dp1v2 {
namespace {

struct PixelRouteResolution {
    PixelFormat pixel_format = PixelFormat::U16;
    InputBitDepth bit_depth = InputBitDepth::Bit16;
    PixelRouteResolvePath path = PixelRouteResolvePath::FromMatDepth;
    bool exact_bit_depth = false;
};

std::optional<InputBitDepth> input_bit_depth_from_int(const int bit_depth) {
    switch (bit_depth) {
        case 8:
            return InputBitDepth::Bit8;
        case 10:
            return InputBitDepth::Bit10;
        case 12:
            return InputBitDepth::Bit12;
        case 14:
            return InputBitDepth::Bit14;
        case 16:
            return InputBitDepth::Bit16;
        default:
            return std::nullopt;
    }
}

std::optional<PixelFormat> pixel_format_from_mat_depth(const cv::Mat &image) {
    if (image.depth() == CV_8U) {
        return PixelFormat::U8;
    }

    if (image.depth() == CV_16U) {
        return PixelFormat::U16;
    }

    return std::nullopt;
}

std::optional<PixelRouteResolution> resolve_pixel_route(const cv::Mat &image, const FrameHeaderHint &hint) {
    const auto mat_pixel_format = pixel_format_from_mat_depth(image);
    if (!mat_pixel_format.has_value()) {
        return std::nullopt;
    }

    if (hint.bit_depth.has_value()) {
        const auto bit_depth = input_bit_depth_from_int(*hint.bit_depth);
        if (!bit_depth.has_value()) {
            return std::nullopt;
        }
        return PixelRouteResolution{
            .pixel_format = *mat_pixel_format,
            .bit_depth = *bit_depth,
            .path = PixelRouteResolvePath::FromHeader,
            .exact_bit_depth = true,
        };
    }

    if (*mat_pixel_format == PixelFormat::U8) {
        return PixelRouteResolution{
            .pixel_format = PixelFormat::U8,
            .bit_depth = InputBitDepth::Bit8,
            .path = PixelRouteResolvePath::FromMatDepth,
            .exact_bit_depth = false,
        };
    }

    return PixelRouteResolution{
        .pixel_format = PixelFormat::U16,
        .bit_depth = InputBitDepth::Bit16,
        .path = PixelRouteResolvePath::FromMatDepth,
        .exact_bit_depth = false,
    };
}

bool is_header_mat_compatible(const InputBitDepth header_bit_depth, const PixelFormat mat_pixel_format) {
    if (mat_pixel_format == PixelFormat::U8) {
        return header_bit_depth == InputBitDepth::Bit8;
    }

    if (mat_pixel_format == PixelFormat::U16) {
        return header_bit_depth == InputBitDepth::Bit10 || header_bit_depth == InputBitDepth::Bit12 ||
               header_bit_depth == InputBitDepth::Bit14 || header_bit_depth == InputBitDepth::Bit16;
    }

    return false;
}

PixelRange default_pixel_range(const InputBitDepth bit_depth) {
    const int max_value = bit_depth == InputBitDepth::Bit8 ? 255 :
                          bit_depth == InputBitDepth::Bit10 ? 1023 :
                          bit_depth == InputBitDepth::Bit12 ? 4095 :
                          bit_depth == InputBitDepth::Bit14 ? 16383 : 65535;
    return PixelRange{
        .min_value = 0.0,
        .max_value = static_cast<double>(max_value),
        .black_level = 0.0,
        .saturation_level = static_cast<double>(max_value),
    };
}

} // namespace

bool FramePacketBuildResult::ok() const {
    return error == FramePacketBuildError::None;
}

const char *frame_packet_error_to_cstr(const FramePacketBuildError error) {
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

FramePacketBuildResult make_frame_packet(
    const cv::Mat &image,
    const FrameHeaderHint &hint,
    const std::chrono::steady_clock::time_point ingest_steady_ts) {
    if (image.empty()) {
        return FramePacketBuildResult{.error = FramePacketBuildError::EmptyFrame};
    }

    if (image.channels() != 1) {
        return FramePacketBuildResult{.error = FramePacketBuildError::InvalidChannelCount};
    }

    const auto mat_pixel_format = pixel_format_from_mat_depth(image);
    if (!mat_pixel_format.has_value()) {
        return FramePacketBuildResult{.error = FramePacketBuildError::UnsupportedMatDepth};
    }

    if (hint.bit_depth.has_value()) {
        const auto header_bit_depth = input_bit_depth_from_int(*hint.bit_depth);
        if (!header_bit_depth.has_value()) {
            return FramePacketBuildResult{.error = FramePacketBuildError::UnsupportedBitDepthHint};
        }

        if (!is_header_mat_compatible(*header_bit_depth, *mat_pixel_format)) {
            return FramePacketBuildResult{.error = FramePacketBuildError::HeaderMatConflict};
        }
    }

    const auto pixel_route = resolve_pixel_route(image, hint);
    if (!pixel_route.has_value()) {
        return FramePacketBuildResult{.error = FramePacketBuildError::UnsupportedMatDepth};
    }

    FramePacket packet{};
    packet.frame_id = hint.frame_id.value_or(0);
    packet.camera_id = hint.camera_id.value_or(-1);
    packet.image = image;
    packet.pixel_format = pixel_route->pixel_format;
    packet.bit_depth = pixel_route->bit_depth;
    packet.pixel_range = default_pixel_range(pixel_route->bit_depth);
    packet.geometry = FrameGeometry{.width = image.cols, .height = image.rows};
    packet.ingest_time = TimestampRef{
        .clock = TimestampClock::Steady,
        .steady_time = ingest_steady_ts,
        .source = "dp1_ingest",
        .valid = true,
    };
    if (hint.exposure_start.has_value()) {
        packet.acquisition_time = TimestampRef{
            .clock = TimestampClock::Camera,
            .system_time = *hint.exposure_start,
            .source = "frame_header.exposure_start",
            .valid = true,
        };
    }

    return FramePacketBuildResult{
        .packet = std::move(packet),
        .error = FramePacketBuildError::None,
        .resolve_path = pixel_route->path,
        .exact_bit_depth = pixel_route->exact_bit_depth,
    };
}

} // namespace dp1v2
