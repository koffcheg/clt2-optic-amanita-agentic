//
//  CLIApp.cpp
//  manager
//
//  Created by apple on 25.05.2024.
//

#include "CLIApp.h"
#include "utils/Log.h"

namespace cm
{

CLIApp::CLIApp(const CLIOptions& options, std::unique_ptr<Action> action)
    : _messageQueue(false)
    , _action(std::move(action))
{
}

CLIApp::~CLIApp()
{
    
}

int CLIApp::run()
{
    if (!_action)
    {
        LOG_INFO << "No action, exiting";
        return -1;
    }

    return _action->execute(_messageQueue);
}

}
