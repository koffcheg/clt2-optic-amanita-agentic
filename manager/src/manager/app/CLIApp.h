//
//  CLIApp.h
//  manager
//
//  Created by apple on 25.05.2024.
//

#pragma once

#include "App.h"
#include "IPMessageQueue.h"
#include "Action.h"
#include <memory>

namespace cm
{
    class CLIApp : public App
    {
    public:
        CLIApp(const CLIOptions& options, std::unique_ptr<Action> action);
        ~CLIApp();
        
        int run() override;
        
    private:
        IPMessageQueue _messageQueue;
        std::unique_ptr<Action> _action;

    };
}
