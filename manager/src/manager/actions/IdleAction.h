//
//  IdleAction.h
//  manager
//
//  Created by apple on 01.06.2024.
//

#pragma once

#include "Action.h"

namespace cm
{
    class IdleAction: public Action
    {
    public:
        IdleAction(const CLIOptions& options);
        ~IdleAction();
        
        int execute(IPMessageQueue& queue) override;
    private:
        std::string _path;
    };
}
