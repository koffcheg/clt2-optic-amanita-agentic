//
//  StartAction.cpp
//  manager
//
//  Created by apple on 01.06.2024.
//

#include "StartAction.h"

#include "StopAction.h"
#include "Constants.h"
#include "utils/Log.h"
#include "config/Environment.h"

#include <filesystem>
#include <fstream>
#include <boost/process.hpp>
#include <iostream>

namespace cm
{

StartAction::StartAction(const CLIOptions& options)
    : _path(options.path())
{
    LOG_INFO << "Start action initialized";
}

StartAction::~StartAction()
{
    
}

int StartAction::execute(IPMessageQueue& queue)
{
    if (!std::filesystem::exists(manager_enabled_file_path))
    {
        std::ofstream touch(manager_enabled_file_path);
    }

    {
        Environment env;
        if (env.runMode() == Environment::RunMode::Secondary)
        {
            LOG_INFO << "Daemon process is already running, exiting";
            return 0;
        }
        else if (env.runMode() == Environment::RunMode::Failed)
        {
            LOG_ERROR << "Daemon pid file access failed";
            return 0;
        }
    }
    
    std::vector<std::string> arguments = {"--daemon"};
    
    boost::process::child child(_path, boost::process::args(arguments));
    int pid  = child.native_handle();
    child.detach();
    
    LOG_INFO << "Started daemon process with pid " << pid;

    return 0;
}

}
