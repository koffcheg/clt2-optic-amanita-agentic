#pragma once

#include <vector>

#include "dp1v2/config/config.hpp"
#include "dp1v2/domain/binary_mask.hpp"
#include "dp1v2/domain/candidate.hpp"
#include "dp1v2/domain/frame_context.hpp"
#include "dp1v2/domain/segment.hpp"
#include "dp1v2/domain/tile_binary_mask.hpp"
#include "dp1v2/domain/tile_context.hpp"
#include "dp1v2/stages/stage_capabilities.hpp"
#include "dp1v2/stages/stage_outcome.hpp"

namespace dp1v2 {

/// Full-frame segmentation input from mask and optional candidate hints.
struct SegmentationFullFrameInput {
    const BinaryMask &mask;
    const std::vector<Candidate> &candidates;
};

/// Full-frame segmentation output in Struct domain.
struct SegmentationFullFrameOutput {
    std::vector<Segment> segments;
};

/// Tile-route segmentation input from tile-local mask and candidate hints.
struct SegmentationTileInput {
    const TileBinaryMask &mask;
    const std::vector<Candidate> &candidates;
};

/// Tile-route segmentation output before tile merge.
struct SegmentationTileOutput {
    std::vector<Segment> segments;
};

/// Canonical segmentation refinement stage boundary.
///
/// Implementations refine mask/candidate information into explicit `Segment[]`.
/// They must not emit validated objects or measurements.
class ISegmentationStage {
public:
    virtual ~ISegmentationStage() = default;

    /// Returns supported segmentation routes.
    virtual StageCapabilities capabilities() const noexcept = 0;

    /// Produces full-frame segments from a binary mask and candidate hints.
    virtual StageOutcome<SegmentationFullFrameOutput> processFullFrame(
        const SegmentationFullFrameInput &input,
        FrameContext &context,
        const StageConfig &config) = 0;

    /// Produces tile-local segments from a tile mask and candidate hints.
    virtual StageOutcome<SegmentationTileOutput> processTile(
        const SegmentationTileInput &input,
        TileContext &tile_context,
        FrameContext &frame_context,
        const StageConfig &config) = 0;
};

} // namespace dp1v2
