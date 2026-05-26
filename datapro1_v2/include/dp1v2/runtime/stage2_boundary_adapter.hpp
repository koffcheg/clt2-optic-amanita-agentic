#pragma once

#include <cstddef>
#include <string_view>
#include <vector>

#include <opencv2/core.hpp>

#include "dp1v2/domain/canonical_frame.hpp"
#include "dp1v2/domain/processing_frame.hpp"
#include "dp1v2/domain/tile_processing_frame.hpp"
#include "dp1v2/domain/tile_raw_view.hpp"
#include "dp1v2/stages/radiometric_stage.hpp"
#include "dp1v2/stages/stage_outcome.hpp"

namespace dp1v2 {

enum class Stage2BoundarySource {
    RadiometricOutput,
    RawBypassFromDisabledRadiometric,
    RawBypassFromSkippedRadiometric,
};

/// Owns temporary conversion buffers for one Stage 2 boundary execution (full-frame or tile).
/// `FrameContext` stores only metadata/provenance and must not store image payload.
/// `tile_bypass_u8_by_task` must be resized to the tile task count before parallel execution;
/// each worker writes only to `tile_bypass_u8_by_task[task_index]` and must not push/emplace.
/// U8 bypass reuses payload via shallow `cv::Mat` assignment.
/// U16 bypass materializes one CV_8UC1 buffer per full-frame or per tile task.
struct Stage2BoundaryWorkspace {
    cv::Mat full_frame_bypass_u8;
    std::vector<cv::Mat> tile_bypass_u8_by_task;
};

/// Describes the selected full-frame payload exposed at the Stage 2 boundary.
struct Stage2FullFrameSelection {
    ProcessingFrame frame;
    Stage2BoundarySource source = Stage2BoundarySource::RadiometricOutput;
    bool is_bypass = false;
};

/// Describes the selected tile payload exposed at the Stage 2 boundary.
struct Stage2TileSelection {
    TileProcessingFrame frame;
    Stage2BoundarySource source = Stage2BoundarySource::RadiometricOutput;
    bool is_bypass = false;
};

/// Converts `RadiometricStage` outcomes into the stable Stage 2 processing boundary.
class Stage2BoundaryAdapter final {
public:
    std::string_view boundaryKey() const noexcept;

    Stage2FullFrameSelection selectFullFrameOutput(
        const CanonicalFrame& input,
        const StageOutcome<RadiometricFullFrameOutput>& radiometric_result,
        Stage2BoundaryWorkspace& workspace) const;

    Stage2TileSelection selectTileOutput(
        std::size_t task_index,
        const TileRawView& input,
        const StageOutcome<RadiometricTileOutput>& radiometric_result,
        Stage2BoundaryWorkspace& workspace) const;

private:
    ProcessingFrame makeBypassFrame(
        const CanonicalFrame& input,
        Stage2BoundaryWorkspace& workspace) const;

    TileProcessingFrame makeBypassTileFrame(
        std::size_t task_index,
        const TileRawView& input,
        Stage2BoundaryWorkspace& workspace) const;
};

} // namespace dp1v2
