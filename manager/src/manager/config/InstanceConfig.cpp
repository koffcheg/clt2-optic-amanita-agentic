//
//  InstanceConfig.cpp
//  manager
//
//  Created by apple on 24.05.2024.
//

#include "InstanceConfig.h"
#include <boost/foreach.hpp>
#include <iostream>
#include "utils/Log.h"

namespace cm
{

void ParseStringArray(boost::property_tree::ptree& pt, std::vector<std::string>& sarr)
{
    BOOST_FOREACH(boost::property_tree::ptree::value_type &v, pt)
    {
        sarr.push_back(v.second.data());
    }
}

InstanceConfig::InstanceConfig()
    : _heartbeat (false)
{
}

InstanceConfig::InstanceConfig(boost::property_tree::ptree& pt)
    : _heartbeat (false)
{
    try
    {
        BOOST_FOREACH(boost::property_tree::ptree::value_type &v, pt)
        {
            if (v.first == "name")
                _name = v.second.data();
            if (v.first == "args")
            {
                ParseStringArray(v.second, _args);
            }
            if (v.first == "heartbeat")
            {
                std::string heartbeatStr = v.second.data();
                int heartbeatValue = std::atoi(heartbeatStr.c_str());
                _heartbeat = (heartbeatValue > 0);
            }
        }
    }
    catch (const std::exception& e)
    {
        LOG_ERROR << e.what();
    }
}

InstanceConfig::~InstanceConfig()
{
    
}

}
