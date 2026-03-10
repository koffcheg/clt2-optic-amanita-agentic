//
//  Agent.h
//  manager
//
//  Created by apple on 25.05.2024.
//

#pragma once

#include "RunParams.h"
#include "Worker.h"
#include "RunPayload.h"
#include <vector>

namespace cm
{
    class Agent
    {
    public:
        Agent(const RunParams& params);
        ~Agent();
        
        void start();
        void stop();
        
        bool finished() const;
        const std::string& name() const { return _params.name; }
        const RunParams params() const { return _params; }
        
    private:
        RunParams _params;
        std::unique_ptr<Worker> _worker;
        std::shared_ptr<RunPayload> _payload;
    };
}
