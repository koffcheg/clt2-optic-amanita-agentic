#pragma once

#include <vector>

#include "dp1v2/config/config.hpp"
#include "dp1v2/domain/binary_mask.hpp"
#include "dp1v2/domain/candidate.hpp"
#include "dp1v2/domain/frame_context.hpp"
#include "dp1v2/domain/processing_frame.hpp"
#include "dp1v2/domain/tile_binary_mask.hpp"
#include "dp1v2/domain/tile_context.hpp"
#include "dp1v2/domain/tile_processing_frame.hpp"
#include "dp1v2/stages/stage_capabilities.hpp"
#include "dp1v2/stages/stage_outcome.hpp"

namespace dp1v2 {

/// Full-frame candidate extraction input from detector response or allowed processing carrier.
struct CandidateExtractionFullFrameInput {
    const ProcessingFrame &response;
};

/// Full-frame candidate extraction output: mask plus provisional candidates.
struct CandidateExtractionFullFrameOutput {
    BinaryMask mask;
    std::vector<Candidate> candidates;
};

/// Tile-route candidate extraction input from tile-local detector response.
struct CandidateExtractionTileInput {
    const TileProcessingFrame &response;
};

/// Tile-route candidate extraction output before frame-level merge.
struct CandidateExtractionTileOutput {
    TileBinaryMask mask;
    std::vector<Candidate> candidates;
};

/// Canonical candidate extraction stage boundary.
///
/// Implementations convert a detector response or allowed Processing-domain
/// carrier into an explicit mask and `Candidate[]`. They must not emit segments,
/// validated objects, measurements, or DP2 payloads.
class ICandidateExtractionStage {
public:
    virtual ~ICandidateExtractionStage() = default;

    /// Returns supported candidate extraction routes.
    virtual StageCapabilities capabilities() const noexcept = 0;

    /// Extracts a full-frame binary mask and candidate list.
    virtual StageOutcome<CandidateExtractionFullFrameOutput> processFullFrame(
        const CandidateExtractionFullFrameInput &input,
        FrameContext &context,
        const StageConfig &config) = 0;

    /// Extracts a tile-local binary mask and candidate list.
    virtual StageOutcome<CandidateExtractionTileOutput> processTile(
        const CandidateExtractionTileInput &input,
        TileContext &tile_context,
        FrameContext &frame_context,
        const StageConfig &config) = 0;
};

} // namespace dp1v2
