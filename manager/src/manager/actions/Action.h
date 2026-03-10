//
//  Action.h
//  manager
//
//  Created by apple on 01.06.2024.
//

#pragma once

#include <memory>
#include "CLIOptions.h"
#include "IPMessageQueue.h"

namespace cm
{
    class Action
    {
    public:
        Action();
        virtual ~Action();
        
        virtual int execute(IPMessageQueue& queue) = 0;
        
        static std::unique_ptr<Action> create(const CLIOptions& options);
        
    };
}

