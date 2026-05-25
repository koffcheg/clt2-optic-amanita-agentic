#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "dp1v2/config/config.hpp"
#include "dp1v2/domain/tile_result.hpp"
#include "dp1v2/runtime/runtime.hpp"

namespace dp1v2 {

struct TileFrameAggregationSummary {
    std::size_t total_tiles = 0;
    std::size_t completed_tile_count = 0;
    std::size_t failed_tile_count = 0;
    std::size_t unsupported_tile_count = 0;
    std::size_t skipped_tile_count = 0;
    std::size_t disabled_tile_count = 0;

    std::size_t candidate_count = 0;
    std::size_t segment_count = 0;
    std::size_t validated_object_count = 0;
    std::size_t measurement_count = 0;

    FrameTerminalStatus terminal_status = FrameTerminalStatus::Completed;
    std::string reason;
};

class TileFrameAggregator final {
public:
    TileFrameAggregationSummary aggregate(
        const std::vector<TileResult>& tile_results,
        const TileAggregationConfig& aggregation_config,
        const TileExecutionConfig& execution_config) const;
};

} // namespace dp1v2
