//
//  StopApp.h
//  manager
//
//  Created by apple on 25.05.2024.
//

#pragma once


#include "App.h"

namespace cm
{
    class StopApp : public App
    {
    public:
        StopApp(const CLIOptions& options);
        ~StopApp();
        
        int run() override;
        
    private:
    };
}
