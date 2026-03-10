//
//  Process.h
//  manager
//
//  Created by apple on 27.05.2024.
//

#pragma once

#include "RunParams.h"
#include "StreamAnalyzer.h"
#include <atomic>

namespace cm
{
    class Process
    {
    public:
        Process(const RunParams& params);
        ~Process();
        
        int run();
        void cancel();
    private:
        RunParams _params;
        std::atomic<bool> _running;
    };
}

