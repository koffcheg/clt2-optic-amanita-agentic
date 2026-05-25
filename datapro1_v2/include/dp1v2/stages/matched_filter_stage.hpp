#pragma once

#include "dp1v2/config/config.hpp"
#include "dp1v2/domain/frame_context.hpp"
#include "dp1v2/domain/processing_frame.hpp"
#include "dp1v2/domain/tile_context.hpp"
#include "dp1v2/domain/tile_processing_frame.hpp"
#include "dp1v2/stages/stage_capabilities.hpp"
#include "dp1v2/stages/stage_outcome.hpp"

namespace dp1v2 {

/// Full-frame matched-filter input in Processing domain.
struct MatchedFilterFullFrameInput {
    const ProcessingFrame &frame;
};

/// Full-frame detector response output in Processing domain.
struct MatchedFilterFullFrameOutput {
    ProcessingFrame response;
};

/// Tile-route matched-filter input in tile-local Processing domain.
struct MatchedFilterTileInput {
    const TileProcessingFrame &frame;
};

/// Tile-route detector response output in tile-local Processing domain.
struct MatchedFilterTileOutput {
    TileProcessingFrame response;
};

/// Canonical matched-filter stage boundary.
///
/// Implementations consume enhanced/allowed Processing-domain frames and emit an
/// explicit detector response carrier. The response is not a visualization image
/// and must retain Processing-domain metadata.
class IMatchedFilterStage {
public:
    virtual ~IMatchedFilterStage() = default;

    /// Returns supported matched-filter routes without touching stage state.
    virtual StageCapabilities capabilities() const noexcept = 0;

    /// Produces a full-frame detector response.
    virtual StageOutcome<MatchedFilterFullFrameOutput> process(
        const MatchedFilterFullFrameInput &input,
        FrameContext &context,
        const StageConfig &config) = 0;

    /// Produces a tile-local detector response.
    virtual StageOutcome<MatchedFilterTileOutput> process(
        const MatchedFilterTileInput &input,
        TileContext &tile_context,
        const StageConfig &config) = 0;
};

} // namespace dp1v2
