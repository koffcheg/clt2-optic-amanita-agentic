#include "dp1v2/stages/radiometric_stage.hpp"

#include <exception>
#include <string>

namespace dp1v2 {
namespace {

constexpr const char* kInverseMedianVariant = "inverse_median";

int inputDepthFromPixelFormat(PixelFormat pixel_format)
{
    if (pixel_format == PixelFormat::U8) {
        return CV_8U;
    }
    if (pixel_format == PixelFormat::U16) {
        return CV_16U;
    }
    return -1;
}

int bitDepthToInt(InputBitDepth bit_depth)
{
    return static_cast<int>(bit_depth);
}

InverseMedianInputRoute makeInverseMedianRoute(const FramePacket& frame)
{
    InverseMedianInputRoute route{};
    route.frame_size = frame.image.size();
    route.input_depth = inputDepthFromPixelFormat(frame.pixel_format);
    route.bit_depth = bitDepthToInt(frame.bit_depth);
    route.range_min = static_cast<int>(frame.pixel_range.min_value);
    route.range_max = static_cast<int>(frame.pixel_range.max_value);
    route.binning_factor = 1;
    route.binning_owner = InverseMedianBinningOwner::None;
    return route;
}

PixelFormat processingPixelFormatFor(const cv::Mat& image)
{
    if (image.type() == CV_16SC1) {
        return PixelFormat::S16;
    }
    if (image.type() == CV_32SC1) {
        return PixelFormat::S32;
    }
    if (image.type() == CV_8UC1) {
        return PixelFormat::U8;
    }
    if (image.type() == CV_16UC1) {
        return PixelFormat::U16;
    }
    return PixelFormat::F32;
}

PixelRange signedResidualRangeFor(const FramePacket& frame)
{
    return PixelRange{
        .min_value = frame.pixel_range.min_value - frame.pixel_range.max_value,
        .max_value = frame.pixel_range.max_value - frame.pixel_range.min_value,
        .black_level = 0.0,
        .saturation_level = frame.pixel_range.max_value - frame.pixel_range.min_value,
    };
}

ProcessingFrame makeProcessingFrame(
    const FramePacket& input_frame,
    const cv::Mat& output_image,
    RangePolicy range_policy)
{
    ProcessingFrame frame{};
    frame.frame_id = input_frame.frame_id;
    frame.source_frame_id = input_frame.frame_id;
    frame.image = output_image;
    frame.pixel_format = processingPixelFormatFor(output_image);
    frame.value_range = range_policy == RangePolicy::SignedResidual ?
        signedResidualRangeFor(input_frame) : input_frame.pixel_range;
    frame.processing_domain = ProcessingDomain::RadiometricResidual;
    frame.range_policy = range_policy;
    frame.geometry = input_frame.geometry;
    frame.coordinate_space = CoordinateSpace::FrameGlobal;
    return frame;
}

const cv::Mat* selectOutputImage(const InverseMedianResult& result, RangePolicy& range_policy)
{
    if (result.converted_residual != nullptr) {
        range_policy = RangePolicy::ClippedToInputRange;
        return result.converted_residual;
    }
    range_policy = RangePolicy::SignedResidual;
    return result.residual;
}

} // namespace

RadiometricStage::RadiometricStage(RadiometricResolvedConfig resolved_config)
    : inverse_median_config_(resolved_config.inverse_median)
{
    if (inverse_median_config_.has_value()) {
        inverse_median_.emplace(*inverse_median_config_);
    }
}

StageCapabilities RadiometricStage::capabilities() const noexcept
{
    return StageCapabilities{
        .supports_full_frame = inverse_median_config_.has_value(),
        .supports_tiles = false,
        .supports_roi = false,
        .supports_adaptive_roi = false,
    };
}

std::optional<StageOutcome<RadiometricFullFrameOutput>>
RadiometricStage::validateInverseMedianFullFrameConfig(const StageConfig& config) const
{
    if (!config.enabled) {
        return StageOutcome<RadiometricFullFrameOutput>{
            .status = StageExecutionStatus::Disabled,
            .reason = "radiometric stage is disabled",
        };
    }

    if (config.variant != kInverseMedianVariant) {
        return StageOutcome<RadiometricFullFrameOutput>{
            .status = StageExecutionStatus::Unsupported,
            .reason = "unsupported radiometric variant: " + config.variant,
        };
    }

    if (!inverse_median_config_.has_value() || !inverse_median_.has_value()) {
        return StageOutcome<RadiometricFullFrameOutput>{
            .status = StageExecutionStatus::Failed,
            .reason = "inverse_median resolved configuration is missing",
        };
    }

    if (!inverse_median_config_->enabled) {
        return StageOutcome<RadiometricFullFrameOutput>{
            .status = StageExecutionStatus::Disabled,
            .reason = "inverse_median variant is disabled",
        };
    }

    return std::nullopt;
}

StageOutcome<RadiometricFullFrameOutput> RadiometricStage::makeInverseMedianFullFrameOutcome(
    const FramePacket& input_frame,
    const InverseMedianResult& result) const
{
    if (result.status == InverseMedianStatus::Disabled) {
        return StageOutcome<RadiometricFullFrameOutput>{
            .status = StageExecutionStatus::Disabled,
            .reason = "inverse_median variant is disabled",
        };
    }

    if (result.status == InverseMedianStatus::WarmingUp) {
        return StageOutcome<RadiometricFullFrameOutput>{
            .status = StageExecutionStatus::Skipped,
            .reason = "inverse_median warming up",
        };
    }

    RangePolicy range_policy = RangePolicy::Unknown;
    const cv::Mat* output_image = selectOutputImage(result, range_policy);
    if (output_image == nullptr) {
        return StageOutcome<RadiometricFullFrameOutput>{
            .status = StageExecutionStatus::Failed,
            .reason = "inverse_median did not produce a residual frame",
        };
    }

    return StageOutcome<RadiometricFullFrameOutput>{
        .status = StageExecutionStatus::Completed,
        .output = RadiometricFullFrameOutput{
            .frame = makeProcessingFrame(input_frame, *output_image, range_policy),
        },
        .reason = "",
    };
}

StageOutcome<RadiometricFullFrameOutput> RadiometricStage::process(
    const RadiometricFullFrameInput& input,
    FrameContext& context,
    const StageConfig& config)
{
    (void)context;

    if (const auto validation_result = validateInverseMedianFullFrameConfig(config)) {
        return *validation_result;
    }

    try {
        const InverseMedianInputRoute route = makeInverseMedianRoute(input.frame);
        inverse_median_->resetIfRouteChanged(route);
        const InverseMedianResult& result = inverse_median_->processFrame(
            input.frame.image,
            InverseMedianProcessContext{.frame_index = input.frame.frame_id});
        return makeInverseMedianFullFrameOutcome(input.frame, result);
    } catch (const std::exception& error) {
        return StageOutcome<RadiometricFullFrameOutput>{
            .status = StageExecutionStatus::Failed,
            .reason = error.what(),
        };
    }
}

StageOutcome<RadiometricTileOutput> RadiometricStage::process(
    const RadiometricTileInput& input,
    TileContext& tile_context,
    FrameContext& frame_context,
    const StageConfig& config)
{
    (void)input;
    (void)tile_context;
    (void)frame_context;

    if (!config.enabled) {
        return StageOutcome<RadiometricTileOutput>{
            .status = StageExecutionStatus::Disabled,
            .reason = "radiometric stage is disabled",
        };
    }

    if (config.variant == kInverseMedianVariant) {
        return StageOutcome<RadiometricTileOutput>{
            .status = StageExecutionStatus::Unsupported,
            .reason = "inverse_median tile route requires per-tile history ownership",
        };
    }

    return StageOutcome<RadiometricTileOutput>{
        .status = StageExecutionStatus::Unsupported,
        .reason = "unsupported radiometric tile variant: " + config.variant,
    };
}

} // namespace dp1v2
