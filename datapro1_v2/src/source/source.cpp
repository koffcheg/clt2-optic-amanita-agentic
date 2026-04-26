#include "dp1v2/source/source.hpp"

namespace dp1v2 {

const char *source_read_status_to_cstr(const SourceReadStatus status) {
    switch (status) {
        case SourceReadStatus::FrameReady:
            return "frame_ready";
        case SourceReadStatus::SourceExhausted:
            return "source_exhausted";
        case SourceReadStatus::Timeout:
            return "timeout";
        case SourceReadStatus::StopRequested:
            return "stop_requested";
        case SourceReadStatus::Failed:
            return "failed";
        default:
            return "unknown";
    }
}

} // namespace dp1v2
