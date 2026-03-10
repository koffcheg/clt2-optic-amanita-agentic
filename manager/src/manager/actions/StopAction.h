//
//  StopAction.h
//  manager
//
//  Created by apple on 01.06.2024.
//

#pragma once

#include "Action.h"

namespace cm
{
    class StopAction: public Action
    {
    public:
        StopAction(const CLIOptions& options);
        ~StopAction();
        
        int execute(IPMessageQueue& queue) override;
    };
}

