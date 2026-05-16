#include "dp1v2/source/uri_file_source.hpp"

#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <limits>
#include <string>
#include <utility>
#include <vector>

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

namespace {

constexpr int kU8BitDepth = 8;
constexpr int kU16BitDepth = 16;
constexpr int kU8BytesPerPixel = 1;
constexpr int kU16BytesPerPixel = 2;

std::string expand_environment_placeholders(const std::string &path) {
    std::string expanded;
    expanded.reserve(path.size());

    for (std::size_t pos = 0; pos < path.size();) {
        if (path[pos] == '$' && pos + 1 < path.size() && path[pos + 1] == '{') {
            const auto end = path.find('}', pos + 2);
            if (end != std::string::npos) {
                const auto name = path.substr(pos + 2, end - pos - 2);
                if (const char *value = std::getenv(name.c_str())) {
                    expanded += value;
                } else {
                    expanded += path.substr(pos, end - pos + 1);
                }
                pos = end + 1;
                continue;
            }
        }

        expanded += path[pos];
        ++pos;
    }

    return expanded;
}

std::string lower_copy(std::string value) {
    for (char &c : value) {
        if (c >= 'A' && c <= 'Z') {
            c = static_cast<char>(c - 'A' + 'a');
        }
    }
    return value;
}

bool has_printf_integer_placeholder(const std::string &path) {
    for (std::size_t pos = 0; pos < path.size(); ++pos) {
        if (path[pos] != '%') {
            continue;
        }
        if (pos + 1 < path.size() && path[pos + 1] == '%') {
            ++pos;
            continue;
        }

        std::size_t spec = pos + 1;
        if (spec < path.size() && path[spec] == '0') {
            ++spec;
        }
        while (spec < path.size() && path[spec] >= '0' && path[spec] <= '9') {
            ++spec;
        }
        if (spec < path.size() && (path[spec] == 'd' || path[spec] == 'i' || path[spec] == 'u')) {
            return true;
        }
    }
    return false;
}

bool has_image_extension(const std::string &path) {
    const std::string extension = lower_copy(std::filesystem::path(path).extension().string());
    return extension == ".tif" || extension == ".tiff" || extension == ".png" || extension == ".bmp" ||
           extension == ".pgm" || extension == ".ppm" || extension == ".jpg" || extension == ".jpeg";
}

std::string format_sequence_path(const std::string &pattern, const std::uint64_t frame_id) {
    if (frame_id > static_cast<std::uint64_t>(std::numeric_limits<int>::max())) {
        return {};
    }

    const int frame_index = static_cast<int>(frame_id);
    const int size = std::snprintf(nullptr, 0, pattern.c_str(), frame_index);
    if (size <= 0) {
        return {};
    }

    std::vector<char> buffer(static_cast<std::size_t>(size) + 1U, '\0');
    std::snprintf(buffer.data(), buffer.size(), pattern.c_str(), frame_index);
    return std::string(buffer.data(), static_cast<std::size_t>(size));
}

cv::Mat normalize_video_frame_to_mono(const cv::Mat &frame) {
    cv::Mat gray;
    if (frame.channels() == 1) {
        gray = frame;
    } else if (frame.channels() == 3) {
        cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
    } else if (frame.channels() == 4) {
        cv::cvtColor(frame, gray, cv::COLOR_BGRA2GRAY);
    } else {
        return {};
    }

    if (gray.depth() == CV_8U || gray.depth() == CV_16U) {
        return gray;
    }

    return {};
}

cv::Mat preserve_image_frame_if_supported_mono(const cv::Mat &frame) {
    if (frame.channels() != 1) {
        return {};
    }
    if (frame.depth() == CV_8U || frame.depth() == CV_16U) {
        return frame;
    }
    return {};
}

int bit_depth_for_frame(const cv::Mat &frame) {
    if (frame.depth() == CV_8U) {
        return kU8BitDepth;
    }
    if (frame.depth() == CV_16U) {
        return kU16BitDepth;
    }
    return 0;
}

int bytes_per_pixel_for_frame(const cv::Mat &frame) {
    if (frame.depth() == CV_8U) {
        return kU8BytesPerPixel;
    }
    if (frame.depth() == CV_16U) {
        return kU16BytesPerPixel;
    }
    return 0;
}

} // namespace

namespace dp1v2 {

UriFileFrameSource::UriFileFrameSource(std::string link)
    : link_(expand_environment_placeholders(std::move(link))) {
    const bool image_sequence = has_printf_integer_placeholder(link_) && has_image_extension(link_);
    if (image_sequence) {
        source_kind_ = SourceKind::ImageSequence;
        return;
    }

    if (has_image_extension(link_)) {
        source_kind_ = SourceKind::StillImage;
        return;
    }

    source_kind_ = SourceKind::Video;
    if (!link_.empty()) {
        capture_.open(link_);
    }
}

bool UriFileFrameSource::is_open() const {
    if (link_.empty()) {
        return false;
    }
    if (source_kind_ == SourceKind::StillImage) {
        return std::filesystem::exists(link_);
    }
    if (source_kind_ == SourceKind::ImageSequence) {
        const std::string first_path = format_sequence_path(link_, 0);
        return !first_path.empty() && std::filesystem::exists(first_path);
    }
    return capture_.isOpened();
}

SourceReadResult UriFileFrameSource::read_next() {
    if (link_.empty()) {
        return SourceReadResult{.status = SourceReadStatus::Failed, .reason = "source_link_empty"};
    }

    cv::Mat frame;
    if (source_kind_ == SourceKind::StillImage) {
        if (next_frame_id_ > 0) {
            return SourceReadResult{.status = SourceReadStatus::SourceExhausted, .reason = "source_exhausted"};
        }
        frame = cv::imread(link_, cv::IMREAD_UNCHANGED);
    } else if (source_kind_ == SourceKind::ImageSequence) {
        const std::string frame_path = format_sequence_path(link_, next_frame_id_);
        if (frame_path.empty()) {
            return SourceReadResult{.status = SourceReadStatus::Failed, .reason = "source_sequence_format_failed"};
        }
        frame = cv::imread(frame_path, cv::IMREAD_UNCHANGED);
    } else {
        if (!capture_.isOpened()) {
            return SourceReadResult{.status = SourceReadStatus::Failed, .reason = "source_not_open"};
        }
        capture_ >> frame;
    }

    if (frame.empty()) {
        return SourceReadResult{.status = SourceReadStatus::SourceExhausted, .reason = "source_exhausted"};
    }

    cv::Mat normalized;
    if (source_kind_ == SourceKind::Video) {
        normalized = normalize_video_frame_to_mono(frame);
    } else {
        normalized = preserve_image_frame_if_supported_mono(frame);
    }
    if (normalized.empty()) {
        return SourceReadResult{.status = SourceReadStatus::Failed, .reason = "unsupported_frame_format"};
    }

    RawFrameEnvelope envelope{};
    envelope.frame = normalized;
    envelope.header_hint.bit_depth = bit_depth_for_frame(normalized);
    envelope.header_hint.bytes_per_pixel = bytes_per_pixel_for_frame(normalized);
    envelope.header_hint.frame_id = next_frame_id_++;
    envelope.received_steady_ts = std::chrono::steady_clock::now();

    return SourceReadResult{
        .status = SourceReadStatus::FrameReady,
        .envelope = std::move(envelope),
        .reason = "",
    };
}

} // namespace dp1v2
