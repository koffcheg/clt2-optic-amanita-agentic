#pragma once

#include <string>

namespace dp1v2 {

struct CliOptions {
    int cam_index = -1;
    std::string config_path;
};

CliOptions parse_cli_options(int argc, char *argv[]);
std::string resolve_config_path(const std::string &config_path, const char *arg0);
int run_startup(int argc, char *argv[]);

} // namespace dp1v2
