#include "dp1v2/runtime/profiling_log_formatter.hpp"

#include <iomanip>
#include <sstream>
#include <string>
#include <string_view>

namespace dp1v2 {
namespace {

constexpr std::string_view kPrepTilesDownstreamNotConnectedCode =
    "prep.tiles.downstream_not_connected";
constexpr std::size_t kMaxFieldValueLength = 128;

const char *stage_status_to_cstr(const StageStatusCode status) {
    switch (status) {
    case StageStatusCode::NotStarted:
        return "not_started";
    case StageStatusCode::Completed:
        return "completed";
    case StageStatusCode::Skipped:
        return "skipped";
    case StageStatusCode::Disabled:
        return "disabled";
    case StageStatusCode::Unsupported:
        return "unsupported";
    case StageStatusCode::Failed:
        return "failed";
    }
    return "unknown";
}

const char *frame_status_to_cstr(const FrameTerminalStatus status) {
    switch (status) {
    case FrameTerminalStatus::Completed:
        return "completed";
    case FrameTerminalStatus::Skipped:
        return "skipped";
    case FrameTerminalStatus::Dropped:
        return "dropped";
    case FrameTerminalStatus::Failed:
        return "failed";
    }
    return "unknown";
}

const char *pixel_format_to_cstr(const PixelFormat pixel_format) {
    switch (pixel_format) {
    case PixelFormat::U8:
        return "U8";
    case PixelFormat::U16:
        return "U16";
    case PixelFormat::F32:
        return "F32";
    case PixelFormat::MaskU8:
        return "MaskU8";
    case PixelFormat::S16:
        return "S16";
    case PixelFormat::S32:
        return "S32";
    }
    return "unknown";
}

std::string sanitized_field_value(const std::string_view value) {
    std::string sanitized;
    sanitized.reserve(value.size());
    for (const char ch : value) {
        if (sanitized.size() >= kMaxFieldValueLength) {
            break;
        }
        if (ch == '\r' || ch == '\n' || ch == '\t' || ch == ' ' ||
            ch == '=' || ch == '"' || ch == ';') {
            sanitized.push_back('_');
        } else {
            sanitized.push_back(ch);
        }
    }
    return sanitized;
}

std::string diagnostic_code_to_reason(const std::string_view code) {
    std::string reason;
    reason.reserve(code.size());
    for (const char ch : code) {
        reason.push_back(ch == '.' ? '_' : ch);
    }
    return sanitized_field_value(reason);
}

bool has_diagnostic_code(
    const FrameContextSnapshot &frame,
    const std::string_view diagnostic_code) {
    for (const DiagnosticMessage &diagnostic : frame.diagnostics) {
        if (diagnostic.code == diagnostic_code) {
            return true;
        }
    }
    return false;
}

bool is_tiles_downstream_controlled_failure(const SingleFramePipelineResult &result) {
    if (has_diagnostic_code(result.frame, kPrepTilesDownstreamNotConnectedCode)) {
        return true;
    }

    for (const StageStatus &status : result.frame.stage_statuses) {
        if (status.stage_key == "radiometric_correction" &&
            status.status == StageStatusCode::NotStarted &&
            status.route == "tiles") {
            return result.lifecycle.status == FrameTerminalStatus::Failed;
        }
    }
    return false;
}

double ns_to_ms(const std::int64_t duration_ns) {
    return static_cast<double>(duration_ns) / 1000000.0;
}

void append_duration_ms(std::ostringstream &stream, const std::int64_t duration_ns) {
    stream << std::fixed << std::setprecision(3) << ns_to_ms(duration_ns) << "ms";
}

std::string format_stage_timings(const std::vector<StageTiming> &stage_timings) {
    std::ostringstream stream;
    for (std::size_t index = 0; index < stage_timings.size(); ++index) {
        const StageTiming &timing = stage_timings[index];
        if (index > 0) {
            stream << ';';
        }
        stream << sanitized_field_value(timing.stage_key) << ':'
               << stage_status_to_cstr(timing.status) << ':';
        append_duration_ms(stream, timing.duration_ns);
        if (!timing.variant.empty() || !timing.level.empty()) {
            stream << ':' << sanitized_field_value(timing.variant)
                   << ':' << sanitized_field_value(timing.level)
                   << ':' << pixel_format_to_cstr(timing.input_format)
                   << "->" << pixel_format_to_cstr(timing.output_format);
        }
    }
    return stream.str();
}

std::string format_stage_routes(const std::vector<StageStatus> &stage_statuses) {
    std::ostringstream stream;
    for (std::size_t index = 0; index < stage_statuses.size(); ++index) {
        const StageStatus &status = stage_statuses[index];
        if (index > 0) {
            stream << ';';
        }
        stream << sanitized_field_value(status.stage_key) << ':'
               << stage_status_to_cstr(status.status);
        if (!status.route.empty()) {
            stream << ':' << sanitized_field_value(status.route);
        }
    }
    return stream.str();
}

std::string frame_reason(const SingleFramePipelineResult &result, const bool controlled) {
    if (controlled && has_diagnostic_code(result.frame, kPrepTilesDownstreamNotConnectedCode)) {
        return diagnostic_code_to_reason(kPrepTilesDownstreamNotConnectedCode);
    }
    return sanitized_field_value(result.lifecycle.reason);
}

} // namespace

std::string format_frame_profile_log(const SingleFramePipelineResult &result) {
    const bool controlled = is_tiles_downstream_controlled_failure(result);
    const std::string reason = frame_reason(result, controlled);

    std::ostringstream stream;
    stream << "event=frame_profile"
           << " frame_id=" << result.frame.frame_id
           << " camera_id=" << result.frame.camera_id;
    if (!result.frame.source_id.empty()) {
        stream << " source_id=" << sanitized_field_value(result.frame.source_id);
    }
    stream << " status=" << frame_status_to_cstr(result.lifecycle.status);
    if (!reason.empty()) {
        stream << " reason=" << reason;
    }
    if (controlled) {
        stream << " controlled=true";
    }
    stream << " total_duration_ms=" << std::fixed << std::setprecision(3)
           << ns_to_ms(result.frame.profiling.frame_duration_ns)
           << " tile_count=" << result.frame.profiling.cardinality.tile_count
           << " stage_count=" << result.frame.profiling.stage_timings.size()
           << " stages=\"" << format_stage_timings(result.frame.profiling.stage_timings)
           << "\"";
    if (!result.frame.stage_statuses.empty()) {
        stream << " routes=\"" << format_stage_routes(result.frame.stage_statuses) << "\"";
    }
    return stream.str();
}

} // namespace dp1v2
