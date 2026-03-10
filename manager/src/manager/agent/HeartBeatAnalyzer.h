//
//  HeartBeatAnalyzer.h
//  manager
//
//  Created by apple on 27.05.2024.
//

#pragma once

#include "StreamAnalyzer.h"
#include <chrono>
#include <optional>

namespace cm
{
    class HeartBeatAnalyzer: public StreamAnalyzer
    {
    public:
        HeartBeatAnalyzer();
        ~HeartBeatAnalyzer();
        Result analyze(const std::string& s) override;
    private:
        typedef std::chrono::time_point<std::chrono::system_clock> TimePoint;
        std::optional<TimePoint> _latest_heartbeat;
    };
}
