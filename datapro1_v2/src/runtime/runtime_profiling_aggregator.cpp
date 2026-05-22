#include "dp1v2/runtime/runtime_profiling_aggregator.hpp"

#include <iomanip>
#include <sstream>
#include <string_view>

namespace dp1v2 {
namespace {

constexpr std::string_view kPrepTilesDownstreamNotConnectedCode =
    "prep.tiles.downstream_not_connected";

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

const char *process_status_to_cstr(const ProcessTerminalStatus status) {
    switch (status) {
    case ProcessTerminalStatus::StartupValidated:
        return "startup_validated";
    case ProcessTerminalStatus::NoDataAvailable:
        return "no_data_available";
    case ProcessTerminalStatus::StopRequested:
        return "stop_requested";
    case ProcessTerminalStatus::SourceExhausted:
        return "source_exhausted";
    case ProcessTerminalStatus::Failed:
        return "failed";
    }
    return "unknown";
}

double ns_to_ms(const std::int64_t duration_ns) {
    return static_cast<double>(duration_ns) / 1000000.0;
}

void append_ms(std::ostringstream &stream, const std::int64_t duration_ns) {
    stream << std::fixed << std::setprecision(3) << ns_to_ms(duration_ns);
}

std::int64_t average_ns(const std::int64_t total_ns, const std::uint64_t count) {
    if (count == 0) {
        return 0;
    }
    return total_ns / static_cast<std::int64_t>(count);
}

RuntimeStageDurationSummary *find_stage_summary(
    RuntimeProfilingSummary &summary,
    const std::string &stage_key) {
    for (RuntimeStageDurationSummary &stage : summary.stages) {
        if (stage.stage_key == stage_key) {
            return &stage;
        }
    }
    return nullptr;
}

const RuntimeStageDurationSummary *find_stage_summary(
    const RuntimeProfilingSummary &summary,
    const std::string_view stage_key) {
    for (const RuntimeStageDurationSummary &stage : summary.stages) {
        if (stage.stage_key == stage_key) {
            return &stage;
        }
    }
    return nullptr;
}

void record_duration(RuntimeProfilingSummary &summary, const std::int64_t duration_ns) {
    summary.total_frame_duration_ns += duration_ns;
    if (summary.frames_total == 1 || duration_ns < summary.min_frame_duration_ns) {
        summary.min_frame_duration_ns = duration_ns;
    }
    if (duration_ns > summary.max_frame_duration_ns) {
        summary.max_frame_duration_ns = duration_ns;
    }
}

void record_stage(RuntimeProfilingSummary &summary, const StageTiming &timing) {
    RuntimeStageDurationSummary *stage = find_stage_summary(summary, timing.stage_key);
    if (stage == nullptr) {
        summary.stages.push_back(RuntimeStageDurationSummary{.stage_key = timing.stage_key});
        stage = &summary.stages.back();
    }
    ++stage->calls;
    stage->total_duration_ns += timing.duration_ns;
    if (timing.duration_ns > stage->max_duration_ns) {
        stage->max_duration_ns = timing.duration_ns;
    }
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

const StageStatus *find_stage_status(
    const FrameContextSnapshot &frame,
    const std::string_view stage_key) {
    for (const StageStatus &status : frame.stage_statuses) {
        if (status.stage_key == stage_key) {
            return &status;
        }
    }
    return nullptr;
}

std::string sanitized_field_value(const std::string_view value) {
    std::string sanitized;
    sanitized.reserve(value.size());
    for (const char ch : value) {
        if (ch == '\r' || ch == '\n' || ch == '\t' || ch == ' ' ||
            ch == '=' || ch == '"' || ch == ';') {
            sanitized.push_back('_');
        } else {
            sanitized.push_back(ch);
        }
    }
    return sanitized;
}

std::uint64_t average_tile_count(const RuntimeProfilingSummary &summary) {
    if (summary.tile_count_samples == 0) {
        return 0;
    }
    return summary.total_tile_count / summary.tile_count_samples;
}

void append_stage_average(
    std::ostringstream &stream,
    const RuntimeProfilingSummary &summary,
    const std::string_view stage_key,
    const std::string_view field_name) {
    if (const RuntimeStageDurationSummary *stage = find_stage_summary(summary, stage_key)) {
        stream << ' ' << field_name << '=';
        append_ms(stream, average_ns(stage->total_duration_ns, stage->calls));
    }
}

} // namespace

void RuntimeProfilingAggregator::record_frame(const SingleFramePipelineResult &result) {
    record_frame_in_summary(run_, result);
    record_frame_in_summary(window_, result);
}

void RuntimeProfilingAggregator::record_dropped_frame() {
    record_dropped_in_summary(run_);
    record_dropped_in_summary(window_);
}

const RuntimeProfilingSummary &RuntimeProfilingAggregator::run_summary() const {
    return run_;
}

const RuntimeProfilingSummary &RuntimeProfilingAggregator::window_summary() const {
    return window_;
}

RuntimeProfilingSummary RuntimeProfilingAggregator::consume_window_summary() {
    RuntimeProfilingSummary summary = window_;
    window_ = RuntimeProfilingSummary{};
    return summary;
}

void RuntimeProfilingAggregator::record_frame_in_summary(
    RuntimeProfilingSummary &summary,
    const SingleFramePipelineResult &result) {
    ++summary.frames_total;
    if (result.lifecycle.status == FrameTerminalStatus::Completed) {
        ++summary.completed;
    } else if (result.lifecycle.status == FrameTerminalStatus::Dropped) {
        ++summary.dropped;
    } else {
        ++summary.failed;
    }

    record_duration(summary, result.frame.profiling.frame_duration_ns);
    ++summary.tile_count_samples;
    summary.total_tile_count += result.frame.profiling.cardinality.tile_count;
    if (result.frame.profiling.cardinality.tile_count > summary.max_tile_count) {
        summary.max_tile_count = result.frame.profiling.cardinality.tile_count;
    }
    for (const StageTiming &timing : result.frame.profiling.stage_timings) {
        record_stage(summary, timing);
    }
}

void RuntimeProfilingAggregator::record_dropped_in_summary(RuntimeProfilingSummary &summary) {
    ++summary.frames_total;
    ++summary.dropped;
}

bool should_emit_frame_profile_log(
    const LoggingConfig &logging,
    const ProfilingConfig &profiling,
    const bool debug_enabled) {
    return logging.enabled && profiling.enabled && profiling.reports.emit_frame_reports && debug_enabled;
}

bool should_emit_window_profile_log(const LoggingConfig &logging, const ProfilingConfig &profiling) {
    return logging.enabled
           && profiling.enabled
           && profiling.reports.emit_window_summary
           && profiling.logging_bridge.emit_aggregated_summaries
           && profiling.logging_bridge.summary_every_n_frames > 0;
}

bool should_emit_run_profile_log(const LoggingConfig &logging, const ProfilingConfig &profiling) {
    return logging.enabled && profiling.enabled && profiling.reports.emit_run_summary;
}

bool is_controlled_tiles_frame_failure(const SingleFramePipelineResult &result) {
    if (result.lifecycle.status != FrameTerminalStatus::Failed) {
        return false;
    }
    if (has_diagnostic_code(result.frame, kPrepTilesDownstreamNotConnectedCode)) {
        return true;
    }
    const StageStatus *radiometric = find_stage_status(result.frame, "radiometric_correction");
    return radiometric != nullptr
           && radiometric->status == StageStatusCode::NotStarted
           && radiometric->route == "tiles";
}

std::string format_frame_failed_log(const SingleFramePipelineResult &result) {
    const StageStatus *prep = find_stage_status(result.frame, "prep");
    const StageStatus *radiometric = find_stage_status(result.frame, "radiometric_correction");
    const bool controlled = is_controlled_tiles_frame_failure(result);

    std::ostringstream stream;
    stream << "event=frame_failed"
           << " frame_id=" << result.frame.frame_id
           << " camera_id=" << result.frame.camera_id
           << " status=" << frame_status_to_cstr(result.lifecycle.status)
           << " controlled=" << (controlled ? "true" : "false");
    if (controlled) {
        stream << " reason=prep_tiles_downstream_not_connected";
    } else if (!result.lifecycle.reason.empty()) {
        stream << " reason=" << sanitized_field_value(result.lifecycle.reason);
    }
    if (prep != nullptr && !prep->variant.empty()) {
        stream << " prep_variant=" << prep->variant;
    }
    stream << " tile_count=" << result.frame.profiling.cardinality.tile_count;
    if (radiometric != nullptr) {
        stream << " radiometric_status=" << stage_status_to_cstr(radiometric->status);
    }
    return stream.str();
}

std::string format_profiling_window_summary_log(const RuntimeProfilingSummary &summary) {
    std::ostringstream stream;
    stream << "event=profiling_window_summary"
           << " frames=" << summary.frames_total
           << " completed=" << summary.completed
           << " failed=" << summary.failed
           << " dropped=" << summary.dropped
           << " avg_frame_duration_ms=";
    append_ms(stream, average_ns(summary.total_frame_duration_ns, summary.frames_total));
    stream << " min_frame_duration_ms=";
    append_ms(stream, summary.min_frame_duration_ns);
    stream << " max_frame_duration_ms=";
    append_ms(stream, summary.max_frame_duration_ns);
    append_stage_average(stream, summary, "input", "stage_input_avg_ms");
    append_stage_average(stream, summary, "input_normalization", "stage_input_normalization_avg_ms");
    append_stage_average(stream, summary, "prep", "stage_prep_avg_ms");
    append_stage_average(stream, summary, "radiometric_correction", "stage_radiometric_correction_avg_ms");
    stream << " avg_tile_count=" << average_tile_count(summary)
           << " max_tile_count=" << summary.max_tile_count;
    return stream.str();
}

std::string format_profiling_run_summary_log(const RuntimeProfilingRunSummary &summary) {
    std::ostringstream stream;
    stream << "event=profiling_run_summary"
           << " frames_total=" << summary.profile.frames_total
           << " completed=" << summary.profile.completed
           << " failed=" << summary.profile.failed
           << " dropped=" << summary.profile.dropped
           << " source_read_attempts=" << summary.source_read_attempts
           << " source_empty_reads=" << summary.source_empty_reads
           << " avg_frame_duration_ms=";
    append_ms(stream, average_ns(summary.profile.total_frame_duration_ns, summary.profile.frames_total));
    stream << " max_frame_duration_ms=";
    append_ms(stream, summary.profile.max_frame_duration_ns);
    stream << " last_status=" << process_status_to_cstr(summary.last_status);
    if (!summary.last_reason.empty()) {
        stream << " last_reason=" << summary.last_reason;
    }
    return stream.str();
}

} // namespace dp1v2
