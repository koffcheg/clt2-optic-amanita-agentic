#include "dp1v2/runtime/pipeline.hpp"

#include <string_view>

#include "dp1v2/frame/frame_context.hpp"
#include "dp1v2/result/result_builder.hpp"
#include "dp1v2/stages/radiometric_stage.hpp"

namespace dp1v2 {
namespace {

constexpr const char* kInverseMedianVariant = "inverse_median";
constexpr std::string_view kRadiometricStageName = "radiometric";

bool shouldRunRadiometricStage(const StageConfig& radiometric_config) {
    return radiometric_config.enabled && radiometric_config.variant == kInverseMedianVariant;
}

} // namespace

SingleFramePipelineResult process_single_frame(
    const RawFrameEnvelope& envelope,
    const int cam_index,
    const PipelineConfig& pipeline_config,
    RadiometricStage& radiometric_stage,
    VisualizationSink& visualization_sink) {
    const auto packet_result = make_frame_packet(
        envelope.frame,
        envelope.header_hint,
        pipeline_config.input_route,
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

    if (shouldRunRadiometricStage(pipeline_config.stages.radiometric)) {
        const auto radiometric_result = radiometric_stage.process(
            RadiometricFullFrameInput{.frame = packet_result.packet},
            frame_context,
            pipeline_config.stages.radiometric);
        if (visualization_sink.enabled_for_stage(kRadiometricStageName)) {
            visualization_sink.write_stage_output(frame_context, kRadiometricStageName, radiometric_result);
        }
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
