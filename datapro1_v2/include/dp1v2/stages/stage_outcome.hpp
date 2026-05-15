#pragma once

#include <string>

#include "dp1v2/stages/stage_status.hpp"

namespace dp1v2 {

template <typename T>
struct StageOutcome {
    StageExecutionStatus status = StageExecutionStatus::Skipped;
    T output{};
    std::string reason;
};

} // namespace dp1v2
