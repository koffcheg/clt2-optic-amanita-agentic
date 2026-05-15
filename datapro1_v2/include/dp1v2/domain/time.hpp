#pragma once

#include <chrono>
#include <string>

namespace dp1v2 {

enum class TimestampClock {
    Unknown,
    Steady,
    System,
    Camera,
};

struct TimestampRef {
    TimestampClock clock = TimestampClock::Unknown;
    std::chrono::steady_clock::time_point steady_time{};
    std::chrono::system_clock::time_point system_time{};
    std::string source;
    bool valid = false;
};

} // namespace dp1v2
