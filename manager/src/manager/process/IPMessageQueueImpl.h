//
//  IPMessageQueueImpl.h
//  manager
//
//  Created by apple on 31.05.2024.
//

#pragma once

#include <boost/interprocess/ipc/message_queue.hpp>
#include <memory>
#include "IPMessage.h"

namespace cm
{
    class IPMessageQueueImpl
    {
    public:
        IPMessageQueueImpl(bool primary);
        ~IPMessageQueueImpl();
        
        void send(const IPMessage& message);
        bool receive(IPMessage& message);
    private:
        std::unique_ptr<boost::interprocess::message_queue> _queue;
    };
}
