#pragma once

#include <cstdint>

namespace dp1v2 {

enum class StageExecutionStatus : std::uint8_t {
    Completed,
    Skipped,
    Disabled,
    Unsupported,
    Failed,
};

enum class ExecutionRoute : std::uint8_t {
    FullFrame,
    Tiles,
    Roi,
    AdaptiveRoi,
};

} // namespace dp1v2
