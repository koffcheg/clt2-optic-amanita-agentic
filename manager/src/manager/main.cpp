//
//  main.cpp
//  manager
//
//  Created by apple on 24.05.2024.
//

#include <iostream>
#include <boost/filesystem.hpp>
#include <memory>
#include "config/Config.h"
#include "config/Constants.h"
#include "app/App.h"
#include "utils/Log.h"


int main(int argc, const char * argv[])
{
    //LOG_INFO << argv[0];
    auto cliOptions = std::make_unique<cm::CLIOptions>(argc, argv);
    auto app = cm::App::create(*cliOptions);
    return app->run();
}
