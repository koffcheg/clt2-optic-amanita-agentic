#pragma once

#include <optional>

#include "dp1v2/config/config.hpp"
#include "dp1v2/domain/frame_context.hpp"
#include "dp1v2/domain/frame_packet.hpp"
#include "dp1v2/domain/processing_frame.hpp"
#include "dp1v2/domain/tile_context.hpp"
#include "dp1v2/domain/tile_processing_frame.hpp"
#include "dp1v2/domain/tile_raw_view.hpp"
#include "dp1v2/stages/radiometric_stage.inverse_median.hpp"
#include "dp1v2/stages/stage_capabilities.hpp"
#include "dp1v2/stages/stage_outcome.hpp"

namespace dp1v2 {

/// Full-frame radiometric input. The raw frame is read-only for this stage.
struct RadiometricFullFrameInput {
    const FramePacket &frame;
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
/// selected `config.variant` requires them. They must not mutate raw input or
/// write primary output into `FrameContext`.
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
        FrameContext &frame_context,
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
        FrameContext &frame_context,
        const StageConfig &config) override;

private:
    std::optional<StageOutcome<RadiometricFullFrameOutput>> validateInverseMedianFullFrameConfig(
        const StageConfig &config) const;
    StageOutcome<RadiometricFullFrameOutput> makeInverseMedianFullFrameOutcome(
        const FramePacket &input_frame,
        const InverseMedianResult &result) const;

    std::optional<InverseMedianParametersConfig> inverse_median_config_;
    std::optional<InverseMedianFilter> inverse_median_;
};

} // namespace dp1v2
