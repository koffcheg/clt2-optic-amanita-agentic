#include "dp1v2/stages/prep_stage.hpp"

namespace dp1v2 {
namespace {

constexpr const char *kFullFrameVariant = "full_frame";
constexpr const char *kTilesVariant = "tiles";

StageOutcome<PrepFullFrameOutput> fullFrameFailure(
    const StageExecutionStatus status,
    const std::string &reason)
{
    return StageOutcome<PrepFullFrameOutput>{
        .status = status,
        .reason = reason,
    };
}

} // namespace

PrepStage::PrepStage(PrepResolvedConfig resolved_config)
    : tiles_config_(resolved_config.tiles)
{
}

StageCapabilities PrepStage::capabilities() const noexcept
{
    return StageCapabilities{
        .supports_full_frame = true,
        .supports_tiles = tiles_config_.has_value(),
        .supports_roi = false,
        .supports_adaptive_roi = false,
    };
}

StageOutcome<PrepFullFrameOutput> PrepStage::process(
    const PrepFullFrameInput &input,
    FrameContext &context,
    const StageConfig &config)
{
    (void)context;

    if (!config.enabled) {
        return fullFrameFailure(StageExecutionStatus::Disabled, "prep stage is disabled");
    }

    if (config.variant != kFullFrameVariant) {
        return fullFrameFailure(
            StageExecutionStatus::Unsupported,
            "unsupported prep full-frame variant: " + config.variant);
    }

    const CanonicalFrame &frame = input.frame;
    if (frame.image.empty()) {
        return fullFrameFailure(StageExecutionStatus::Failed, "canonical frame image is empty");
    }

    if (frame.image.channels() != 1) {
        return fullFrameFailure(
            StageExecutionStatus::Failed,
            "canonical frame image must be single-channel");
    }

    if (frame.geometry.width != frame.image.cols || frame.geometry.height != frame.image.rows ||
        frame.geometry.width <= 0 || frame.geometry.height <= 0) {
        return fullFrameFailure(
            StageExecutionStatus::Failed,
            "canonical frame geometry does not match image payload");
    }

    if (frame.coordinate_space != CoordinateSpace::FrameGlobal) {
        return fullFrameFailure(
            StageExecutionStatus::Failed,
            "canonical frame coordinate space must be frame-global");
    }

    return StageOutcome<PrepFullFrameOutput>{
        .status = StageExecutionStatus::Completed,
        .output = PrepFullFrameOutput{.frame = &input.frame},
        .reason = "",
    };
}

StageOutcome<PrepTilesOutput> PrepStage::process(
    const PrepTilesInput &input,
    FrameContext &context,
    const StageConfig &config)
{
    (void)input;
    (void)context;

    if (!config.enabled) {
        return StageOutcome<PrepTilesOutput>{
            .status = StageExecutionStatus::Disabled,
            .reason = "prep stage is disabled",
        };
    }

    if (config.variant != kTilesVariant) {
        return StageOutcome<PrepTilesOutput>{
            .status = StageExecutionStatus::Unsupported,
            .reason = "unsupported prep tiles variant: " + config.variant,
        };
    }

    if (!tiles_config_.has_value()) {
        return StageOutcome<PrepTilesOutput>{
            .status = StageExecutionStatus::Failed,
            .reason = "prep tiles resolved configuration is missing",
        };
    }

    return StageOutcome<PrepTilesOutput>{
        .status = StageExecutionStatus::Unsupported,
        .reason = "prep tiles route is not implemented",
    };
}
} // namespace dp1v2
