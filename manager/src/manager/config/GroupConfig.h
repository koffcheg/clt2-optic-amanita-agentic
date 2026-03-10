//
//  GroupConfig.h
//  manager
//
//  Created by apple on 24.05.2024.
//

#pragma once

#include "ConfigInterfaces.h"
#include "InstanceConfig.h"
#include <boost/property_tree/ptree.hpp>
#include <vector>

namespace cm
{
    class GroupConfig : public IGroupConfig
    {
    public:
        GroupConfig();
        GroupConfig(boost::property_tree::ptree& pt);
        ~GroupConfig();
        
        const std::string& name() const override { return _name; }
        const IInstanceConfig* source() const override { return _source.get(); }
        const std::vector<const IInstanceConfig*> destinations() const override 
        {
            std::vector<const IInstanceConfig*> res;
            for (auto& d : _destinations)
                res.push_back(d.get());
            return res;
        }
    private:
        std::string _name;
        std::unique_ptr<IInstanceConfig> _source;
        std::vector<std::unique_ptr<IInstanceConfig>> _destinations;
    };
}

