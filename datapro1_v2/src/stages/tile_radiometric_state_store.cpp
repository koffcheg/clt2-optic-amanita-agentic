#include "dp1v2/stages/tile_radiometric_state_store.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <unordered_set>

namespace dp1v2 {
namespace {

int inputDepthFromPixelFormat(const PixelFormat pixel_format)
{
    if (pixel_format == PixelFormat::U8) {
        return CV_8U;
    }
    if (pixel_format == PixelFormat::U16) {
        return CV_16U;
    }
    return -1;
}

int bitDepthToInt(const InputBitDepth bit_depth)
{
    return static_cast<int>(bit_depth);
}

int rangeValueToInt(const double value)
{
    return static_cast<int>(std::lround(value));
}

std::string boolSignature(const bool value)
{
    return value ? "true" : "false";
}

InverseMedianInputRoute routeFromSignature(const TileRadiometricStateSignature& signature)
{
    InverseMedianInputRoute route{};
    route.frame_size = signature.tile_size;
    route.input_depth = signature.input_depth;
    route.bit_depth = signature.bit_depth;
    route.range_min = signature.range_min;
    route.range_max = signature.range_max;
    route.binning_factor = signature.binning_factor;
    route.binning_owner = signature.binning_factor > 1
        ? InverseMedianBinningOwner::Dp1
        : InverseMedianBinningOwner::None;
    return route;
}

} // namespace

bool operator==(const TileRadiometricStateKey& lhs, const TileRadiometricStateKey& rhs) noexcept
{
    return lhs.camera_id == rhs.camera_id && lhs.tile_id == rhs.tile_id;
}

std::size_t TileRadiometricStateKeyHash::operator()(
    const TileRadiometricStateKey& key) const noexcept
{
    const std::size_t camera_hash = std::hash<int>{}(key.camera_id);
    const std::size_t tile_hash = std::hash<int>{}(key.tile_id);
    return camera_hash ^ (tile_hash + 0x9e3779b9U + (camera_hash << 6U) + (camera_hash >> 2U));
}

bool operator==(
    const TileRadiometricStateSignature& lhs,
    const TileRadiometricStateSignature& rhs) noexcept
{
    return lhs.tile_size == rhs.tile_size
        && lhs.frame_size_after_stage0 == rhs.frame_size_after_stage0
        && lhs.pixel_format == rhs.pixel_format
        && lhs.input_depth == rhs.input_depth
        && lhs.bit_depth == rhs.bit_depth
        && lhs.range_min == rhs.range_min
        && lhs.range_max == rhs.range_max
        && lhs.binning_factor == rhs.binning_factor
        && lhs.radiometric_variant == rhs.radiometric_variant
        && lhs.inverse_median_config_signature == rhs.inverse_median_config_signature;
}

bool operator!=(
    const TileRadiometricStateSignature& lhs,
    const TileRadiometricStateSignature& rhs) noexcept
{
    return !(lhs == rhs);
}

TileRadiometricStateKey makeTileRadiometricStateKey(const TileRawView& tile)
{
    return TileRadiometricStateKey{
        .camera_id = tile.camera_id,
        .tile_id = tile.tile_id,
    };
}

std::string makeInverseMedianConfigSignature(const InverseMedianParametersConfig& config)
{
    std::string signature = "enabled=" + boolSignature(config.enabled);
    signature += ";mode=" + toString(config.mode);
    signature += ";stride=" + std::to_string(config.stride);
    signature += ";output_median_frame=" + boolSignature(config.output_median_frame);
    signature += ";output_dynamic_range_mode=" + toString(config.output_dynamic_range_mode);
    return signature;
}

TileRadiometricStateSignature makeTileRadiometricStateSignature(
    const TileRawView& tile,
    const cv::Size frame_size_after_stage0,
    const int binning_factor,
    const std::string& radiometric_variant,
    const InverseMedianParametersConfig& inverse_median_config)
{
    return TileRadiometricStateSignature{
        .tile_size = tile.image.size(),
        .frame_size_after_stage0 = frame_size_after_stage0,
        .pixel_format = tile.pixel_format,
        .input_depth = inputDepthFromPixelFormat(tile.pixel_format),
        .bit_depth = bitDepthToInt(tile.bit_depth),
        .range_min = rangeValueToInt(tile.pixel_range.min_value),
        .range_max = rangeValueToInt(tile.pixel_range.max_value),
        .binning_factor = std::max(1, binning_factor),
        .radiometric_variant = radiometric_variant,
        .inverse_median_config_signature =
            makeInverseMedianConfigSignature(inverse_median_config),
    };
}

InverseMedianInputRoute makeTileInverseMedianRoute(
    const TileRawView& tile,
    const int binning_factor)
{
    InverseMedianInputRoute route{};
    route.frame_size = tile.image.size();
    route.input_depth = inputDepthFromPixelFormat(tile.pixel_format);
    route.bit_depth = bitDepthToInt(tile.bit_depth);
    route.range_min = rangeValueToInt(tile.pixel_range.min_value);
    route.range_max = rangeValueToInt(tile.pixel_range.max_value);
    route.binning_factor = std::max(1, binning_factor);
    route.binning_owner = route.binning_factor > 1
        ? InverseMedianBinningOwner::Dp1
        : InverseMedianBinningOwner::None;
    return route;
}

void TileRadiometricStateStore::prepareTileStates(
    const TileRadiometricStatePreparationInput& input)
{
    std::unordered_set<TileRadiometricStateKey, TileRadiometricStateKeyHash> expected_keys;
    expected_keys.reserve(input.tile_views.size());

    for (const TileRawView& tile : input.tile_views) {
        const TileRadiometricStateKey key = makeTileRadiometricStateKey(tile);
        if (key.camera_id < 0 || key.tile_id < 0) {
            throw std::logic_error("tile radiometric state key must have valid camera_id and tile_id");
        }
        if (tile.image.empty()) {
            throw std::logic_error("tile radiometric state cannot be prepared for an empty tile image");
        }

        const bool inserted = expected_keys.insert(key).second;
        if (!inserted) {
            throw std::logic_error("duplicate tile radiometric state key in current frame layout");
        }

        const TileRadiometricStateSignature signature = makeTileRadiometricStateSignature(
            tile,
            input.frame_size_after_stage0,
            input.binning_factor,
            input.radiometric_variant,
            input.inverse_median_config);

        if (signature.input_depth != CV_8U && signature.input_depth != CV_16U) {
            throw std::logic_error("inverse_median tile route supports only U8 or U16 tile input");
        }

        auto state = states_.find(key);
        if (state == states_.end()) {
            Entry entry{};
            entry.signature = signature;
            entry.filter = std::make_unique<InverseMedianFilter>(input.inverse_median_config);
            entry.filter->reset(routeFromSignature(signature));
            states_.emplace(key, std::move(entry));
            continue;
        }

        if (state->second.signature != signature) {
            state->second.signature = signature;
            state->second.filter = std::make_unique<InverseMedianFilter>(input.inverse_median_config);
            state->second.filter->reset(routeFromSignature(signature));
        }
    }

    for (auto state = states_.begin(); state != states_.end();) {
        if (expected_keys.find(state->first) == expected_keys.end()) {
            state = states_.erase(state);
        } else {
            ++state;
        }
    }
}

InverseMedianFilter& TileRadiometricStateStore::getTileState(
    const TileRadiometricStateKey& key)
{
    auto state = states_.find(key);
    if (state == states_.end() || state->second.filter == nullptr) {
        throw std::logic_error("tile radiometric state was not prepared for the requested tile");
    }
    return *state->second.filter;
}

const InverseMedianFilter& TileRadiometricStateStore::getTileState(
    const TileRadiometricStateKey& key) const
{
    const auto state = states_.find(key);
    if (state == states_.end() || state->second.filter == nullptr) {
        throw std::logic_error("tile radiometric state was not prepared for the requested tile");
    }
    return *state->second.filter;
}

void TileRadiometricStateStore::reset()
{
    states_.clear();
}

std::size_t TileRadiometricStateStore::size() const noexcept
{
    return states_.size();
}

} // namespace dp1v2
