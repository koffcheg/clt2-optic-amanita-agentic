//
//  Config.cpp
//  manager
//
//  Created by apple on 24.05.2024.
//

#include "Config.h"
#include "AgentConfig.h"
#include "GroupConfig.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/foreach.hpp>
#include <iostream>
#include <filesystem>
#include <set>
#include "utils/Log.h"


namespace cm
{

template <class T>
static T& EmptyInstance()
{
    static T emptyInstance;
    return emptyInstance;
}

static void ParseAgents(boost::property_tree::ptree root, std::vector<std::unique_ptr<IAgentConfig>>& agents)
{
    BOOST_FOREACH(boost::property_tree::ptree::value_type &v, root)
    {
        auto agent = std::make_unique<AgentConfig>(v.second);
        agents.push_back(std::move(agent));
    }
}

static void ParseGroups(boost::property_tree::ptree root, std::vector<std::unique_ptr<IGroupConfig>>& groups)
{
    BOOST_FOREACH(boost::property_tree::ptree::value_type &v, root)
        groups.push_back(std::make_unique<GroupConfig>(v.second));
}

static bool FileExists(const std::string& path)
{
    try
    {
        return std::filesystem::exists(path);
    }
    catch (const std::exception& e)
    {
        LOG_ERROR << e.what();
        return false;
    }
}

Config::Config(const std::string& path)
{
    boost::property_tree::ptree pt;
    boost::property_tree::read_json(path, pt);
    
    try
    {
        BOOST_FOREACH(boost::property_tree::ptree::value_type &v, pt)
        {
            if (v.first == "agents")
                ParseAgents(v.second, _agents);
            
            if (v.first == "groups")
                ParseGroups(v.second, _groups);
        }
    }
    catch (const std::exception& e)
    {
        _agents.clear();
        _groups.clear();
        LOG_ERROR << e.what();
    }
    
    if (!Validate())
    {
        _agents.clear();
        _groups.clear();
    }
}

Config::~Config()
{
    
}

const IAgentConfig& Config::agentAtIndex(size_t index) const
{
    if (index >= _agents.size())
        return EmptyInstance<AgentConfig>();
    
    return *_agents[index];
}

const IGroupConfig& Config::groupAtIndex(size_t index) const
{
    if (index >= _groups.size())
        return EmptyInstance<GroupConfig>();

    return *_groups[index];
}

bool Config::Validate()
{
    for (auto& agent : _agents)
    {
        if (agent->name().empty())
        {
            LOG_DEBUG << "Config::Validate failed: agent name is empty";
            return false;
        }

        if (_agentByName.find(agent->name()) != _agentByName.end())
        {
            LOG_DEBUG << "Config::Validate failed: duplicated agent name: " << agent->name();
            return false;
        }

        if (!FileExists(agent->path()))
        {
            LOG_DEBUG << "Config::Validate failed: path doesn't exist: " << agent->path();
            //return false;
        }
        
        _agentByName[agent->name()] = agent.get();
    }
    
    std::set<std::string> groupNames;
    
    for (auto& group : _groups)
    {
        if (group->name().empty())
        {
            LOG_DEBUG << "Config::Validate failed: group name is empty";
            return false;
        }
        
        if (groupNames.find(group->name()) != groupNames.end())
        {
            LOG_DEBUG << "Config::Validate failed: duplicated group name: " << group->name();
            return false;
        }

        if (group->source() == nullptr || _agentByName.find(group->source()->name()) == _agentByName.end())
        {
            LOG_DEBUG << "Config::Validate failed: no source " << group->source() << " for group " << group->name();
            return false;
        }

        for (auto& destnation : group->destinations())
        {
            if (destnation == nullptr || _agentByName.find(destnation->name()) == _agentByName.end())
            {
                LOG_DEBUG << "Config::Validate failed: no destination " << destnation << " for group " << group->name();
                return false;
            }
        }
        
        groupNames.insert(group->name());
    }
    
    return true;
}

const IAgentConfig& Config::agentByName(const std::string& name) const
{
    const auto w = _agentByName.find(name);
    return (w == _agentByName.end()) ? (EmptyInstance<AgentConfig>()) : (*w->second);
}

}
