//
//  Config.h
//  manager
//
//  Created by apple on 24.05.2024.
//

#pragma once

#include <string>
#include <vector>
#include <memory>
#include <map>
#include "ConfigInterfaces.h"

namespace cm
{
    class Config
    {
    public:
        Config(const std::string& path);
        ~Config();
        
        size_t agentsCount() const { return _agents.size(); }
        const IAgentConfig& agentAtIndex(size_t index) const;
        
        size_t groupsCount() const { return _groups.size(); }
        const IGroupConfig& groupAtIndex(size_t index) const;
        
        bool empty() const { return _agents.size() == 0; }
        
        const IAgentConfig& agentByName(const std::string& name) const;
        
    private:
        
        bool Validate();
        
    private:
        
        std::vector<std::unique_ptr<IAgentConfig>> _agents;
        std::vector<std::unique_ptr<IGroupConfig>> _groups;
        std::map<std::string, IAgentConfig*> _agentByName;
    };
}


