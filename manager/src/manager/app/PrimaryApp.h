//
//  PrimaryApp.h
//  manager
//
//  Created by apple on 25.05.2024.
//

#pragma once

#include "App.h"
#include "Agent.h"
#include "IPMessageQueue.h"

namespace cm
{
    class PrimaryApp : public App
    {
    public:
        PrimaryApp(const CLIOptions& options, std::unique_ptr<Environment> env, std::unique_ptr<Config> config);
        ~PrimaryApp();
        
        int run() override;
        void close();
        
    private:
        bool processSingleMessage();
        bool waitUntilFinished(int milliseconds);
        bool finished() const;
        IPMessage buildPongMessage() const;
        
        std::unique_ptr<Environment> _env;
        std::unique_ptr<Config> _config;
        std::vector<std::unique_ptr<Agent>> _agents;
        IPMessageQueue _messageQueue;
        bool _running;
    };
}
