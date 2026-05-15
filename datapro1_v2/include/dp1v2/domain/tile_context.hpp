#pragma once

#include <cstdint>
#include <vector>

#include "dp1v2/domain/status.hpp"

namespace dp1v2 {

struct TileContext {
    std::uint64_t frame_id = 0;
    int tile_id = -1;
    std::vector<StageStatus> stage_statuses;
    std::vector<DiagnosticMessage> diagnostics;
};

} // namespace dp1v2
