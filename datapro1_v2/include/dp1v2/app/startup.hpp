#pragma once

#include <string>

#include "dp1v2/stages/calibration.hpp"
#include "dp1v2/config/config.hpp"

namespace dp1v2 {

struct CliOptions {
    int cam_index = -1;
    std::string config_path;
    std::string log_config_path;
};

struct StartupContext {
    CliOptions cli;
    std::string resolved_config_path;
    std::string resolved_log_config_path;
    RuntimeConfig config;
    CalibrationState calibration;
};

CliOptions parse_cli_options(int argc, char *argv[]);
std::string resolve_config_path(const std::string &config_path, const char *arg0);
StartupContext build_startup_context(int argc, char *argv[]);
int run_startup(int argc, char *argv[]);

} // namespace dp1v2
