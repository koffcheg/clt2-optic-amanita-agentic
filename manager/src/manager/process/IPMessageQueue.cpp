//
//  MessageQueue.cpp
//  manager
//
//  Created by apple on 31.05.2024.
//

#include "IPMessageQueue.h"
#include "IPMessageQueueImpl.h"


namespace cm
{

IPMessageQueue::IPMessageQueue(bool primary)
{
    _impl = std::make_unique<IPMessageQueueImpl>(primary);
}

IPMessageQueue::~IPMessageQueue()
{
    
}

void IPMessageQueue::send(const IPMessage& message)
{
    _impl->send(message);
}

bool IPMessageQueue::receive(IPMessage& message)
{
    return _impl->receive(message);
}

}
