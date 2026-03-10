//
//  CLIOptions.cpp
//  manager
//
//  Created by apple on 25.05.2024.
//

#include "CLIOptions.h"


namespace cm
{

class CLIOptionsParser
{
public:
    CLIOptionsParser(std::unordered_map<std::string, std::string>& m, std::vector<std::string>& l)
    : prev_key_(false)
    , m_(m)
    , l_(l) {}
    
    static bool IsArgumentKey(const std::string& argument)
    {
        return argument.find("--") == 0;
    }
    
    void add(const std::string& argument)
    {
        if (IsArgumentKey(argument))
        {
            if (prev_key_)
            {
                prev_key_ = false;
                l_.push_back(key_);
                m_[key_] = "";
                key_.clear();
            }
            
            prev_key_ = true;
            key_ = argument;
        }
        else
        {
            if (prev_key_)
            {
                prev_key_ = false;
                l_.push_back(key_);
                m_[key_] = argument;
                key_.clear();
            }
            else
            {
                l_.push_back(argument);
            }
        }
    }
    
    void finish()
    {
        if (prev_key_)
        {
            prev_key_ = false;
            l_.push_back(key_);
            m_[key_] = "";
            key_.clear();
        }
    }
    
private:
    bool prev_key_;
    std::string key_;
    std::vector<std::string>& l_;
    std::unordered_map<std::string, std::string>& m_;
};

CLIOptions::CLIOptions(int argc, const char* argv[])
{
    if (argc == 0 || argv == nullptr)
        return;
    
    path_ = std::string(argv[0]);
    
    CLIOptionsParser parser(map_, list_);
    
    for (int i = 1; i < argc; ++i)
    {
        parser.add(argv[i]);
    }
    
    parser.finish();
}

CLIOptions::~CLIOptions()
{
    
}

size_t CLIOptions::KeysCount() const
{
    return list_.size();
}

std::optional<std::string> CLIOptions::KeyAtIndex(size_t index) const
{
    if (index < list_.size())
        return list_[index];
    return std::nullopt;
}

std::optional<std::string> CLIOptions::ValueAtKey(const std::string& key) const
{
    auto w = map_.find(key);
    if (w != map_.end())
        return w->second;
    return std::nullopt;
}

std::string CLIOptions::ValueAtKey(const std::string& key, const std::string& def_value) const
{
    auto w = map_.find(key);
    if (w != map_.end())
        return w->second;
    return def_value;
}

}
