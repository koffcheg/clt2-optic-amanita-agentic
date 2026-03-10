//
//  StreamAnalyzer.h
//  manager
//
//  Created by apple on 27.05.2024.
//

#pragma once

#include <string>

namespace cm
{
    class StreamAnalyzer
    {
    public:
        enum class Result
        {
            None,
            CancelProcess
        };
    public:
        virtual ~StreamAnalyzer() {}
        virtual Result analyze(const std::string& s) = 0;
    };
}
