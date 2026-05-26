#include "dp1v2/runtime/full_frame_pipeline.hpp"

#include <chrono>
#include <optional>
#include <string_view>

#include "dp1v2/frame/frame_context.hpp"
#include "dp1v2/result/result_builder.hpp"
#include "dp1v2/runtime/stage2_boundary_adapter.hpp"

namespace dp1v2 {
namespace {

constexpr std::string_view kPrepFullFrameVariant = "full_frame";
constexpr std::string_view kRadiometricStageName = "radiometric";
constexpr std::string_view kRadiometricCanonicalStageName = "radiometric_correction";
constexpr std::string_view kRadiometricBypassProducerStage = "radiometric_bypass";
constexpr std::string_view kCanonicalFrameArtifactId = "canonical_frame";

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

void record_full_frame_stage_status(
    FrameContext &context,
    const StageConfig &config,
    const StageStatusCode status,
    const std::string_view reason = {}) {
    record_stage_status(
        context,
        StageStatusUpdate{
            .stage_key = kRadiometricCanonicalStageName,
            .status = status,
            .variant = config.variant,
            .level = config.level,
            .route = kPrepFullFrameVariant,
            .reason = reason,
        });
}

} // namespace

FullFramePipeline::FullFramePipeline(
    RadiometricStage &radiometric_stage,
    Stage2BoundaryAdapter &stage2_boundary_adapter,
    VisualizationSink &visualization_sink)
    : radiometric_stage_(radiometric_stage),
      stage2_boundary_adapter_(stage2_boundary_adapter),
      visualization_sink_(visualization_sink) {
}

FullFramePipelineResult FullFramePipeline::process(const FullFramePipelineArgs &args) {
    const StageConfig &radiometric_config = args.pipeline_config.stages.radiometric;
    const auto radiometric_start = std::chrono::steady_clock::now();
    const auto radiometric_result = radiometric_stage_.process(
        RadiometricFullFrameInput{.frame = args.frame},
        args.frame_context,
        radiometric_config);
    const auto radiometric_end = std::chrono::steady_clock::now();

    if (visualization_sink_.enabled_for_stage(kRadiometricStageName)) {
        visualization_sink_.write_stage_output(
            args.frame_context,
            kRadiometricStageName,
            radiometric_result);
    }

    Stage2BoundaryWorkspace stage2_workspace;
    std::optional<Stage2FullFrameSelection> stage2_selection;
    switch (radiometric_result.status) {
    case StageExecutionStatus::Completed:
    case StageExecutionStatus::Disabled:
    case StageExecutionStatus::Skipped:
        stage2_selection = stage2_boundary_adapter_.selectFullFrameOutput(
            args.frame,
            radiometric_result,
            stage2_workspace);
        break;
    case StageExecutionStatus::Failed:
    case StageExecutionStatus::Unsupported:
        break;
    }

    const PixelFormat radiometric_output_format = stage2_selection.has_value()
        ? stage2_selection->frame.pixel_format
        : args.frame.pixel_format;
    record_stage_timing(
        args.frame_context,
        kRadiometricCanonicalStageName,
        toStageStatusCode(radiometric_result.status),
        radiometric_config.variant,
        radiometric_config.level,
        args.frame.pixel_format,
        radiometric_output_format,
        radiometric_start,
        radiometric_end,
        radiometric_result.reason);
    record_full_frame_stage_status(
        args.frame_context,
        radiometric_config,
        toStageStatusCode(radiometric_result.status),
        radiometric_result.reason);

    if (radiometric_result.status == StageExecutionStatus::Failed ||
        radiometric_result.status == StageExecutionStatus::Unsupported) {
        return FullFramePipelineResult{
            .lifecycle = FrameLifecycleResult{
                .status = FrameTerminalStatus::Failed,
                .reason = "radiometric_stage_failed",
            },
            .sink = ResultSinkOutcome{},
        };
    }

    if (!stage2_selection.has_value()) {
        return FullFramePipelineResult{
            .lifecycle = FrameLifecycleResult{
                .status = FrameTerminalStatus::Failed,
                .reason = "stage2_boundary_selection_missing",
            },
            .sink = ResultSinkOutcome{},
        };
    }

    const std::string_view producer_stage = stage2_selection->is_bypass
        ? kRadiometricBypassProducerStage
        : kRadiometricCanonicalStageName;
    register_stage2_boundary_processing_artifact(
        args.frame_context,
        stage2_selection->frame,
        producer_stage,
        kCanonicalFrameArtifactId);

    const auto result = build_empty_result(args.frame_context);
    const auto sink = publish_result_to_sinks(result);

    return FullFramePipelineResult{
        .lifecycle = FrameLifecycleResult{
            .status = sink.ok() ? FrameTerminalStatus::Completed : FrameTerminalStatus::Failed,
            .reason = sink.reason,
        },
        .sink = sink,
    };
}

} // namespace dp1v2
