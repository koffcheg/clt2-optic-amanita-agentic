//
//  CLIOptions.h
//  manager
//
//  Created by apple on 25.05.2024.
//

#pragma once

#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace cm
{
    class CLIOptions
    {
    public:
        CLIOptions(int argc, const char* argv[]);
        ~CLIOptions();
        
        const std::string& path() const { return path_; }
        size_t KeysCount() const;
        std::optional<std::string> KeyAtIndex(size_t index) const;
        std::optional<std::string> ValueAtKey(const std::string& key) const;
        std::string ValueAtKey(const std::string& key, const std::string& def_value) const;
        
    private:
        std::string path_;
        std::vector<std::string> list_;
        std::unordered_map<std::string, std::string> map_;
    };
}
