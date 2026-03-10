//
//  Environment.h
//  manager
//
//  Created by apple on 25.05.2024.
//

#pragma once

#include <string>

namespace cm
{
    class Environment
    {
    public:
        enum class RunMode
        {
            Primary,
            Secondary,
            Failed
        };
    public:
        Environment();
        ~Environment();

        RunMode runMode() const { return _runMode; }
    private:
        RunMode _runMode;
        int _fileHandle;
    };
}

