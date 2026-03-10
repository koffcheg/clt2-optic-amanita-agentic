//
//  HelpAction.cpp
//  manager
//
//  Created by apple on 01.06.2024.
//

#include "HelpAction.h"
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

HelpAction::HelpAction(const CLIOptions& options)
    : _path(options.path())
{
}

HelpAction::~HelpAction()
{
    
}

int HelpAction::execute(IPMessageQueue& queue)
{
    std::cout << "Welcome to Manager app" << std::endl;
    std::cout << "Here are available commands:" << std::endl;
    std::cout << "  --help - print this message" << std::endl;
    std::cout << "  --start - gracefully starts instnce of manager in daemon mode" << std::endl;
    std::cout << "  --stop - stops manager daemon" << std::endl;
    std::cout << "  --daemon - starts the app in daemon mode, for debug only" << std::endl;
    return 0;
}

}


