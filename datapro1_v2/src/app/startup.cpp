#include "dp1v2/app/startup.hpp"

#include <filesystem>
#include <stdexcept>

#include <log4cxx/xml/domconfigurator.h>

#include "dp1v2/stages/calibration.hpp"
#include "dp1v2/config/config.hpp"
#include "dp1v2/runtime/runtime.hpp"

namespace dp1v2 {

CliOptions parse_cli_options(const int argc, char *argv[]) {
    if (argc < 2) {
        throw std::logic_error("camera index is required");
    }
    if (argc < 3) {
        throw std::logic_error("config path is required");
    }

    CliOptions options{};
    options.cam_index = std::stoi(argv[1]);
    options.config_path.clear();
    options.log_config_path.clear();

    for (int i = 2; i < argc; ++i) {
        const std::string arg = argv[i];
        if (!arg.empty() && arg[0] == '-') {
            throw std::logic_error("unknown option: " + arg);
        }

        if (options.config_path.empty()) {
            options.config_path = arg;
            continue;
        }

        if (options.log_config_path.empty()) {
            options.log_config_path = arg;
            continue;
        }

        throw std::logic_error("unexpected extra argument: " + arg);
    }

    if (options.config_path.empty()) {
        throw std::logic_error("config path is required");
    }

    return options;
}

std::string resolve_config_path(const std::string &config_path, const char *arg0) {
    namespace fs = std::filesystem;
    fs::path direct_candidate = config_path;
    if (direct_candidate.is_absolute() && fs::exists(direct_candidate)) {
        return direct_candidate.string();
    }

    if (!direct_candidate.is_absolute() && fs::exists(direct_candidate)) {
        return direct_candidate.string();
    }

    const fs::path exec_path = fs::absolute(arg0).parent_path();
    const fs::path from_binary_dir = exec_path / config_path;
    if (fs::exists(from_binary_dir)) {
        return from_binary_dir.string();
    }

    return config_path;
}

void configure_logging_if_requested(const std::string &log_config_path) {
    if (log_config_path.empty()) {
        return;
    }

    const auto configuration_status = log4cxx::xml::DOMConfigurator::configureAndWatch(log_config_path);
    if (configuration_status == log4cxx::spi::ConfigurationStatus::NotConfigured) {
        throw std::logic_error("unable to configure logging subsystem from cfg.file: " + log_config_path);
    }
}

StartupContext build_startup_context(const int argc, char *argv[]) {
    const auto options = parse_cli_options(argc, argv);
    const auto config_path = resolve_config_path(options.config_path, argv[0]);
    const auto log_config_path = options.log_config_path.empty()
        ? std::string{}
        : resolve_config_path(options.log_config_path, argv[0]);
    configure_logging_if_requested(log_config_path);

    StartupContext context{};
    context.cli = options;
    context.resolved_config_path = config_path;
    context.resolved_log_config_path = log_config_path;
    context.config = load_runtime_config(config_path);
    context.calibration = load_calibration_state(context.config.calibration);
    return context;
}

int run_startup(const int argc, char *argv[]) {
    const auto context = build_startup_context(argc, argv);
    const auto result = run_runtime_skeleton(context);
    return result.exit_code;
}

} // namespace dp1v2
