//
//  TouchAction.cpp
//  manager
//
//  Created by apple on 01.06.2024.
//

#include "IdleAction.h"
#include "utils/Log.h"
#include "config/Environment.h"

#include "StopAction.h"
#include "Constants.h"

#include <filesystem>
#include <fstream>
#include <boost/process.hpp>
#include <iostream>

namespace cm
{

IdleAction::IdleAction(const CLIOptions& options)
    : _path(options.path())
{
    BOOST_LOG_TRIVIAL(info) << "Idle action initialized";
}

IdleAction::~IdleAction()
{
    
}

int IdleAction::execute(IPMessageQueue& queue)
{
    if (!std::filesystem::exists(manager_enabled_file_path))
    {
        LOG_INFO << "Idle: file toggle absent";
        return 0;
    }

    {
        Environment env;
        if (env.runMode() != Environment::RunMode::Primary)
        {
            LOG_INFO << "Idle: daemon process is already running, exiting";
            return 0;
        }
    }
    
    std::vector<std::string> arguments = {"--daemon"};
    
    boost::process::child child(_path, boost::process::args(arguments), boost::process::std_out > manager_log_file_path);
    int pid  = child.native_handle();
    child.detach();
    
    LOG_INFO << "Idle: started daemon process with pid " << pid;

    return 0;
}

}


