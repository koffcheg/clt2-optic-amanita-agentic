#pragma once

#include <cstddef>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <opencv2/core.hpp>

#include "dp1v2/config/config.hpp"
#include "dp1v2/domain/pixel.hpp"
#include "dp1v2/domain/tile_raw_view.hpp"
#include "dp1v2/stages/radiometric_stage.inverse_median.hpp"

namespace dp1v2 {

struct TileRadiometricStateKey {
    int camera_id = -1;
    int tile_id = -1;
};

bool operator==(const TileRadiometricStateKey& lhs, const TileRadiometricStateKey& rhs) noexcept;

struct TileRadiometricStateKeyHash {
    std::size_t operator()(const TileRadiometricStateKey& key) const noexcept;
};

struct TileRadiometricStateSignature {
    cv::Size tile_size;
    cv::Size frame_size_after_stage0;
    PixelFormat pixel_format = PixelFormat::U16;
    int input_depth = -1;
    int bit_depth = 0;
    int range_min = 0;
    int range_max = 0;
    int binning_factor = 1;
    std::string radiometric_variant;
    std::string inverse_median_config_signature;
};

bool operator==(
    const TileRadiometricStateSignature& lhs,
    const TileRadiometricStateSignature& rhs) noexcept;
bool operator!=(
    const TileRadiometricStateSignature& lhs,
    const TileRadiometricStateSignature& rhs) noexcept;

struct TileRadiometricStatePreparationInput {
    const std::vector<TileRawView>& tile_views;
    cv::Size frame_size_after_stage0;
    int binning_factor = 1;
    std::string radiometric_variant;
    InverseMedianParametersConfig inverse_median_config;
};

class TileRadiometricStateStore final {
public:
    void prepareTileStates(const TileRadiometricStatePreparationInput& input);

    InverseMedianFilter& getTileState(const TileRadiometricStateKey& key);
    const InverseMedianFilter& getTileState(const TileRadiometricStateKey& key) const;

    void reset();
    std::size_t size() const noexcept;

private:
    struct Entry {
        TileRadiometricStateSignature signature;
        std::unique_ptr<InverseMedianFilter> filter;
    };

    std::unordered_map<TileRadiometricStateKey, Entry, TileRadiometricStateKeyHash> states_;
};

TileRadiometricStateKey makeTileRadiometricStateKey(const TileRawView& tile);
TileRadiometricStateSignature makeTileRadiometricStateSignature(
    const TileRawView& tile,
    cv::Size frame_size_after_stage0,
    int binning_factor,
    const std::string& radiometric_variant,
    const InverseMedianParametersConfig& inverse_median_config);
InverseMedianInputRoute makeTileInverseMedianRoute(
    const TileRawView& tile,
    int binning_factor);
std::string makeInverseMedianConfigSignature(const InverseMedianParametersConfig& config);

} // namespace dp1v2
