//
//  Worker.h
//  manager
//
//  Created by apple on 27.05.2024.
//

#pragma once

#include "Payload.h"
#include <vector>
#include <memory>
#include <thread>
#include <atomic>

namespace cm
{
    class Worker
    {
    private:
        struct Context
        {
            std::shared_ptr<Context> anchor;
            std::shared_ptr<Payload> payload;
            //std::thread thread;
            std::atomic<bool> finished;
        };
        
        Worker(std::shared_ptr<Context> context);
    public:
        ~Worker();
        
        void join();
        
        bool finished() const;
        
        static std::unique_ptr<Worker> create(std::shared_ptr<Payload> payload);
    private:
        static void threadProc(Context* context_ptr);
        
        std::weak_ptr<Context> _context;
        std::thread _thread;
    };
}
