//
//  Agent.cpp
//  manager
//
//  Created by apple on 25.05.2024.
//

#include "Agent.h"

namespace cm
{

Agent::Agent(const RunParams& params)
    : _params(params)
{
    
}

Agent::~Agent()
{
    
}

void Agent::start()
{
    _payload = std::make_shared<RunPayload>(_params);
    _worker = Worker::create(_payload);
}

void Agent::stop()
{
    _payload->cancel();
}

bool Agent::finished() const
{
    return _worker->finished();
}

}

