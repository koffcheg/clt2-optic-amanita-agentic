//
//  IPMessageQueue.h
//  manager
//
//  Created by apple on 31.05.2024.
//

#pragma once

#include <memory>

#include "IPMessage.h"

namespace cm
{
    class IPMessageQueueImpl;

    class IPMessageQueue
    {
    public:
        IPMessageQueue(bool primary);
        ~IPMessageQueue();
        
        void send(const IPMessage& message);
        bool receive(IPMessage& message);

    private:
        std::unique_ptr<IPMessageQueueImpl> _impl;
    };
}

