//
//  IPMessageQueueImpl.cpp
//  manager
//
//  Created by apple on 31.05.2024.
//

#include "IPMessageQueueImpl.h"
#include <iostream>
#include "utils/Log.h"

static const char* IPMessageQueueName = "manager.mq";
static const int IPMessageQueueSize = 32;
static const int IPMessageQueueMessageSize = sizeof(cm::IPMessage) * 2;


namespace cm
{

IPMessageQueueImpl::IPMessageQueueImpl(bool primary)
{
    if (primary)
    {
        boost::interprocess::message_queue::remove(IPMessageQueueName);
        try
        {
            
            _queue = std::make_unique<boost::interprocess::message_queue>(boost::interprocess::create_only,
                                                                          IPMessageQueueName,
                                                                          IPMessageQueueSize,
                                                                          IPMessageQueueMessageSize);
        }
        catch (boost::interprocess::interprocess_exception& e)
        {
            LOG_WARNING << "Message queue creation error: " << e.what();
        }
    }
    else
    {
        try
        {
            _queue = std::make_unique<boost::interprocess::message_queue>(boost::interprocess::open_only,
                                                                          IPMessageQueueName);
        }
        catch (boost::interprocess::interprocess_exception& e)
        {
            LOG_WARNING << "Message queue opening error: " << e.what();
        }
    }
}

IPMessageQueueImpl::~IPMessageQueueImpl()
{
    
}

void IPMessageQueueImpl::send(const IPMessage& message)
{
    if (!_queue)
        return;
    _queue->send(&message, sizeof(message), 0);
}

bool IPMessageQueueImpl::receive(IPMessage& message)
{
    if (!_queue)
        return false;
    if constexpr (sizeof(message) > IPMessageQueueMessageSize)
        return false;
    char buff[IPMessageQueueMessageSize];
    boost::interprocess::message_queue::size_type receivedSize = 0;
    unsigned int priority = 0;
    if (!_queue->try_receive(buff, IPMessageQueueMessageSize, receivedSize, priority))
        return false;
    if (receivedSize != sizeof(message))
        return false;
    
    std::memcpy(&message, buff, sizeof(message));
    return true;
}

}
