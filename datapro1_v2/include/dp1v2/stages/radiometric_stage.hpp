#pragma once

#include <optional>

#include "dp1v2/config/config.hpp"
#include "dp1v2/domain/canonical_frame.hpp"
#include "dp1v2/domain/frame_context.hpp"
#include "dp1v2/domain/processing_frame.hpp"
#include "dp1v2/domain/tile_context.hpp"
#include "dp1v2/domain/tile_processing_frame.hpp"
#include "dp1v2/domain/tile_raw_view.hpp"
#include "dp1v2/stages/radiometric_stage.inverse_median.hpp"
#include "dp1v2/stages/stage_capabilities.hpp"
#include "dp1v2/stages/stage_outcome.hpp"
#include "dp1v2/stages/tile_radiometric_state_store.hpp"

namespace dp1v2 {

struct PrepTilesOutput;

/// Full-frame radiometric input. The raw frame is read-only for this stage.
struct RadiometricFullFrameInput {
    const CanonicalFrame &frame;
};

/// Full-frame radiometric output in Processing domain.
struct RadiometricFullFrameOutput {
    ProcessingFrame frame;
};

/// Tile-route radiometric input. The tile view is read-only and non-owning.
struct RadiometricTileInput {
    const TileRawView &tile;
};

/// Tile-route radiometric output backed by tile-local processing storage.
struct RadiometricTileOutput {
    TileProcessingFrame frame;
};

/// Canonical radiometric correction stage boundary.
///
/// Implementations consume raw or route-local input, return explicit Processing
/// domain output, and may own stateful background/history buffers when the
/// selected `config.variant` requires them. They must not mutate raw input,
/// store primary output only in `FrameContext`, or leave successful
/// authoritative output without `FrameContext.artifacts` reflection.
class IRadiometricStage {
public:
    virtual ~IRadiometricStage() = default;

    /// Returns supported radiometric execution routes without touching stage state.
    virtual StageCapabilities capabilities() const noexcept = 0;

    /// Processes one full-frame carrier and returns a Processing-domain result.
    virtual StageOutcome<RadiometricFullFrameOutput> process(
        const RadiometricFullFrameInput &input,
        FrameContext &context,
        const StageConfig &config) = 0;

    /// Processes one tile carrier or returns `Unsupported` for unsupported variants.
    virtual StageOutcome<RadiometricTileOutput> process(
        const RadiometricTileInput &input,
        TileContext &tile_context,
        const StageConfig &config) = 0;
};

class RadiometricStage final : public IRadiometricStage {
public:
    RadiometricStage() = default;

    /// Creates the dispatcher and any configured stateful variant engines.
    explicit RadiometricStage(RadiometricResolvedConfig resolved_config);

    StageCapabilities capabilities() const noexcept override;

    StageOutcome<RadiometricFullFrameOutput> process(
        const RadiometricFullFrameInput &input,
        FrameContext &context,
        const StageConfig &config) override;

    StageOutcome<RadiometricTileOutput> process(
        const RadiometricTileInput &input,
        TileContext &tile_context,
        const StageConfig &config) override;

    std::optional<std::string> prepareTileStates(
        const PrepTilesOutput& prep_output,
        const StageConfig& config,
        cv::Size frame_size_after_stage0,
        int binning_factor);

private:
    std::optional<StageOutcome<RadiometricFullFrameOutput>> validateInverseMedianFullFrameConfig(
        const StageConfig &config) const;
    std::optional<std::string> validateInverseMedianTileConfig(const StageConfig& config);
    StageOutcome<RadiometricFullFrameOutput> makeInverseMedianFullFrameOutcome(
        const CanonicalFrame &input_frame,
        const InverseMedianResult &result) const;
    StageOutcome<RadiometricTileOutput> makeInverseMedianTileOutcome(
        const TileRawView& input_tile,
        const InverseMedianResult& result) const;

    std::optional<InverseMedianParametersConfig> inverse_median_config_;
    std::optional<InverseMedianFilter> inverse_median_;
    TileRadiometricStateStore tile_state_store_;
    int prepared_tile_binning_factor_ = 1;
};

} // namespace dp1v2
