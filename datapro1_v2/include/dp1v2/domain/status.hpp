#pragma once

#include <cstdint>
#include <string>

namespace dp1v2 {

enum class StageStatusCode : std::uint8_t {
    NotStarted,
    Completed,
    Skipped,
    Disabled,
    Unsupported,
    Failed,
};

struct StageStatus {
    std::string stage_key;
    StageStatusCode status = StageStatusCode::NotStarted;
    std::string reason;
};

struct DiagnosticMessage {
    std::string code;
    std::string message;
};

} // namespace dp1v2
