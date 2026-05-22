#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "dp1v2/config/config.hpp"
#include "dp1v2/runtime/pipeline.hpp"
#include "dp1v2/runtime/runtime.hpp"

namespace dp1v2 {

struct RuntimeStageDurationSummary {
    std::string stage_key;
    std::uint64_t calls = 0;
    std::int64_t total_duration_ns = 0;
    std::int64_t max_duration_ns = 0;
};

struct RuntimeProfilingSummary {
    std::uint64_t frames_total = 0;
    std::uint64_t completed = 0;
    std::uint64_t failed = 0;
    std::uint64_t dropped = 0;
    std::int64_t total_frame_duration_ns = 0;
    std::int64_t min_frame_duration_ns = 0;
    std::int64_t max_frame_duration_ns = 0;
    std::uint64_t tile_count_samples = 0;
    std::uint64_t total_tile_count = 0;
    std::uint32_t max_tile_count = 0;
    std::vector<RuntimeStageDurationSummary> stages;
};

struct RuntimeProfilingRunSummary {
    RuntimeProfilingSummary profile;
    std::uint64_t source_read_attempts = 0;
    std::uint64_t source_empty_reads = 0;
    FrameLifecycleResult last_frame;
    ProcessTerminalStatus last_status = ProcessTerminalStatus::StartupValidated;
    std::string last_reason;
};

class RuntimeProfilingAggregator {
public:
    void record_frame(const SingleFramePipelineResult &result);
    void record_dropped_frame();

    const RuntimeProfilingSummary &run_summary() const;
    const RuntimeProfilingSummary &window_summary() const;
    RuntimeProfilingSummary consume_window_summary();

private:
    void record_frame_in_summary(RuntimeProfilingSummary &summary, const SingleFramePipelineResult &result);
    void record_dropped_in_summary(RuntimeProfilingSummary &summary);

    RuntimeProfilingSummary run_;
    RuntimeProfilingSummary window_;
};

bool should_emit_frame_profile_log(
    const LoggingConfig &logging,
    const ProfilingConfig &profiling,
    bool debug_enabled);

bool should_emit_window_profile_log(const LoggingConfig &logging, const ProfilingConfig &profiling);
bool should_emit_run_profile_log(const LoggingConfig &logging, const ProfilingConfig &profiling);

bool is_controlled_tiles_frame_failure(const SingleFramePipelineResult &result);

std::string format_frame_failed_log(const SingleFramePipelineResult &result);
std::string format_profiling_window_summary_log(const RuntimeProfilingSummary &summary);
std::string format_profiling_run_summary_log(const RuntimeProfilingRunSummary &summary);

} // namespace dp1v2
