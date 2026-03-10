//
//  InstanceConfig.h
//  manager
//
//  Created by apple on 24.05.2024.
//

#pragma once

#include "ConfigInterfaces.h"
#include <boost/property_tree/ptree.hpp>
#include <vector>

namespace cm
{
    class InstanceConfig : public IInstanceConfig
    {
    public:
        InstanceConfig();
        InstanceConfig(boost::property_tree::ptree& pt);
        ~InstanceConfig();
        
        const std::string& name() const override { return _name; }
        bool heartbeat() const override { return _heartbeat; }
        const std::vector<std::string>& args() const override { return _args; }

    private:
        std::string _name;
        bool _heartbeat;
        std::vector<std::string> _args;
    };
}
