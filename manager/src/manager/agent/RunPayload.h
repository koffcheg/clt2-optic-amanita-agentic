//
//  RunPayload.h
//  manager
//
//  Created by apple on 27.05.2024.
//

#pragma once

#include "RunParams.h"
#include "Payload.h"
#include <atomic>
#include <mutex>

namespace cm
{
    class Process;

    class RunPayload: public Payload
    {
    public:
        RunPayload(const RunParams& params);
        ~RunPayload();
        
        int run() override;
        void cancel();
    private:
        RunParams _params;
        Process* _process;
        std::mutex _processMutex;
        std::atomic<bool> _running;
    };
}
