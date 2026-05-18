#include "dp1v2/stages/acquisition_input_normalization_stage.hpp"

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

StageOutcome<AcquisitionInputNormalizationOutput> failure(const StageExecutionStatus status, std::string reason) {
    return StageOutcome<AcquisitionInputNormalizationOutput>{
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

StageOutcome<AcquisitionInputNormalizationOutput> AcquisitionInputNormalizationStage::process(
    const AcquisitionInputNormalizationInput &input,
    FrameContext &context,
    const InputRouteConfig &input_route,
    const StageConfig &config) const {
    (void)context;

    if (!config.enabled) {
        return failure(StageExecutionStatus::Disabled, "acquisition stage is disabled");
    }
    if (config.variant != kPassthroughVariant) {
        return failure(StageExecutionStatus::Unsupported, "unsupported acquisition variant: " + config.variant);
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
    if (packet.pixel_format != input_route.pixel_format) {
        return failure(StageExecutionStatus::Unsupported, "input_route pixel_format mismatch");
    }
    if (packet.bit_depth != input_route.bit_depth) {
        return failure(StageExecutionStatus::Unsupported, "input_route bit_depth mismatch");
    }
    if (!samePixelRange(packet.pixel_range, input_route.pixel_range)) {
        return failure(StageExecutionStatus::Unsupported, "input_route pixel_range mismatch");
    }
    if (packet.image.depth() != expectedCvDepth(input_route.pixel_format)) {
        return failure(StageExecutionStatus::Unsupported, "input_route carrier depth mismatch");
    }

    return StageOutcome<AcquisitionInputNormalizationOutput>{
        .status = StageExecutionStatus::Completed,
        .output = AcquisitionInputNormalizationOutput{.frame = makeCanonicalFrame(packet)},
        .reason = "",
    };
}

} // namespace dp1v2
