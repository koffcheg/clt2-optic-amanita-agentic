#include "dp1v2/stages/radiometric_stage.hpp"

#include <algorithm>
#include <exception>
#include <string>

#include "dp1v2/stages/prep_stage.hpp"

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

InverseMedianInputRoute makeInverseMedianRoute(const CanonicalFrame& frame)
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

PixelRange signedResidualRangeFor(const CanonicalFrame& frame)
{
    return PixelRange{
        .min_value = frame.pixel_range.min_value - frame.pixel_range.max_value,
        .max_value = frame.pixel_range.max_value - frame.pixel_range.min_value,
        .black_level = 0.0,
        .saturation_level = frame.pixel_range.max_value - frame.pixel_range.min_value,
    };
}

ProcessingFrame makeProcessingFrame(
    const CanonicalFrame& input_frame,
    const cv::Mat& output_image,
    RangePolicy range_policy)
{
    ProcessingFrame frame{};
    frame.frame_id = input_frame.frame_id;
    frame.source_frame_id = input_frame.frame_id;
    frame.image = output_image;
    frame.pixel_format = PixelFormat::F32;
    frame.value_range = range_policy == RangePolicy::SignedResidual ?
        signedResidualRangeFor(input_frame) : input_frame.pixel_range;
    frame.processing_domain = ProcessingDomain::RadiometricResidual;
    frame.range_policy = range_policy;
    frame.geometry = input_frame.geometry;
    frame.coordinate_space = CoordinateSpace::FrameGlobal;
    return frame;
}

PixelRange signedResidualRangeFor(const TileRawView& tile)
{
    return PixelRange{
        .min_value = tile.pixel_range.min_value - tile.pixel_range.max_value,
        .max_value = tile.pixel_range.max_value - tile.pixel_range.min_value,
        .black_level = 0.0,
        .saturation_level = tile.pixel_range.max_value - tile.pixel_range.min_value,
    };
}

TileProcessingFrame makeTileProcessingFrame(
    const TileRawView& input_tile,
    const cv::Mat& output_image,
    const RangePolicy range_policy)
{
    TileProcessingFrame frame{};
    frame.frame_id = input_tile.frame_id;
    frame.tile_id = input_tile.tile_id;
    frame.image = output_image;
    frame.pixel_format = PixelFormat::F32;
    frame.value_range = range_policy == RangePolicy::SignedResidual
        ? signedResidualRangeFor(input_tile)
        : input_tile.pixel_range;
    frame.processing_domain = ProcessingDomain::RadiometricResidual;
    frame.range_policy = range_policy;
    frame.geometry = input_tile.geometry;
    frame.origin_in_frame = input_tile.origin_in_frame;
    frame.valid_area = input_tile.valid_area;
    frame.coordinate_space = input_tile.coordinate_space;
    return frame;
}

const cv::Mat* selectOutputImage(const InverseMedianResult& result, RangePolicy& range_policy)
{
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
        .supports_tiles = inverse_median_config_.has_value(),
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

std::optional<std::string> RadiometricStage::validateInverseMedianTileConfig(
    const StageConfig& config)
{
    if (!config.enabled) {
        tile_state_store_.reset();
        return std::nullopt;
    }

    if (config.variant != kInverseMedianVariant) {
        return std::nullopt;
    }

    if (!inverse_median_config_.has_value()) {
        return "inverse_median resolved configuration is missing";
    }

    if (!inverse_median_config_->enabled) {
        tile_state_store_.reset();
        return std::nullopt;
    }

    return std::nullopt;
}

StageOutcome<RadiometricFullFrameOutput> RadiometricStage::makeInverseMedianFullFrameOutcome(
    const CanonicalFrame& input_frame,
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

StageOutcome<RadiometricTileOutput> RadiometricStage::makeInverseMedianTileOutcome(
    const TileRawView& input_tile,
    const InverseMedianResult& result) const
{
    if (result.status == InverseMedianStatus::Disabled) {
        return StageOutcome<RadiometricTileOutput>{
            .status = StageExecutionStatus::Disabled,
            .reason = "inverse_median variant is disabled",
        };
    }

    if (result.status == InverseMedianStatus::WarmingUp) {
        return StageOutcome<RadiometricTileOutput>{
            .status = StageExecutionStatus::Skipped,
            .reason = "inverse_median warming up",
        };
    }

    RangePolicy range_policy = RangePolicy::Unknown;
    const cv::Mat* output_image = selectOutputImage(result, range_policy);
    if (output_image == nullptr) {
        return StageOutcome<RadiometricTileOutput>{
            .status = StageExecutionStatus::Failed,
            .reason = "inverse_median did not produce a residual tile",
        };
    }

    return StageOutcome<RadiometricTileOutput>{
        .status = StageExecutionStatus::Completed,
        .output = RadiometricTileOutput{
            .frame = makeTileProcessingFrame(input_tile, *output_image, range_policy),
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

std::optional<std::string> RadiometricStage::prepareTileStates(
    const PrepTilesOutput& prep_output,
    const StageConfig& config,
    const cv::Size frame_size_after_stage0,
    const int binning_factor)
{
    if (const auto validation_error = validateInverseMedianTileConfig(config)) {
        return validation_error;
    }

    if (!config.enabled || config.variant != kInverseMedianVariant) {
        return std::nullopt;
    }

    if (!inverse_median_config_.has_value() || !inverse_median_config_->enabled) {
        return std::nullopt;
    }

    try {
        tile_state_store_.prepareTileStates(
            TileRadiometricStatePreparationInput{
                .tile_views = prep_output.tile_views,
                .frame_size_after_stage0 = frame_size_after_stage0,
                .binning_factor = binning_factor,
                .radiometric_variant = config.variant,
                .inverse_median_config = *inverse_median_config_,
            });
        prepared_tile_binning_factor_ = std::max(1, binning_factor);
        return std::nullopt;
    } catch (const std::exception& error) {
        return error.what();
    }
}

StageOutcome<RadiometricTileOutput> RadiometricStage::process(
    const RadiometricTileInput& input,
    TileContext& tile_context,
    const StageConfig& config)
{
    (void)tile_context;

    if (!config.enabled) {
        return StageOutcome<RadiometricTileOutput>{
            .status = StageExecutionStatus::Disabled,
            .reason = "radiometric stage is disabled",
        };
    }

    if (config.variant == kInverseMedianVariant) {
        if (!inverse_median_config_.has_value()) {
            return StageOutcome<RadiometricTileOutput>{
                .status = StageExecutionStatus::Failed,
                .reason = "inverse_median resolved configuration is missing",
            };
        }

        if (!inverse_median_config_->enabled) {
            return StageOutcome<RadiometricTileOutput>{
                .status = StageExecutionStatus::Disabled,
                .reason = "inverse_median variant is disabled",
            };
        }

        try {
            const TileRadiometricStateKey key = makeTileRadiometricStateKey(input.tile);
            InverseMedianFilter& filter = tile_state_store_.getTileState(key);
            const InverseMedianInputRoute route = makeTileInverseMedianRoute(
                input.tile,
                prepared_tile_binning_factor_);
            if (filter.requiresReset(route)) {
                return StageOutcome<RadiometricTileOutput>{
                    .status = StageExecutionStatus::Failed,
                    .reason = "prepared inverse_median tile state does not match tile route",
                };
            }

            const InverseMedianResult& result = filter.processFrame(
                input.tile.image,
                InverseMedianProcessContext{.frame_index = input.tile.frame_id});
            return makeInverseMedianTileOutcome(input.tile, result);
        } catch (const std::exception& error) {
            return StageOutcome<RadiometricTileOutput>{
                .status = StageExecutionStatus::Failed,
                .reason = error.what(),
            };
        }
    }

    return StageOutcome<RadiometricTileOutput>{
        .status = StageExecutionStatus::Unsupported,
        .reason = "unsupported radiometric tile variant: " + config.variant,
    };
}

} // namespace dp1v2
