//
//  StartAction.h
//  manager
//
//  Created by apple on 01.06.2024.
//

#pragma once

#include "Action.h"

namespace cm
{
    class StartAction: public Action
    {
    public:
        StartAction(const CLIOptions& options);
        ~StartAction();
        
        int execute(IPMessageQueue& queue) override;
    private:
        std::string _path;
    };
}
