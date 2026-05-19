#include "dp1v2/stages/input_normalization_stage.hpp"

#include <string>
#include <utility>

namespace dp1v2 {
namespace {

constexpr const char *kPassthroughVariant = "passthrough";
constexpr const char *kRawFrameArtifactId = "raw_frame";

int expectedCvDepth(const PixelFormat pixel_format) {
    if (pixel_format == PixelFormat::U8) {
        return CV_8U;
    }
    if (pixel_format == PixelFormat::U16) {
        return CV_16U;
    }
    return -1;
}

bool samePixelRange(const PixelRange &left, const PixelRange &right) {
    return left.min_value == right.min_value && left.max_value == right.max_value &&
           left.black_level == right.black_level && left.saturation_level == right.saturation_level;
}

StageOutcome<InputNormalizationOutput> failure(const StageExecutionStatus status, std::string reason) {
    return StageOutcome<InputNormalizationOutput>{
        .status = status,
        .reason = std::move(reason),
    };
}

CanonicalFrame makeCanonicalFrame(const FramePacket &packet) {
    CanonicalFrame frame{};
    frame.frame_id = packet.frame_id;
    frame.camera_id = packet.camera_id;
    frame.source_id = packet.source_id;
    frame.image = packet.image;
    frame.image_ownership = CanonicalPayloadOwnership::BorrowedReadOnly;
    frame.pixel_format = packet.pixel_format;
    frame.bit_depth = packet.bit_depth;
    frame.pixel_range = packet.pixel_range;
    frame.geometry = packet.geometry;
    frame.coordinate_space = CoordinateSpace::FrameGlobal;
    frame.ingest_time = packet.ingest_time;
    frame.acquisition_time = packet.acquisition_time;
    frame.parent_artifact_id = kRawFrameArtifactId;
    frame.normalization = NormalizationProvenance{
        .source = NormalizationSource::Stage0,
        .copied = false,
        .converted = false,
        .binned = false,
        .bin_factor_x = 1,
        .bin_factor_y = 1,
        .binning_mode = BinningMode::None,
        .source_pixel_format = packet.pixel_format,
        .source_bit_depth = packet.bit_depth,
        .source_pixel_range = packet.pixel_range,
    };
    return frame;
}

} // namespace

StageOutcome<InputNormalizationOutput> InputNormalizationStage::process(
    const InputNormalizationInput &input,
    FrameContext &context,
    const InputNormalizationConfig &config) const {
    (void)context;

    if (!config.stage.enabled) {
        return failure(StageExecutionStatus::Disabled, "input_normalization stage is disabled");
    }
    if (config.stage.variant != kPassthroughVariant) {
        return failure(StageExecutionStatus::Unsupported, "unsupported input_normalization variant: " + config.stage.variant);
    }

    const FramePacket &packet = input.frame;
    if (packet.image.empty()) {
        return failure(StageExecutionStatus::Failed, "input frame is empty");
    }
    if (packet.image.channels() != 1) {
        return failure(StageExecutionStatus::Failed, "input frame must be single-channel");
    }
    if (packet.geometry.width != packet.image.cols || packet.geometry.height != packet.image.rows ||
        packet.geometry.width <= 0 || packet.geometry.height <= 0) {
        return failure(StageExecutionStatus::Failed, "input frame geometry does not match image payload");
    }
    if (packet.image.step[0] == 0) {
        return failure(StageExecutionStatus::Failed, "input frame stride is invalid");
    }
    if (packet.pixel_format != config.input_route.pixel_format) {
        return failure(StageExecutionStatus::Unsupported, "input_route pixel_format mismatch");
    }
    if (packet.bit_depth != config.input_route.bit_depth) {
        return failure(StageExecutionStatus::Unsupported, "input_route bit_depth mismatch");
    }
    if (!samePixelRange(packet.pixel_range, config.input_route.pixel_range)) {
        return failure(StageExecutionStatus::Unsupported, "input_route pixel_range mismatch");
    }
    if (packet.image.depth() != expectedCvDepth(config.input_route.pixel_format)) {
        return failure(StageExecutionStatus::Unsupported, "input_route carrier depth mismatch");
    }

    return StageOutcome<InputNormalizationOutput>{
        .status = StageExecutionStatus::Completed,
        .output = InputNormalizationOutput{.frame = makeCanonicalFrame(packet)},
        .reason = "",
    };
}

} // namespace dp1v2
