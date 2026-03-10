//
//  HeartBeatAnalyzer.cpp
//  manager
//
//  Created by apple on 27.05.2024.
//

#include "HeartBeatAnalyzer.h"
#include "utils/Log.h"

static const int kMaxHeartbeatInterval = 5000; // milliseconds
static const char* kHeartbeatToken = "HEARTBEAT_TOKEN";

namespace cm
{

HeartBeatAnalyzer::HeartBeatAnalyzer()
{

}

HeartBeatAnalyzer::~HeartBeatAnalyzer()
{

}

StreamAnalyzer::Result HeartBeatAnalyzer::analyze(const std::string& s)
{
    auto now = std::chrono::system_clock::now();

    if (!_latest_heartbeat || s.find(kHeartbeatToken) != std::string::npos)
    {
        _latest_heartbeat = now;
        return StreamAnalyzer::Result::None;
    }

    auto latest = *_latest_heartbeat;

    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(now - latest).count();

    if (milliseconds > kMaxHeartbeatInterval)
        return StreamAnalyzer::Result::CancelProcess;

    return StreamAnalyzer::Result::None;
}
}

