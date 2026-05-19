#include "dp1v2/runtime/pipeline.hpp"

#include <chrono>
#include <string>
#include <string_view>

#include "dp1v2/frame/frame_context.hpp"
#include "dp1v2/result/result_builder.hpp"
#include "dp1v2/stages/input_normalization_stage.hpp"
#include "dp1v2/stages/radiometric_stage.hpp"

namespace dp1v2 {
namespace {

constexpr const char* kInverseMedianVariant = "inverse_median";
constexpr std::string_view kInputBoundaryName = "input";
constexpr std::string_view kInputNormalizationStageName = "input_normalization";
constexpr std::string_view kRadiometricStageName = "radiometric";
constexpr std::string_view kRadiometricCanonicalStageName = "radiometric_correction";

bool shouldRunRadiometricStage(const StageConfig& radiometric_config) {
    return radiometric_config.enabled && radiometric_config.variant == kInverseMedianVariant;
}

StageStatusCode toStageStatusCode(const StageExecutionStatus status) {
    switch (status) {
    case StageExecutionStatus::Completed:
        return StageStatusCode::Completed;
    case StageExecutionStatus::Skipped:
        return StageStatusCode::Skipped;
    case StageExecutionStatus::Disabled:
        return StageStatusCode::Disabled;
    case StageExecutionStatus::Unsupported:
        return StageStatusCode::Unsupported;
    case StageExecutionStatus::Failed:
        return StageStatusCode::Failed;
    }
    return StageStatusCode::Failed;
}

} // namespace

SingleFramePipelineResult process_single_frame(
    const RawFrameEnvelope& envelope,
    const int cam_index,
    const PipelineConfig& pipeline_config,
    RadiometricStage& radiometric_stage,
    VisualizationSink& visualization_sink) {
    const auto input_start = std::chrono::steady_clock::now();
    const auto packet_result = make_frame_packet(
        envelope.frame,
        envelope.header_hint,
        envelope.received_steady_ts);
    if (!packet_result.ok()) {
        return SingleFramePipelineResult{
            .lifecycle = FrameLifecycleResult{
                .status = FrameTerminalStatus::Failed,
                .reason = frame_packet_error_to_cstr(packet_result.error),
            },
            .sink = ResultSinkOutcome{},
        };
    }

    auto frame_context = build_frame_context(packet_result.packet, cam_index);
    frame_context.config_ref = &pipeline_config;
    register_raw_frame_artifact(frame_context, packet_result.packet);
    record_stage_timing(
        frame_context,
        kInputBoundaryName,
        StageStatusCode::Completed,
        {},
        {},
        packet_result.packet.pixel_format,
        packet_result.packet.pixel_format,
        input_start,
        std::chrono::steady_clock::now());

    const InputNormalizationStage input_normalization_stage;
    const auto input_normalization_start = std::chrono::steady_clock::now();
    const auto input_normalization_result = input_normalization_stage.process(
        InputNormalizationInput{.frame = packet_result.packet},
        frame_context,
        InputNormalizationConfig{
            .input_route = pipeline_config.input_route,
            .stage = pipeline_config.stages.input_normalization,
        });
    const auto input_normalization_end = std::chrono::steady_clock::now();
    if (input_normalization_result.status == StageExecutionStatus::Completed) {
        register_canonical_frame_artifact(frame_context, input_normalization_result.output.frame);
    }
    const PixelFormat input_normalization_output_format =
        input_normalization_result.status == StageExecutionStatus::Completed
            ? input_normalization_result.output.frame.pixel_format
            : frame_context.input_format;
    record_stage_timing(
        frame_context,
        kInputNormalizationStageName,
        toStageStatusCode(input_normalization_result.status),
        pipeline_config.stages.input_normalization.variant,
        pipeline_config.stages.input_normalization.level,
        frame_context.input_format,
        input_normalization_output_format,
        input_normalization_start,
        input_normalization_end,
        input_normalization_result.reason);
    if (input_normalization_result.status != StageExecutionStatus::Completed) {
        std::string lifecycle_reason = "input_normalization_stage_failed";
        if (!input_normalization_result.reason.empty()) {
            lifecycle_reason += ": ";
            lifecycle_reason += input_normalization_result.reason;
        }
        return SingleFramePipelineResult{
            .lifecycle = FrameLifecycleResult{
                .status = FrameTerminalStatus::Failed,
                .reason = lifecycle_reason,
            },
            .sink = ResultSinkOutcome{},
        };
    }

    const CanonicalFrame &canonical_frame = input_normalization_result.output.frame;

    if (shouldRunRadiometricStage(pipeline_config.stages.radiometric)) {
        const auto radiometric_start = std::chrono::steady_clock::now();
        const auto radiometric_result = radiometric_stage.process(
            RadiometricFullFrameInput{.frame = canonical_frame},
            frame_context,
            pipeline_config.stages.radiometric);
        const auto radiometric_end = std::chrono::steady_clock::now();
        if (visualization_sink.enabled_for_stage(kRadiometricStageName)) {
            visualization_sink.write_stage_output(frame_context, kRadiometricStageName, radiometric_result);
        }
        if (radiometric_result.status == StageExecutionStatus::Completed) {
            register_radiometric_processing_artifact(frame_context, radiometric_result.output.frame);
        }
        const PixelFormat radiometric_output_format =
            radiometric_result.status == StageExecutionStatus::Completed
                ? radiometric_result.output.frame.pixel_format
                : frame_context.input_format;
        record_stage_timing(
            frame_context,
            kRadiometricCanonicalStageName,
            toStageStatusCode(radiometric_result.status),
            pipeline_config.stages.radiometric.variant,
            pipeline_config.stages.radiometric.level,
            frame_context.input_format,
            radiometric_output_format,
            radiometric_start,
            radiometric_end,
            radiometric_result.reason);
        if (radiometric_result.status == StageExecutionStatus::Failed ||
            radiometric_result.status == StageExecutionStatus::Unsupported) {
            return SingleFramePipelineResult{
                .lifecycle = FrameLifecycleResult{
                    .status = FrameTerminalStatus::Failed,
                    .reason = "radiometric_stage_failed",
                },
                .sink = ResultSinkOutcome{},
            };
        }
    }

    const auto result = build_empty_result(frame_context);
    const auto sink = publish_result_to_sinks(result);

    return SingleFramePipelineResult{
        .lifecycle = FrameLifecycleResult{
            .status = sink.ok() ? FrameTerminalStatus::Completed : FrameTerminalStatus::Failed,
            .reason = sink.reason,
        },
        .sink = sink,
    };
}

} // namespace dp1v2
