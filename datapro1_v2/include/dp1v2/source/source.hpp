#pragma once

#include <chrono>

#include "dp1v2/frame/frame_packet.hpp"

namespace dp1v2 {

enum class SourceReadStatus {
    FrameReady,
    SourceExhausted,
    Timeout,
    StopRequested,
    Failed,
};

struct RawFrameEnvelope {
    cv::Mat frame;
    FrameHeaderHint header_hint;
    std::chrono::steady_clock::time_point received_steady_ts{};
};

struct SourceReadResult {
    SourceReadStatus status = SourceReadStatus::Failed;
    RawFrameEnvelope envelope{};
    const char *reason = "";

    [[nodiscard]] bool has_frame() const {
        return status == SourceReadStatus::FrameReady;
    }
};

const char *source_read_status_to_cstr(SourceReadStatus status);

class IFrameSource {
public:
    virtual ~IFrameSource() = default;
    virtual SourceReadResult read_next() = 0;
};

} // namespace dp1v2
