//
//  PrimaryApp.cpp
//  manager
//
//  Created by apple on 25.05.2024.
//

#include "PrimaryApp.h"
#include "Agent.h"
#include <thread>
#include <sstream>
#include <filesystem>
#include "Constants.h"
#include "utils/Log.h"

namespace cm
{

PrimaryApp::PrimaryApp(const CLIOptions& options, std::unique_ptr<Environment> env, std::unique_ptr<Config> config)
    : _config(std::move(config))
    , _messageQueue(true)
    , _running(true)
    , _env(std::move(env))
{
    LOG_INFO << "Initialized";
    size_t cnt = _config->groupsCount();
    for (size_t i = 0; i < cnt; ++i)
    {
        const auto& groupConfig = _config->groupAtIndex(i);
        {
            const auto& agentConfig = _config->agentByName(groupConfig.source()->name());
            RunParams params;
            params.name = agentConfig.name();
            params.path = agentConfig.path();
            params.workDir = agentConfig.workDirectory();
            params.group = groupConfig.name();
            params.destinationId = std::nullopt;
            params.heartbeat = groupConfig.source()->heartbeat();
            params.args = groupConfig.source()->args();
            
            _agents.push_back(std::make_unique<Agent>(params));
        }
        
        int destinationCount = 0;
        for (auto destination : groupConfig.destinations())
        {
            const auto& agentConfig = _config->agentByName(destination->name());
            RunParams params;
            params.name = agentConfig.name();
            params.path = agentConfig.path();
            params.workDir = agentConfig.workDirectory();
            params.group = groupConfig.name();
            params.destinationId = std::to_string(destinationCount++);
            params.heartbeat = destination->heartbeat();
            params.args = destination->args();
            
            _agents.push_back(std::make_unique<Agent>(params));
        }
    }
}

PrimaryApp::~PrimaryApp()
{
    
}

int PrimaryApp::run()
{
    if (!std::filesystem::exists(manager_enabled_file_path))
    {
        LOG_INFO << "File toggle absent, exiting";
        return 0;
    }
    
    LOG_INFO << "Starting agents";

    for (auto& agent : _agents)
        agent->start();
    
    while (_running)
    {
        while (processSingleMessage()) {}
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    LOG_INFO << "Stopping agents";
    
    for (auto& agent : _agents)
        agent->stop();
    
    LOG_INFO << "Waiting...";

    if (!waitUntilFinished(5000))
    {
        LOG_INFO << "Agents haven't stopped, terminating";
        return -1;
    }

    LOG_INFO << "Gracefully shutting down";
    
    return 0;
}

void PrimaryApp::close()
{
    LOG_INFO << "Closing by signal";
    _running = false;
}

bool PrimaryApp::processSingleMessage()
{
    IPMessage message;
    if (!_messageQueue.receive(message))
        return false;

    LOG_INFO << "Got message: " << (int)message.type;
    
    switch (message.type)
    {
        case IPMessageType_Unknown:
            break;
        case IPMessageType_Stop:
            close();
            break;
        case IPMessageType_Ping:
            _messageQueue.send(buildPongMessage());
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            break;
        default:
            break;
    }
    
    return true;
}

bool PrimaryApp::finished() const
{
    for (const auto& agent : _agents)
    {
        if (!agent->finished())
            return false;
    }
    return true;
}

IPMessage PrimaryApp::buildPongMessage() const
{
    std::map<std::string, int> agentsMap;

    for (const auto& agentPtr : _agents)
    {
        std::string name = agentPtr->name();
        if (agentsMap.find(name) == agentsMap.end())
            agentsMap[name] = 0;
        else
            agentsMap[name] = agentsMap[name] + 1;
    }

    std::stringstream ss;
    auto b = agentsMap.begin();
    auto e = agentsMap.end();

    for (auto i = b; i != e; ++i)
    {
        if (i != b)
            ss << ",";
        ss << i->first << ":" << i->second;
    }

    std::string result = ss.str();
    const char* ptr = result.c_str();

    IPMessage message(IPMessageType_Pong);
    size_t length = std::min(kIPMessageBufferSize - 1, strlen(ptr));
    memcpy(message.message, ptr, length);
    message.message[length] = '\0';

    return message;
}

bool PrimaryApp::waitUntilFinished(int milliseconds)
{
    auto start = std::chrono::system_clock::now();
    
    for (;;)
    {
        if (finished())
            return true;
        
        auto now = std::chrono::system_clock::now();
        
        int elapsed = (int)std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count();
        if (elapsed > milliseconds)
            return false;
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    return false;
}

}
