#include "dp1v2/runtime/pipeline.hpp"

#include "dp1v2/frame/frame_context.hpp"
#include "dp1v2/frame/frame_normalizer.hpp"
#include "dp1v2/result/result_builder.hpp"

namespace dp1v2 {

SingleFramePipelineResult process_single_frame(const RawFrameEnvelope &envelope, const int cam_index) {
    const auto packet_result = normalize_raw_frame_envelope(envelope);
    if (!packet_result.ok()) {
        return SingleFramePipelineResult{
            .lifecycle = FrameLifecycleResult{
                .status = FrameTerminalStatus::Failed,
                .reason = frame_packet_error_to_cstr(packet_result.error),
            },
            .sink = ResultSinkOutcome{},
        };
    }

    const auto frame_context = build_frame_context(packet_result.packet, cam_index);
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
