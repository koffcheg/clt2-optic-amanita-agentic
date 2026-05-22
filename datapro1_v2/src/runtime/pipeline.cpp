#include "dp1v2/runtime/pipeline.hpp"

#include <chrono>
#include <cstdint>
#include <string>
#include <string_view>

#include "dp1v2/frame/frame_context.hpp"
#include "dp1v2/result/result_builder.hpp"
#include "dp1v2/stages/input_normalization_stage.hpp"
#include "dp1v2/stages/prep_stage.hpp"
#include "dp1v2/stages/radiometric_stage.hpp"

namespace dp1v2 {
namespace {

constexpr std::string_view kInputBoundaryName = "input";
constexpr std::string_view kInputNormalizationStageName = "input_normalization";
constexpr std::string_view kPrepStageName = "prep";
constexpr std::string_view kRadiometricStageName = "radiometric";
constexpr std::string_view kRadiometricCanonicalStageName = "radiometric_correction";
constexpr std::string_view kPrepFullFrameVariant = "full_frame";
constexpr std::string_view kPrepTilesVariant = "tiles";
constexpr std::string_view kPrepTilesDownstreamNotConnectedReason =
    "prep tiles layout is built, but downstream tile pipeline is not connected yet";

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

FrameContextSnapshot make_frame_context_snapshot(const FrameContext &context) {
    return FrameContextSnapshot{
        .stage_statuses = context.stage_statuses,
        .profiling = context.profiling,
        .diagnostics = context.diagnostics,
        .artifacts = context.artifacts,
    };
}

SingleFramePipelineResult make_pipeline_result(
    const FrameContext &context,
    const FrameLifecycleResult &lifecycle,
    const ResultSinkOutcome &sink = ResultSinkOutcome{}) {
    return SingleFramePipelineResult{
        .lifecycle = lifecycle,
        .sink = sink,
        .frame = make_frame_context_snapshot(context),
    };
}

void record_pipeline_stage_status(
    FrameContext &context,
    const std::string_view stage_key,
    const StageConfig &config,
    const StageStatusCode status,
    const std::string_view route,
    const std::string_view reason = {}) {
    record_stage_status(
        context,
        StageStatusUpdate{
            .stage_key = stage_key,
            .status = status,
            .variant = config.variant,
            .level = config.level,
            .route = route,
            .reason = reason,
        });
}

} // namespace

SingleFramePipelineResult process_single_frame(
    const RawFrameEnvelope& envelope,
    const int cam_index,
    const PipelineConfig& pipeline_config,
    const InputNormalizationStage& input_normalization_stage,
    PrepStage& prep_stage,
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
    record_stage_status(
        frame_context,
        StageStatusUpdate{
            .stage_key = kInputBoundaryName,
            .status = StageStatusCode::Completed,
            .route = "input",
        });

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
    record_pipeline_stage_status(
        frame_context,
        kInputNormalizationStageName,
        pipeline_config.stages.input_normalization,
        toStageStatusCode(input_normalization_result.status),
        pipeline_config.stages.prep.variant,
        input_normalization_result.reason);
    if (input_normalization_result.status != StageExecutionStatus::Completed) {
        std::string lifecycle_reason = "input_normalization_stage_failed";
        if (!input_normalization_result.reason.empty()) {
            lifecycle_reason += ": ";
            lifecycle_reason += input_normalization_result.reason;
        }
        return make_pipeline_result(
            frame_context,
            FrameLifecycleResult{
                .status = FrameTerminalStatus::Failed,
                .reason = lifecycle_reason,
            });
    }

    const CanonicalFrame &canonical_frame = input_normalization_result.output.frame;

    const CanonicalFrame *prepared_frame = nullptr;
    const std::string_view prep_variant(pipeline_config.stages.prep.variant);

    if (prep_variant == kPrepFullFrameVariant) {
        const auto prep_start = std::chrono::steady_clock::now();
        const auto prep_result = prep_stage.process(
            PrepFullFrameInput{.frame = canonical_frame},
            frame_context,
            pipeline_config.stages.prep);
        const auto prep_end = std::chrono::steady_clock::now();
        record_stage_timing(
            frame_context,
            kPrepStageName,
            toStageStatusCode(prep_result.status),
            pipeline_config.stages.prep.variant,
            pipeline_config.stages.prep.level,
            canonical_frame.pixel_format,
            canonical_frame.pixel_format,
            prep_start,
            prep_end,
            prep_result.reason);
        record_pipeline_stage_status(
            frame_context,
            kPrepStageName,
            pipeline_config.stages.prep,
            toStageStatusCode(prep_result.status),
            prep_variant,
            prep_result.reason);
        if (prep_result.status != StageExecutionStatus::Completed) {
            return make_pipeline_result(
                frame_context,
                FrameLifecycleResult{
                    .status = FrameTerminalStatus::Failed,
                    .reason = "prep_stage_failed: " + prep_result.reason,
                });
        }
        prepared_frame = prep_result.output.frame;
    } else if (prep_variant == kPrepTilesVariant) {
        const auto prep_start = std::chrono::steady_clock::now();
        const auto prep_result = prep_stage.process(
            PrepTilesInput{.frame = canonical_frame},
            frame_context,
            pipeline_config.stages.prep);
        const auto prep_end = std::chrono::steady_clock::now();
        record_stage_timing(
            frame_context,
            kPrepStageName,
            toStageStatusCode(prep_result.status),
            pipeline_config.stages.prep.variant,
            pipeline_config.stages.prep.level,
            canonical_frame.pixel_format,
            canonical_frame.pixel_format,
            prep_start,
            prep_end,
            prep_result.reason);
        record_pipeline_stage_status(
            frame_context,
            kPrepStageName,
            pipeline_config.stages.prep,
            toStageStatusCode(prep_result.status),
            prep_variant,
            prep_result.reason);
        if (prep_result.status != StageExecutionStatus::Completed) {
            return make_pipeline_result(
                frame_context,
                FrameLifecycleResult{
                    .status = FrameTerminalStatus::Failed,
                    .reason = "prep_stage_failed: " + prep_result.reason,
                });
        }
        set_frame_tile_count(
            frame_context,
            static_cast<std::uint32_t>(prep_result.output.tiles.size()));
        record_pipeline_stage_status(
            frame_context,
            kRadiometricCanonicalStageName,
            pipeline_config.stages.radiometric,
            StageStatusCode::NotStarted,
            prep_variant,
            kPrepTilesDownstreamNotConnectedReason);
        record_diagnostic(
            frame_context,
            "prep.tiles.downstream_not_connected",
            kPrepTilesDownstreamNotConnectedReason);
        return make_pipeline_result(
            frame_context,
            FrameLifecycleResult{
                .status = FrameTerminalStatus::Failed,
                .reason = std::string(kPrepTilesDownstreamNotConnectedReason),
            });
    } else {
        const auto prep_start = std::chrono::steady_clock::now();
        const std::string prep_reason =
            "unsupported runtime prep variant: " + pipeline_config.stages.prep.variant;
        record_stage_timing(
            frame_context,
            kPrepStageName,
            StageStatusCode::Unsupported,
            pipeline_config.stages.prep.variant,
            pipeline_config.stages.prep.level,
            canonical_frame.pixel_format,
            canonical_frame.pixel_format,
            prep_start,
            std::chrono::steady_clock::now(),
            prep_reason);
        record_pipeline_stage_status(
            frame_context,
            kPrepStageName,
            pipeline_config.stages.prep,
            StageStatusCode::Unsupported,
            prep_variant,
            prep_reason);
        return make_pipeline_result(
            frame_context,
            FrameLifecycleResult{
                .status = FrameTerminalStatus::Failed,
                .reason = "prep_stage_failed: " + prep_reason,
            });
    }

    const CanonicalFrame &prepared_frame_ref = *prepared_frame;

    const auto radiometric_start = std::chrono::steady_clock::now();
    const auto radiometric_result = radiometric_stage.process(
        RadiometricFullFrameInput{.frame = prepared_frame_ref},
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
            : prepared_frame_ref.pixel_format;
    record_stage_timing(
        frame_context,
        kRadiometricCanonicalStageName,
        toStageStatusCode(radiometric_result.status),
        pipeline_config.stages.radiometric.variant,
        pipeline_config.stages.radiometric.level,
        prepared_frame_ref.pixel_format,
        radiometric_output_format,
        radiometric_start,
        radiometric_end,
        radiometric_result.reason);
    record_pipeline_stage_status(
        frame_context,
        kRadiometricCanonicalStageName,
        pipeline_config.stages.radiometric,
        toStageStatusCode(radiometric_result.status),
        kPrepFullFrameVariant,
        radiometric_result.reason);
    if (radiometric_result.status == StageExecutionStatus::Failed ||
        radiometric_result.status == StageExecutionStatus::Unsupported) {
        return make_pipeline_result(
            frame_context,
            FrameLifecycleResult{
                .status = FrameTerminalStatus::Failed,
                .reason = "radiometric_stage_failed",
            });
    }

    const auto result = build_empty_result(frame_context);
    const auto sink = publish_result_to_sinks(result);

    return make_pipeline_result(
        frame_context,
        FrameLifecycleResult{
            .status = sink.ok() ? FrameTerminalStatus::Completed : FrameTerminalStatus::Failed,
            .reason = sink.reason,
        },
        sink);
}

} // namespace dp1v2
