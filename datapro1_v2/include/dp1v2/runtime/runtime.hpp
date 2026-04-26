#pragma once

#include <cstddef>

namespace dp1v2 {

struct StartupContext;
class IFrameSource;
struct SourceReadResult;

enum class FrameTerminalStatus {
    Completed,
    Skipped,
    Dropped,
    Failed,
};

enum class ProcessTerminalStatus {
    StartupValidated,
    NoDataAvailable,
    StopRequested,
    SourceExhausted,
    Failed,
};

struct FrameLifecycleResult {
    FrameTerminalStatus status = FrameTerminalStatus::Completed;
    const char *reason = "";
};

struct ProcessRunResult {
    ProcessTerminalStatus status = ProcessTerminalStatus::StartupValidated;
    int exit_code = 0;
};

struct RuntimeLoopLimits {
    std::size_t max_frames = 1;
    std::size_t max_empty_reads = 1;
};

struct RuntimeResourceState {
    std::size_t source_read_attempts = 0;
    std::size_t source_empty_reads = 0;
    std::size_t source_timeouts = 0;
    std::size_t frames_completed = 0;
    std::size_t frames_failed = 0;
    std::size_t frames_dropped = 0;
    const char *last_reason = "";
};

struct RuntimeLoopResult {
    ProcessTerminalStatus status = ProcessTerminalStatus::StartupValidated;
    std::size_t frames_completed = 0;
    std::size_t frames_failed = 0;
    FrameLifecycleResult last_frame{};
    RuntimeResourceState resource{};
};

const char *frame_terminal_status_to_cstr(FrameTerminalStatus status);
const char *process_terminal_status_to_cstr(ProcessTerminalStatus status);
ProcessRunResult run_runtime_skeleton(const StartupContext &context);
FrameLifecycleResult classify_source_read_result(const SourceReadResult &result);
RuntimeLoopResult run_bounded_runtime_loop(const StartupContext &context, IFrameSource &source, RuntimeLoopLimits limits);

} // namespace dp1v2
