//
//  Worker.cpp
//  manager
//
//  Created by apple on 27.05.2024.
//

#include "Worker.h"

namespace cm
{

Worker::Worker(std::shared_ptr<Context> context)
    : _context(context)
{
    _thread = std::thread(Worker::threadProc, context.get());
    _thread.detach();
}

Worker::~Worker()
{
    
}

void Worker::join()
{
    if (_thread.joinable())
        _thread.join();
//    _thread = std::thread();
//    auto context = _context.lock();
//    if (!context)
//        return;
//    
//    context->thread.join();
}

bool Worker::finished() const
{
    auto context = _context.lock();
    if (!context)
        return true;

    return context->finished;
}

std::unique_ptr<Worker> Worker::create(std::shared_ptr<Payload> payload)
{
    if (!payload)
        return nullptr;
    
    auto context = std::make_shared<Context>();
    context->anchor = context;
    context->payload = payload;
    context->finished = false;
    //context->thread = std::thread(Worker::threadProc, context.get());
    return std::unique_ptr<Worker>(new Worker(context));
}

void Worker::threadProc(Context* context_ptr)
{
    auto context = context_ptr->anchor;
    context->anchor = nullptr;
    context->payload->run();
    context->finished = true;

    //context->thread.detach();
}

}
