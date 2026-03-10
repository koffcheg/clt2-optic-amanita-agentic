//
//  ConfigInterfaces.h
//  manager
//
//  Created by apple on 24.05.2024.
//

#pragma once

#include <string>
#include <vector>

namespace cm
{
    class IAgentConfig
    {
    public:
        virtual ~IAgentConfig() {}
        
        virtual const std::string& name() const = 0;
        virtual const std::string& path() const = 0;
        virtual const std::string& workDirectory() const = 0;
    };

    class IInstanceConfig
    {
    public:
        virtual ~IInstanceConfig() {}
        virtual const std::string& name() const = 0;
        virtual bool heartbeat() const = 0;
        virtual const std::vector<std::string>& args() const = 0;
    };

    class IGroupConfig
    {
    public:
        virtual ~IGroupConfig() {}

        virtual const std::string& name() const = 0;
        virtual const IInstanceConfig* source() const = 0;
        virtual const std::vector<const IInstanceConfig*> destinations() const = 0;
    };
}
