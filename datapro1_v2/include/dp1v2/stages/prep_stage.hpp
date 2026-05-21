#pragma once

#include <optional>
#include <vector>

#include "dp1v2/config/config.hpp"
#include "dp1v2/domain/canonical_frame.hpp"
#include "dp1v2/domain/frame_context.hpp"
#include "dp1v2/domain/tile_desc.hpp"
#include "dp1v2/domain/tile_raw_view.hpp"
#include "dp1v2/stages/stage_capabilities.hpp"
#include "dp1v2/stages/stage_outcome.hpp"

namespace dp1v2 {

/// Full-frame prep input. `frame` is read-only and keeps the raw image lifetime.
struct PrepFullFrameInput {
    const CanonicalFrame &frame;
};

/// Full-frame prep output. `frame` is a non-owning pointer to the input carrier.
struct PrepFullFrameOutput {
    const CanonicalFrame *frame = nullptr;
};

/// Tile-route prep input. `frame` is read-only and tile views must not mutate it.
struct PrepTilesInput {
    const CanonicalFrame &frame;
};

/// Tile-route prep output with metadata and non-owning ROI views into `CanonicalFrame::image`.
struct PrepTilesOutput {
    std::vector<TileDesc> tiles;
    std::vector<TileRawView> tile_views;
};

/// Canonical prep stage boundary.
///
/// Implementations select a spatial route from `config`, update `context` only
/// with status/diagnostics/profiling metadata, and return explicit route
/// carriers. Prep must not create candidates, masks, segments, objects, or
/// measurements.
class IPrepStage {
public:
    virtual ~IPrepStage() = default;

    /// Returns supported prep routes without allocating image buffers.
    virtual StageCapabilities capabilities() const noexcept = 0;

    /// Prepares a full-frame route without taking ownership of the input image.
    virtual StageOutcome<PrepFullFrameOutput> process(
        const PrepFullFrameInput &input,
        FrameContext &context,
        const StageConfig &config) = 0;

    /// Builds tile descriptors and non-owning tile raw views for one frame.
    virtual StageOutcome<PrepTilesOutput> process(
        const PrepTilesInput &input,
        FrameContext &context,
        const StageConfig &config) = 0;

};

class PrepStage final : public IPrepStage {
public:
    PrepStage() = default;

    explicit PrepStage(PrepResolvedConfig resolved_config);

    StageCapabilities capabilities() const noexcept override;

    StageOutcome<PrepFullFrameOutput> process(
        const PrepFullFrameInput &input,
        FrameContext &context,
        const StageConfig &config) override;

    StageOutcome<PrepTilesOutput> process(
        const PrepTilesInput &input,
        FrameContext &context,
        const StageConfig &config) override;

private:
    std::optional<PrepTilesParametersConfig> tiles_config_;
};

} // namespace dp1v2
