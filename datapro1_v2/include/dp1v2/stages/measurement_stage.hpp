#pragma once

#include <vector>

#include "dp1v2/config/config.hpp"
#include "dp1v2/domain/frame_context.hpp"
#include "dp1v2/domain/frame_packet.hpp"
#include "dp1v2/domain/measurement_record.hpp"
#include "dp1v2/domain/processing_frame.hpp"
#include "dp1v2/domain/segment.hpp"
#include "dp1v2/domain/tile_context.hpp"
#include "dp1v2/domain/tile_processing_frame.hpp"
#include "dp1v2/domain/tile_raw_view.hpp"
#include "dp1v2/domain/validated_object.hpp"
#include "dp1v2/stages/stage_capabilities.hpp"
#include "dp1v2/stages/stage_outcome.hpp"

namespace dp1v2 {

/// Full-frame measurement input from validated objects and optional photometry references.
struct MeasurementFullFrameInput {
    const std::vector<ValidatedObject> &objects;
    const std::vector<Segment> *segments = nullptr;
    const FramePacket *raw_photometry_ref = nullptr;
    const ProcessingFrame *processing_ref = nullptr;
};

/// Full-frame measurement output in Measurement domain.
struct MeasurementFullFrameOutput {
    std::vector<MeasurementRecord> measurements;
};

/// Tile-route measurement input from tile-local objects and optional photometry references.
struct MeasurementTileInput {
    const std::vector<ValidatedObject> &objects;
    const std::vector<Segment> *segments = nullptr;
    const TileRawView *raw_photometry_ref = nullptr;
    const TileProcessingFrame *processing_ref = nullptr;
};

/// Tile-route measurement output before tile merge.
struct MeasurementTileOutput {
    std::vector<MeasurementRecord> measurements;
};

/// Canonical measurement stage boundary.
///
/// Implementations convert validated objects into explicit `MeasurementRecord[]`.
/// Optional photometry references are read-only and must be Raw/Input or
/// explicitly allowed Processing-domain carriers, not visualization artifacts.
class IMeasurementStage {
public:
    virtual ~IMeasurementStage() = default;

    /// Returns supported measurement routes.
    virtual StageCapabilities capabilities() const noexcept = 0;

    /// Produces frame-level measurements.
    virtual StageOutcome<MeasurementFullFrameOutput> process(
        const MeasurementFullFrameInput &input,
        FrameContext &context,
        const StageConfig &config) = 0;

    /// Produces tile-local measurements before merge/globalization.
    virtual StageOutcome<MeasurementTileOutput> process(
        const MeasurementTileInput &input,
        TileContext &tile_context,
        const StageConfig &config) = 0;
};

} // namespace dp1v2
