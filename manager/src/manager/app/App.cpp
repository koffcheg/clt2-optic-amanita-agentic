//
//  App.cpp
//  manager
//
//  Created by apple on 25.05.2024.
//

#include "App.h"
#include "PrimaryApp.h"
#include "CLIApp.h"
#include "StopApp.h"
#include "Constants.h"
#include "utils/Log.h"

#include "Action.h"

namespace cm
{

static bool IsDaemon(const CLIOptions& options)
{
    auto first_key = options.KeyAtIndex(0);
    if (!first_key.has_value())
        return false;
    
    return first_key == "--daemon";
}

std::unique_ptr<App> App::create(const CLIOptions& options)
{
    if (IsDaemon(options))
    {
        ConfigureLogs(cm::manager_log_file_path);

        auto environment = std::make_unique<cm::Environment>();
        if (environment->runMode() == Environment::RunMode::Primary)
        {
            auto config = std::make_unique<cm::Config>(manager_config_file_path);
            return std::make_unique<PrimaryApp>(options, std::move(environment), std::move(config));
        }
    }
    else
    {
        ConfigureLogs(nullptr);
        auto action = Action::create(options);
        
        if (action)
        {
            return std::make_unique<CLIApp>(options, std::move(action));
        }
    }
    return std::make_unique<StopApp>(options);
}

}
