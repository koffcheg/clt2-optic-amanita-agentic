#pragma once

#include "dp1v2/config/config.hpp"
#include "dp1v2/domain/frame_context.hpp"
#include "dp1v2/domain/processing_frame.hpp"
#include "dp1v2/domain/tile_context.hpp"
#include "dp1v2/domain/tile_processing_frame.hpp"
#include "dp1v2/stages/stage_capabilities.hpp"
#include "dp1v2/stages/stage_outcome.hpp"

namespace dp1v2 {

/// Full-frame enhancement input in Processing domain.
struct EnhancementFullFrameInput {
    const ProcessingFrame &frame;
};

/// Full-frame enhancement output in Processing domain.
struct EnhancementFullFrameOutput {
    ProcessingFrame frame;
};

/// Tile-route enhancement input backed by tile-local processing storage.
struct EnhancementTileInput {
    const TileProcessingFrame &frame;
};

/// Tile-route enhancement output backed by tile-local processing storage.
struct EnhancementTileOutput {
    TileProcessingFrame frame;
};

/// Canonical enhancement stage boundary.
///
/// Implementations improve an existing Processing-domain representation and
/// return an explicit Processing-domain output. They must not emit detector
/// responses, masks, candidates, or measurements unless a future stage spec
/// explicitly changes that contract.
class IEnhancementStage {
public:
    virtual ~IEnhancementStage() = default;

    /// Returns supported enhancement routes without allocating image buffers.
    virtual StageCapabilities capabilities() const noexcept = 0;

    /// Processes a full-frame Processing carrier.
    virtual StageOutcome<EnhancementFullFrameOutput> process(
        const EnhancementFullFrameInput &input,
        FrameContext &context,
        const StageConfig &config) = 0;

    /// Processes a tile-local Processing carrier.
    virtual StageOutcome<EnhancementTileOutput> process(
        const EnhancementTileInput &input,
        TileContext &tile_context,
        const StageConfig &config) = 0;
};

} // namespace dp1v2
