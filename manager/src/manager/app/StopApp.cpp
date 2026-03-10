//
//  StopApp.cpp
//  manager
//
//  Created by apple on 25.05.2024.
//

#include "StopApp.h"
#include "utils/Log.h"

namespace cm
{

StopApp::StopApp(const CLIOptions& options)
{
    
}

StopApp::~StopApp()
{
    
}

int StopApp::run()
{
    LOG_INFO << "Nohing to do, stopping";
    return 0;
}

}

