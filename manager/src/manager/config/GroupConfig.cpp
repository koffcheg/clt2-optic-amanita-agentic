//
//  GroupConfig.cpp
//  manager
//
//  Created by apple on 24.05.2024.
//

#include "GroupConfig.h"
#include <boost/foreach.hpp>
#include <iostream>
#include "utils/Log.h"

namespace cm
{

void ParseInstanceArray(boost::property_tree::ptree& pt, std::vector<std::unique_ptr<IInstanceConfig>>& sarr)
{
    BOOST_FOREACH(boost::property_tree::ptree::value_type &v, pt)
    {
        sarr.push_back(std::make_unique<InstanceConfig>(v.second));
    }
}

GroupConfig::GroupConfig()
{
}

GroupConfig::GroupConfig(boost::property_tree::ptree& pt)
{
    try
    {
        BOOST_FOREACH(boost::property_tree::ptree::value_type &v, pt)
        {
            if (v.first == "name")
                _name = v.second.data();
            if (v.first == "source")
                _source = std::make_unique<InstanceConfig>(v.second);
            if (v.first == "destinations")
                ParseInstanceArray(v.second, _destinations);
        }
    }
    catch (const std::exception& e)
    {
        LOG_ERROR << e.what();
    }

}

GroupConfig::~GroupConfig()
{
    
}

}
