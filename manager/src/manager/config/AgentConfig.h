//
//  AgentConfig.h
//  manager
//
//  Created by apple on 24.05.2024.
//

#pragma once

#include "ConfigInterfaces.h"
#include <boost/property_tree/ptree.hpp>

namespace cm
{
    class AgentConfig : public IAgentConfig
    {
    public:
        AgentConfig();
        AgentConfig(boost::property_tree::ptree& pt);
        ~AgentConfig();
        
        const std::string& name() const override { return _name; }
        const std::string& path() const override { return _path; }
        const std::string& workDirectory() const override { return _workDirectory; }

    private:
        std::string _name;
        std::string _path;
        std::string _workDirectory;
    };
}
