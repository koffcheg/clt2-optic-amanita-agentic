#include "dp1v2/runtime/runtime.hpp"

#include "dp1v2/config/config.hpp"
#include "dp1v2/runtime/pipeline.hpp"
#include "dp1v2/runtime/profiling_log_formatter.hpp"
#include "dp1v2/runtime/runtime_profiling_aggregator.hpp"
#include "dp1v2/app/process_control.hpp"
#include "dp1v2/result/result_sink.hpp"
#include "dp1v2/source/source.hpp"
#include "dp1v2/source/source_factory.hpp"
#include "dp1v2/app/startup.hpp"
#include "dp1v2/stages/input_normalization_stage.hpp"
#include "dp1v2/stages/prep_stage.hpp"
#include "dp1v2/stages/radiometric_stage.hpp"
#include "dp1v2/visualization/visualization_sink.hpp"

#include <log4cxx/logger.h>

namespace dp1v2 {

namespace {

log4cxx::LoggerPtr runtime_logger() {
    static log4cxx::LoggerPtr logger = log4cxx::Logger::getLogger("amanita.dp1.runtime");
    return logger;
}

log4cxx::LoggerPtr profiling_logger() {
    static log4cxx::LoggerPtr logger = log4cxx::Logger::getLogger("amanita.dp1.profiling");
    return logger;
}

bool has_frame_budget(const RuntimeLoopResult &result, const RuntimeLoopLimits &limits) {
    return limits.max_frames == 0 || result.frames_completed + result.frames_failed < limits.max_frames;
}

bool is_successful_process_status(const RuntimeLoopResult &result) {
    return result.status == ProcessTerminalStatus::SourceExhausted
           || result.status == ProcessTerminalStatus::StopRequested
           || result.status == ProcessTerminalStatus::NoDataAvailable
           || (result.status == ProcessTerminalStatus::StartupValidated && result.frames_failed == 0);
}

int process_exit_code(const RuntimeLoopResult &result) {
    return is_successful_process_status(result) ? 0 : 1;
}

std::string profiling_levels_csv(const ProfilingConfig &profiling) {
    std::string levels;
    for (const std::string &level : profiling.levels) {
        if (!levels.empty()) {
            levels += ',';
        }
        levels += level;
    }
    return levels;
}

void emit_pipeline_started_log(const StartupContext &context) {
    if (!context.config.application.logging.enabled) {
        return;
    }

    LOG4CXX_INFO(
        runtime_logger(),
        "event=pipeline_started module=dp1"
            << " camera_id=" << context.cli.cam_index
            << " logging_enabled=" << (context.config.application.logging.enabled ? "true" : "false")
            << " profiling_enabled=" << (context.config.application.profiling.enabled ? "true" : "false")
            << " profiling_mode=" << context.config.application.profiling.mode
            << " profiling_levels=" << profiling_levels_csv(context.config.application.profiling));
}

void emit_pipeline_stopped_log(const StartupContext &context, const RuntimeLoopResult &result) {
    if (!context.config.application.logging.enabled) {
        return;
    }

    LOG4CXX_INFO(
        runtime_logger(),
        "event=pipeline_stopped"
            << " status=" << process_terminal_status_to_cstr(result.status)
            << " exit_code=" << process_exit_code(result)
            << " frames_completed=" << result.frames_completed
            << " frames_failed=" << result.frames_failed
            << " reason=" << result.resource.last_reason);
}

RuntimeProfilingRunSummary make_run_summary(
    const RuntimeProfilingAggregator &aggregator,
    const RuntimeLoopResult &result) {
    return RuntimeProfilingRunSummary{
        .profile = aggregator.run_summary(),
        .source_read_attempts = result.resource.source_read_attempts,
        .source_empty_reads = result.resource.source_empty_reads,
        .last_frame = result.last_frame,
        .last_status = result.status,
        .last_reason = result.resource.last_reason,
    };
}

void emit_run_summary_log(
    const StartupContext &context,
    const RuntimeProfilingAggregator &aggregator,
    const RuntimeLoopResult &result) {
    if (!should_emit_run_profile_log(context.config.application.logging, context.config.application.profiling)) {
        return;
    }

    LOG4CXX_INFO(profiling_logger(), format_profiling_run_summary_log(make_run_summary(aggregator, result)));
}

void maybe_emit_window_summary_log(
    const StartupContext &context,
    RuntimeProfilingAggregator &aggregator) {
    if (!should_emit_window_profile_log(context.config.application.logging, context.config.application.profiling)) {
        return;
    }

    if (!is_window_summary_due(context.config.application.profiling, aggregator.window_summary())) {
        return;
    }

    LOG4CXX_INFO(profiling_logger(), format_profiling_window_summary_log(aggregator.consume_window_summary()));
}

void emit_source_drop_log(const StartupContext &context, const SourceReadResult &source_result) {
    if (!context.config.application.logging.enabled) {
        return;
    }

    LOG4CXX_WARN(
        runtime_logger(),
        "event=frame_failed"
            << " camera_id=" << context.cli.cam_index
            << " status=dropped"
            << " controlled=false"
            << " reason=" << source_read_status_to_cstr(source_result.status));
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
    const auto loop_result = run_bounded_runtime_loop(
        context,
        *source,
        RuntimeLoopLimits{.max_frames = 0, .max_empty_reads = 1});

    return ProcessRunResult{
        .status = loop_result.status,
        .exit_code = process_exit_code(loop_result),
        .reason = loop_result.resource.last_reason,
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
    const InputNormalizationStage input_normalization_stage;
    PrepStage prep_stage(context.config.resolved_pipeline.prep);
    RadiometricStage radiometric_stage(context.config.resolved_pipeline.radiometric);
    VisualizationSink visualization_sink(context.config.application.visualization);
    RuntimeProfilingAggregator profiling_aggregator;

    emit_pipeline_started_log(context);

    auto finish_loop = [&]() {
        emit_pipeline_stopped_log(context, loop_result);
        emit_run_summary_log(context, profiling_aggregator, loop_result);
        return loop_result;
    };

    while (has_frame_budget(loop_result, limits)) {
        if (is_process_stop_requested()) {
            loop_result.status = ProcessTerminalStatus::StopRequested;
            loop_result.last_frame = FrameLifecycleResult{.status = FrameTerminalStatus::Skipped, .reason = "process_stop_requested"};
            loop_result.resource.last_reason = "process_stop_requested";
            return finish_loop();
        }

        ++loop_result.resource.source_read_attempts;
        const auto source_result = source.read_next();
        if (is_process_stop_requested()) {
            loop_result.status = ProcessTerminalStatus::StopRequested;
            loop_result.last_frame = classify_source_read_result(source_result);
            loop_result.resource.last_reason = "process_stop_requested";
            return finish_loop();
        }

        if (!source_result.has_frame()) {
            loop_result.last_frame = classify_source_read_result(source_result);
            loop_result.resource.last_reason = loop_result.last_frame.reason;

            switch (source_result.status) {
                case SourceReadStatus::SourceExhausted:
                    loop_result.status = ProcessTerminalStatus::SourceExhausted;
                    return finish_loop();
                case SourceReadStatus::StopRequested:
                    loop_result.status = ProcessTerminalStatus::StopRequested;
                    return finish_loop();
                case SourceReadStatus::Timeout:
                    ++empty_reads;
                    ++loop_result.resource.source_empty_reads;
                    ++loop_result.resource.source_timeouts;
                    ++loop_result.resource.frames_dropped;
                    profiling_aggregator.record_dropped_frame();
                    emit_source_drop_log(context, source_result);
                    maybe_emit_window_summary_log(context, profiling_aggregator);
                    if (empty_reads >= limits.max_empty_reads) {
                        loop_result.status = ProcessTerminalStatus::NoDataAvailable;
                        return finish_loop();
                    }
                    continue;
                case SourceReadStatus::Failed:
                default:
                    loop_result.status = ProcessTerminalStatus::Failed;
                    ++loop_result.frames_failed;
                    ++loop_result.resource.frames_failed;
                    if (context.config.application.logging.enabled) {
                        LOG4CXX_ERROR(
                            runtime_logger(),
                            "event=source_failed camera_id=" << context.cli.cam_index
                                << " status=failed reason=" << source_read_status_to_cstr(source_result.status));
                    }
                    return finish_loop();
            }
        }

        const auto frame_result = process_single_frame(
            source_result.envelope,
            context.cli.cam_index,
            context.config.pipeline,
            input_normalization_stage,
            prep_stage,
            radiometric_stage,
            visualization_sink);
        loop_result.last_frame = frame_result.lifecycle;
        loop_result.resource.last_reason = frame_result.lifecycle.reason;
        profiling_aggregator.record_frame(frame_result);

        if (should_emit_frame_profile_log(
                context.config.application.logging,
                context.config.application.profiling,
                profiling_logger()->isDebugEnabled())) {
            LOG4CXX_DEBUG(profiling_logger(), format_frame_profile_log(frame_result));
        }
        if (is_controlled_tiles_frame_failure(frame_result)) {
            if (context.config.application.logging.enabled) {
                LOG4CXX_WARN(runtime_logger(), format_frame_failed_log(frame_result));
            }
        }
        maybe_emit_window_summary_log(context, profiling_aggregator);

        if (frame_result.lifecycle.status == FrameTerminalStatus::Completed) {
            ++loop_result.frames_completed;
            ++loop_result.resource.frames_completed;
        } else if (frame_result.lifecycle.status == FrameTerminalStatus::Dropped) {
            ++loop_result.frames_failed;
            ++loop_result.resource.frames_failed;
            ++loop_result.resource.frames_dropped;
            loop_result.status = ProcessTerminalStatus::Failed;
            if (context.config.application.logging.enabled) {
                LOG4CXX_WARN(runtime_logger(), format_frame_failed_log(frame_result));
            }
            return finish_loop();
        } else {
            ++loop_result.frames_failed;
            ++loop_result.resource.frames_failed;
            loop_result.status = ProcessTerminalStatus::Failed;
            if (!is_controlled_tiles_frame_failure(frame_result) && context.config.application.logging.enabled) {
                LOG4CXX_ERROR(runtime_logger(), format_frame_failed_log(frame_result));
            }
            return finish_loop();
        }
    }

    loop_result.status = ProcessTerminalStatus::StopRequested;
    loop_result.resource.last_reason = "frame_limit_reached";
    return finish_loop();
}

} // namespace dp1v2
