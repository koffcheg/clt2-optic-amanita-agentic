//
//  App.h
//  manager
//
//  Created by apple on 25.05.2024.
//

#pragma once

#include "Config.h"
#include "CLIOptions.h"
#include "Environment.h"
#include <memory>

namespace cm
{
    class App
    {
    public:
        App() {}
        virtual ~App() {}
        
        virtual int run() = 0;
        
        static std::unique_ptr<App> create(const CLIOptions& options);
    private:
    };
}

