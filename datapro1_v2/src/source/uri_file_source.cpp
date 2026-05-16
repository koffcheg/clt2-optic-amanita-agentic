#include "dp1v2/source/uri_file_source.hpp"

#include <cstdlib>
#include <utility>

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

cv::Mat normalize_uri_frame_to_mono(const cv::Mat &frame) {
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
    if (!link_.empty()) {
        capture_.open(link_);
    }
}

bool UriFileFrameSource::is_open() const {
    return capture_.isOpened();
}

SourceReadResult UriFileFrameSource::read_next() {
    if (link_.empty()) {
        return SourceReadResult{.status = SourceReadStatus::Failed, .reason = "source_link_empty"};
    }

    if (!capture_.isOpened()) {
        return SourceReadResult{.status = SourceReadStatus::Failed, .reason = "source_not_open"};
    }

    cv::Mat frame;
    capture_ >> frame;
    if (frame.empty()) {
        return SourceReadResult{.status = SourceReadStatus::SourceExhausted, .reason = "source_exhausted"};
    }

    cv::Mat normalized = normalize_uri_frame_to_mono(frame);
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
