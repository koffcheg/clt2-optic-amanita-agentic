#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include <opencv2/core.hpp>

#include "dp1v2/domain/candidate.hpp"
#include "dp1v2/domain/frame_context.hpp"
#include "dp1v2/domain/measurement_record.hpp"
#include "dp1v2/domain/segment.hpp"
#include "dp1v2/domain/status.hpp"
#include "dp1v2/domain/validated_object.hpp"

namespace dp1v2 {

enum class TileResultStatus : std::uint8_t {
    Completed,
    Skipped,
    Disabled,
    Unsupported,
    Failed,
};

constexpr StageStatusCode to_stage_status_code(const TileResultStatus status) noexcept
{
    switch (status) {
    case TileResultStatus::Completed:
        return StageStatusCode::Completed;
    case TileResultStatus::Skipped:
        return StageStatusCode::Skipped;
    case TileResultStatus::Disabled:
        return StageStatusCode::Disabled;
    case TileResultStatus::Unsupported:
        return StageStatusCode::Unsupported;
    case TileResultStatus::Failed:
        return StageStatusCode::Failed;
    }

    return StageStatusCode::Failed;
}

struct TileResult {
    std::uint64_t frame_id = 0;
    int camera_id = -1;
    int tile_id = -1;
    cv::Rect valid_area;

    TileResultStatus status = TileResultStatus::Completed;
    std::string error_code;
    std::string reason;

    std::vector<StageStatus> stage_statuses;
    std::vector<DiagnosticMessage> diagnostics;
    std::vector<StageTiming> stage_timings;

    std::vector<Candidate> candidates;
    std::vector<Segment> segments;
    std::vector<ValidatedObject> objects;
    std::vector<MeasurementRecord> measurements;
};

} // namespace dp1v2
