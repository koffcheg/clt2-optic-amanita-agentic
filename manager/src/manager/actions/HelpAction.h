//
//  HelpAction.h
//  manager
//
//  Created by apple on 01.06.2024.
//

#pragma once

#include "Action.h"

namespace cm
{
    class HelpAction: public Action
    {
    public:
        HelpAction(const CLIOptions& options);
        ~HelpAction();
        
        int execute(IPMessageQueue& queue) override;
    private:
        std::string _path;
    };
}
