#pragma once

#include <vector>

#include "dp1v2/config/config.hpp"
#include "dp1v2/domain/candidate.hpp"
#include "dp1v2/domain/frame_context.hpp"
#include "dp1v2/domain/segment.hpp"
#include "dp1v2/domain/tile_context.hpp"
#include "dp1v2/domain/validated_object.hpp"
#include "dp1v2/stages/stage_capabilities.hpp"
#include "dp1v2/stages/stage_outcome.hpp"

namespace dp1v2 {

/// Full-frame object filtering input from candidates or segments.
struct ObjectFilteringFullFrameInput {
    const std::vector<Candidate> *candidates = nullptr;
    const std::vector<Segment> *segments = nullptr;
};

/// Full-frame object filtering output in Struct domain.
struct ObjectFilteringFullFrameOutput {
    std::vector<ValidatedObject> objects;
};

/// Tile-route object filtering input from tile-local candidates or segments.
struct ObjectFilteringTileInput {
    const std::vector<Candidate> *candidates = nullptr;
    const std::vector<Segment> *segments = nullptr;
};

/// Tile-route object filtering output before tile merge.
struct ObjectFilteringTileOutput {
    std::vector<ValidatedObject> objects;
};

/// Canonical object filtering stage boundary.
///
/// Implementations accept candidates and/or segments and return explicit
/// `ValidatedObject[]`. They must not return filtered `Candidate[]` as the
/// primary output and must not emit measurements directly.
class IObjectFilteringStage {
public:
    virtual ~IObjectFilteringStage() = default;

    /// Returns supported object filtering routes.
    virtual StageCapabilities capabilities() const noexcept = 0;

    /// Produces full-frame validated objects.
    virtual StageOutcome<ObjectFilteringFullFrameOutput> processFullFrame(
        const ObjectFilteringFullFrameInput &input,
        FrameContext &context,
        const StageConfig &config) = 0;

    /// Produces tile-local validated objects.
    virtual StageOutcome<ObjectFilteringTileOutput> processTile(
        const ObjectFilteringTileInput &input,
        TileContext &tile_context,
        FrameContext &frame_context,
        const StageConfig &config) = 0;
};

} // namespace dp1v2
