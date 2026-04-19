#pragma once

#include <cstdint>
#include <string>

namespace dp1v2 {

struct SourceConfig {
    std::string source;
    std::string link;
};

struct Dp2ConnConfig {
    std::string host = "127.0.0.1";
    std::uint16_t port = 11511;
    int reconnect_interval_s = 3;
};

struct RuntimeConfig {
    SourceConfig source;
    Dp2ConnConfig dp2_conn;
};

RuntimeConfig load_runtime_config(const std::string &config_path);

} // namespace dp1v2
