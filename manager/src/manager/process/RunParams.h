//
//  RunParams.h
//  manager
//
//  Created by apple on 27.05.2024.
//

#pragma once

#include <string>
#include <optional>
#include <vector>

namespace cm
{
    struct RunParams
    {
        std::string name;
        std::string path;
        std::string workDir;
        std::string group;
        bool heartbeat;
        std::optional<std::string> destinationId;
        std::vector<std::string> args;
    };
}
