//
//  StopAction.cpp
//  manager
//
//  Created by apple on 01.06.2024.
//

#include "StopAction.h"
#include "Constants.h"
#include "utils/Log.h"

#include <filesystem>

namespace cm
{

StopAction::StopAction(const CLIOptions& options)
{
    LOG_INFO << "Stop action initialized";
}

StopAction::~StopAction()
{
    
}

int StopAction::execute(IPMessageQueue& queue)
{
    if (std::filesystem::exists(manager_enabled_file_path))
        std::filesystem::remove(manager_enabled_file_path);
    
    IPMessage message;
    message.type = IPMessageType_Stop;
    queue.send(message);

    LOG_INFO << "File toggle removed, stop message sent";
    return 0;
}

}
