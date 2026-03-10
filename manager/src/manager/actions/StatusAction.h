//
//  StatusAction.h
//  manager
//
//  Created by apple on 01.06.2024.
//

#pragma once

#include "Action.h"

namespace cm
{
    class StatusAction: public Action
    {
    public:
        StatusAction(const CLIOptions& options);
        ~StatusAction();
        
        int execute(IPMessageQueue& queue) override;
    };
}
