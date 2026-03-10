//
//  Action.cpp
//  manager
//
//  Created by apple on 01.06.2024.
//

#include "Action.h"
#include "StopAction.h"
#include "StartAction.h"
#include "IdleAction.h"
#include "HelpAction.h"
#include "StatusAction.h"

namespace cm
{

Action::Action()
{
    
}

Action::~Action()
{
    
}

std::unique_ptr<Action> Action::create(const CLIOptions& options)
{
    auto first_key = options.KeyAtIndex(0);
    if (!first_key.has_value())
        return nullptr;
    
    if (first_key == "--stop")
    {
        return std::make_unique<StopAction>(options);
    }
    else if (first_key == "--start")
    {
        return std::make_unique<StartAction>(options);
    }
    else if (first_key == "--idle")
    {
        return std::make_unique<IdleAction>(options);
    }
    else if (first_key == "--help")
    {
        return std::make_unique<HelpAction>(options);
    }
    else if (first_key == "--status")
    {
        return std::make_unique<StatusAction>(options);
    }

    return nullptr;
}

}
