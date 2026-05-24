#include "dp1v2/stages/input_normalization_stage.hpp"

#include <string>
#include <utility>

namespace dp1v2 {
namespace {

constexpr const char *kPassthroughVariant = "passthrough";
constexpr const char *kAverageBinningVariant = "average_binning";
constexpr const char *kRawFrameArtifactId = "raw_frame";

int expectedCvDepth(const PixelFormat pixel_format) {
    if (pixel_format == PixelFormat::U8) return CV_8U;
    if (pixel_format == PixelFormat::U16) return CV_16U;
    return -1;
}

bool samePixelRange(const PixelRange &left, const PixelRange &right) {
    return left.min_value == right.min_value && left.max_value == right.max_value &&
           left.black_level == right.black_level && left.saturation_level == right.saturation_level;
}

template <typename T, typename SumT>
cv::Mat averageBinning2x2(const cv::Mat &src) {
    cv::Mat dst(src.rows / 2, src.cols / 2, src.type());
    for (int y = 0; y < dst.rows; ++y) {
        const T *row0 = src.ptr<T>(2 * y);
        const T *row1 = src.ptr<T>(2 * y + 1);
        T *dst_row = dst.ptr<T>(y);
        for (int x = 0; x < dst.cols; ++x) {
            const int x2 = 2 * x;
            const SumT sum = static_cast<SumT>(row0[x2]) + static_cast<SumT>(row0[x2 + 1]) +
                             static_cast<SumT>(row1[x2]) + static_cast<SumT>(row1[x2 + 1]);
            dst_row[x] = static_cast<T>((sum + 2) >> 2);
        }
    }
    return dst;
}

template <typename T, typename SumT>
cv::Mat averageBinning4x4(const cv::Mat &src) {
    cv::Mat dst(src.rows / 4, src.cols / 4, src.type());
    for (int y = 0; y < dst.rows; ++y) {
        const T *row0 = src.ptr<T>(4 * y);
        const T *row1 = src.ptr<T>(4 * y + 1);
        const T *row2 = src.ptr<T>(4 * y + 2);
        const T *row3 = src.ptr<T>(4 * y + 3);
        T *dst_row = dst.ptr<T>(y);
        for (int x = 0; x < dst.cols; ++x) {
            const int x4 = 4 * x;
            const SumT sum =
                static_cast<SumT>(row0[x4]) + static_cast<SumT>(row0[x4 + 1]) +
                static_cast<SumT>(row0[x4 + 2]) + static_cast<SumT>(row0[x4 + 3]) +
                static_cast<SumT>(row1[x4]) + static_cast<SumT>(row1[x4 + 1]) +
                static_cast<SumT>(row1[x4 + 2]) + static_cast<SumT>(row1[x4 + 3]) +
                static_cast<SumT>(row2[x4]) + static_cast<SumT>(row2[x4 + 1]) +
                static_cast<SumT>(row2[x4 + 2]) + static_cast<SumT>(row2[x4 + 3]) +
                static_cast<SumT>(row3[x4]) + static_cast<SumT>(row3[x4 + 1]) +
                static_cast<SumT>(row3[x4 + 2]) + static_cast<SumT>(row3[x4 + 3]);
            dst_row[x] = static_cast<T>((sum + 8) >> 4);
        }
    }
    return dst;
}

StageOutcome<InputNormalizationOutput> failure(const StageExecutionStatus status, std::string reason) {
    return StageOutcome<InputNormalizationOutput>{.status = status, .reason = std::move(reason)};
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
    frame.normalization = {.source = NormalizationSource::Stage0, .copied = false, .converted = false, .binned = false,
                           .bin_factor_x = 1, .bin_factor_y = 1, .binning_mode = BinningMode::None,
                           .source_pixel_format = packet.pixel_format, .source_bit_depth = packet.bit_depth,
                           .source_pixel_range = packet.pixel_range};
    return frame;
}

} // namespace

InputNormalizationStage::InputNormalizationStage(InputNormalizationResolvedConfig resolved)
    : resolved_(resolved) {}

StageOutcome<InputNormalizationOutput> InputNormalizationStage::process(
    const InputNormalizationInput &input, FrameContext &context, const InputNormalizationConfig &config) const {
    (void)context;
    if (!config.stage.enabled) return failure(StageExecutionStatus::Disabled, "input_normalization stage is disabled");
    if (config.stage.variant != kPassthroughVariant &&
        config.stage.variant != kAverageBinningVariant) {
        return failure(StageExecutionStatus::Unsupported, "unsupported input_normalization variant: " + config.stage.variant);
    }

    const FramePacket &packet = input.frame;
    if (packet.image.empty()) return failure(StageExecutionStatus::Failed, "input frame is empty");
    if (packet.image.channels() != 1) return failure(StageExecutionStatus::Failed, "input frame must be single-channel");
    if (packet.geometry.width != packet.image.cols || packet.geometry.height != packet.image.rows || packet.geometry.width <= 0 || packet.geometry.height <= 0)
        return failure(StageExecutionStatus::Failed, "input frame geometry does not match image payload");
    if (packet.image.step[0] == 0) return failure(StageExecutionStatus::Failed, "input frame stride is invalid");
    if (packet.pixel_format != config.input_route.pixel_format) return failure(StageExecutionStatus::Unsupported, "input_route pixel_format mismatch");
    if (packet.bit_depth != config.input_route.bit_depth) return failure(StageExecutionStatus::Unsupported, "input_route bit_depth mismatch");
    if (!samePixelRange(packet.pixel_range, config.input_route.pixel_range)) return failure(StageExecutionStatus::Unsupported, "input_route pixel_range mismatch");
    if (packet.image.depth() != expectedCvDepth(config.input_route.pixel_format)) return failure(StageExecutionStatus::Unsupported, "input_route carrier depth mismatch");

    CanonicalFrame frame = makeCanonicalFrame(packet);
    const int kbin = resolved_.bin_factor;
    if (resolved_.binning_mode == BinningMode::None && kbin != 1) {
        return failure(StageExecutionStatus::Unsupported, "input_normalization disabled binning requires kbin=1");
    }
    if (resolved_.binning_mode == BinningMode::Average && (kbin != 2 && kbin != 4)) {
        return failure(StageExecutionStatus::Unsupported, "average binning requires kbin=2 or kbin=4");
    }
    if (resolved_.binning_mode == BinningMode::Average) {
        if ((packet.geometry.width % kbin) != 0 || (packet.geometry.height % kbin) != 0)
            return failure(StageExecutionStatus::Unsupported, "average binning requires frame geometry divisible by kbin");
        if (packet.pixel_format == PixelFormat::U8) {
            frame.image = (kbin == 2)
                ? averageBinning2x2<std::uint8_t, std::uint32_t>(packet.image)
                : averageBinning4x4<std::uint8_t, std::uint32_t>(packet.image);
        } else if (packet.pixel_format == PixelFormat::U16) {
            frame.image = (kbin == 2)
                ? averageBinning2x2<std::uint16_t, std::uint32_t>(packet.image)
                : averageBinning4x4<std::uint16_t, std::uint32_t>(packet.image);
        } else {
            return failure(StageExecutionStatus::Unsupported, "average binning supports U8/U16 only");
        }
        frame.image_ownership = CanonicalPayloadOwnership::OwnedBinned;
        frame.geometry = FrameGeometry{.width = frame.image.cols, .height = frame.image.rows};
        frame.normalization.binned = true;
        frame.normalization.bin_factor_x = kbin;
        frame.normalization.bin_factor_y = kbin;
        frame.normalization.binning_mode = BinningMode::Average;
    }

    return StageOutcome<InputNormalizationOutput>{.status = StageExecutionStatus::Completed, .output = InputNormalizationOutput{.frame = frame}, .reason = ""};
}

} // namespace dp1v2
