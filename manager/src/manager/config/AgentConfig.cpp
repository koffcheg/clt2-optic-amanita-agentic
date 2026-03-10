//
//  AgentConfig.cpp
//  manager
//
//  Created by apple on 24.05.2024.
//

#include "AgentConfig.h"
#include <boost/foreach.hpp>
#include <iostream>
#include "utils/Log.h"
#include <filesystem>

namespace cm
{

AgentConfig::AgentConfig()
{
}

AgentConfig::AgentConfig(boost::property_tree::ptree& pt)
{
    try
    {
        BOOST_FOREACH(boost::property_tree::ptree::value_type &v, pt)
        {
            if (v.first == "name")
                _name = v.second.data();
            if (v.first == "path")
                _path = v.second.data();
            if (v.first == "work_dir")
                _workDirectory = v.second.data();
        }

        if (_workDirectory.empty() && !_path.empty())
        {
            std::filesystem::path exePath = _path;
            exePath.remove_filename();
            _workDirectory = exePath;
        }
    }
    catch (const std::exception& e)
    {
        LOG_ERROR << e.what();
    }
}

AgentConfig::~AgentConfig()
{
    
}

}
