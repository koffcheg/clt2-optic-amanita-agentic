//
//  RunPayload.cpp
//  manager
//
//  Created by apple on 27.05.2024.
//

#include "RunPayload.h"
#include <iostream>
#include <mutex>
#include <thread>
#include <chrono>
#include "Process.h"
#include "utils/Log.h"
#include "HeartBeatAnalyzer.h"

namespace cm
{

RunPayload::RunPayload(const RunParams& params)
    : _params(params)
    , _running(true)
    , _process (nullptr)
{
}

RunPayload::~RunPayload()
{
    
}

int RunPayload::run()
{
    while (_running)
    {
        Process process(_params);
        LOG_INFO << "Started " << _params.name;
        
        {
            std::lock_guard<std::mutex> guard(_processMutex);
            _process = &process;
        }
        
        int exit_code = process.run();
        if (exit_code != 0)
        {
            LOG_INFO << "Process " << _params.name << " exited with error code: " << exit_code;
        }
        
        {
            std::lock_guard<std::mutex> guard(_processMutex);
            _process = nullptr;
        }

        // To prevent spam if controlled app exited immdiately
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
    
    return 0;
}

void RunPayload::cancel()
{
    _running = false;
    {
        std::lock_guard<std::mutex> guard(_processMutex);
        if (_process)
            _process->cancel();
    }
    LOG_INFO << "Cancelled " << _params.name;
}

}
