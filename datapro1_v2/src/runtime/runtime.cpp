#include "dp1v2/runtime/runtime.hpp"

#include "dp1v2/config/config.hpp"
#include "dp1v2/runtime/pipeline.hpp"
#include "dp1v2/app/process_control.hpp"
#include "dp1v2/result/result_sink.hpp"
#include "dp1v2/source/source.hpp"
#include "dp1v2/source/source_factory.hpp"
#include "dp1v2/app/startup.hpp"
#include "dp1v2/stages/radiometric_stage.hpp"
#include "dp1v2/visualization/visualization_sink.hpp"

namespace dp1v2 {

namespace {

bool has_frame_budget(const RuntimeLoopResult &result, const RuntimeLoopLimits &limits) {
    return limits.max_frames == 0 || result.frames_completed + result.frames_failed < limits.max_frames;
}

std::size_t inverse_median_window_size(const InverseMedianParametersConfig &config) {
    if (config.mode == InverseMedianMode::FixedK5) {
        return 5;
    }
    return 3;
}

std::size_t minimum_frame_budget_for_pipeline(const ResolvedPipelineConfig &config) {
    if (config.radiometric.inverse_median.has_value() && config.radiometric.inverse_median->enabled) {
        const auto &inverse_median = *config.radiometric.inverse_median;
        return (inverse_median_window_size(inverse_median) - 1U) *
                   static_cast<std::size_t>(inverse_median.stride) +
               1U;
    }
    return 1;
}

} // namespace

const char *frame_terminal_status_to_cstr(const FrameTerminalStatus status) {
    switch (status) {
        case FrameTerminalStatus::Completed:
            return "completed";
        case FrameTerminalStatus::Skipped:
            return "skipped";
        case FrameTerminalStatus::Dropped:
            return "dropped";
        case FrameTerminalStatus::Failed:
            return "failed";
        default:
            return "unknown";
    }
}

const char *process_terminal_status_to_cstr(const ProcessTerminalStatus status) {
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
        default:
            return "unknown";
    }
}

ProcessRunResult run_runtime_skeleton(const StartupContext &context) {
    initialize_result_sink(context.config.application.dp2, context.cli.cam_index, context.calibration.camera);

    auto source = create_frame_source(context.config.application.source, context.cli.cam_index);
    const auto frame_budget = minimum_frame_budget_for_pipeline(context.config.resolved_pipeline);
    const auto loop_result = run_bounded_runtime_loop(context, *source, RuntimeLoopLimits{.max_frames = frame_budget, .max_empty_reads = 1});

    const bool success = loop_result.status == ProcessTerminalStatus::SourceExhausted
                         || loop_result.status == ProcessTerminalStatus::StopRequested
                         || loop_result.status == ProcessTerminalStatus::NoDataAvailable
                         || (loop_result.status == ProcessTerminalStatus::StartupValidated && loop_result.frames_failed == 0);

    return ProcessRunResult{
        .status = loop_result.status,
        .exit_code = success ? 0 : 1,
    };
}

FrameLifecycleResult classify_source_read_result(const SourceReadResult &result) {
    switch (result.status) {
        case SourceReadStatus::FrameReady:
            return FrameLifecycleResult{.status = FrameTerminalStatus::Completed, .reason = ""};
        case SourceReadStatus::SourceExhausted:
        case SourceReadStatus::StopRequested:
            return FrameLifecycleResult{.status = FrameTerminalStatus::Skipped, .reason = source_read_status_to_cstr(result.status)};
        case SourceReadStatus::Timeout:
            return FrameLifecycleResult{.status = FrameTerminalStatus::Dropped, .reason = source_read_status_to_cstr(result.status)};
        case SourceReadStatus::Failed:
        default:
            return FrameLifecycleResult{.status = FrameTerminalStatus::Failed, .reason = source_read_status_to_cstr(result.status)};
    }
}

RuntimeLoopResult run_bounded_runtime_loop(const StartupContext &context, IFrameSource &source, RuntimeLoopLimits limits) {
    if (limits.max_empty_reads == 0) {
        limits.max_empty_reads = 1;
    }

    RuntimeLoopResult loop_result{};
    std::size_t empty_reads = 0;
    RadiometricStage radiometric_stage(context.config.resolved_pipeline.radiometric);
    VisualizationSink visualization_sink(context.config.application.visualization);

    while (has_frame_budget(loop_result, limits)) {
        if (is_process_stop_requested()) {
            loop_result.status = ProcessTerminalStatus::StopRequested;
            loop_result.last_frame = FrameLifecycleResult{.status = FrameTerminalStatus::Skipped, .reason = "process_stop_requested"};
            return loop_result;
        }

        ++loop_result.resource.source_read_attempts;
        const auto source_result = source.read_next();
        if (is_process_stop_requested()) {
            loop_result.status = ProcessTerminalStatus::StopRequested;
            loop_result.last_frame = classify_source_read_result(source_result);
            loop_result.resource.last_reason = "process_stop_requested";
            return loop_result;
        }

        if (!source_result.has_frame()) {
            loop_result.last_frame = classify_source_read_result(source_result);
            loop_result.resource.last_reason = loop_result.last_frame.reason;

            switch (source_result.status) {
                case SourceReadStatus::SourceExhausted:
                    loop_result.status = ProcessTerminalStatus::SourceExhausted;
                    return loop_result;
                case SourceReadStatus::StopRequested:
                    loop_result.status = ProcessTerminalStatus::StopRequested;
                    return loop_result;
                case SourceReadStatus::Timeout:
                    ++empty_reads;
                    ++loop_result.resource.source_empty_reads;
                    ++loop_result.resource.source_timeouts;
                    ++loop_result.resource.frames_dropped;
                    if (empty_reads >= limits.max_empty_reads) {
                        loop_result.status = ProcessTerminalStatus::NoDataAvailable;
                        return loop_result;
                    }
                    continue;
                case SourceReadStatus::Failed:
                default:
                    loop_result.status = ProcessTerminalStatus::Failed;
                    ++loop_result.frames_failed;
                    ++loop_result.resource.frames_failed;
                    return loop_result;
            }
        }

        const auto frame_result = process_single_frame(
            source_result.envelope,
            context.cli.cam_index,
            context.config.pipeline,
            radiometric_stage,
            visualization_sink);
        loop_result.last_frame = frame_result.lifecycle;
        loop_result.resource.last_reason = frame_result.lifecycle.reason;
        if (frame_result.lifecycle.status == FrameTerminalStatus::Completed) {
            ++loop_result.frames_completed;
            ++loop_result.resource.frames_completed;
        } else if (frame_result.lifecycle.status == FrameTerminalStatus::Dropped) {
            ++loop_result.frames_failed;
            ++loop_result.resource.frames_failed;
            ++loop_result.resource.frames_dropped;
            loop_result.status = ProcessTerminalStatus::Failed;
            return loop_result;
        } else {
            ++loop_result.frames_failed;
            ++loop_result.resource.frames_failed;
            loop_result.status = ProcessTerminalStatus::Failed;
            return loop_result;
        }
    }

    loop_result.status = ProcessTerminalStatus::StopRequested;
    loop_result.resource.last_reason = "frame_limit_reached";
    return loop_result;
}

} // namespace dp1v2
