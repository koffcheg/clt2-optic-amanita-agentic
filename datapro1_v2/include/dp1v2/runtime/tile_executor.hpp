#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "dp1v2/config/config.hpp"
#include "dp1v2/domain/tile_result.hpp"
#include "dp1v2/runtime/stage2_boundary_adapter.hpp"
#include "dp1v2/runtime/tile_processor.hpp"
#include "dp1v2/runtime/tile_task.hpp"

namespace dp1v2 {

struct TileExecutionSummary {
    std::size_t total_tasks = 0;
    std::size_t completed_tasks = 0;
    std::size_t failed_tasks = 0;
    std::size_t unsupported_tasks = 0;
    std::uint64_t duration_ns = 0;
};

class TileExecutor final {
public:
    TileExecutionSummary execute(
        const std::vector<TileTask>& tasks,
        std::vector<TileResult>& results,
        Stage2BoundaryWorkspace& stage2_workspace,
        const TileProcessor& processor,
        const TileExecutionConfig& config) const;
};

} // namespace dp1v2
