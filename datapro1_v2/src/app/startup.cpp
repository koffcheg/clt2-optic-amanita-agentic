#include "dp1v2/app/startup.hpp"

#include <filesystem>
#include <stdexcept>

#include <log4cxx/xml/domconfigurator.h>

#include "dp1v2/runtime/runtime.hpp"

namespace dp1v2 {

CliOptions parse_cli_options(const int argc, char *argv[]) {
    if (argc < 2) {
        throw std::logic_error("camera index is required");
    }
    if (argc < 3) {
        throw std::logic_error("application config path is required");
    }
    if (argc < 4) {
        throw std::logic_error("pipeline config path is required");
    }
    if (argc > 4) {
        throw std::logic_error("unexpected extra argument: " + std::string(argv[4]));
    }

    CliOptions options{};
    options.cam_index = std::stoi(argv[1]);
    options.application_config_path = argv[2];
    options.pipeline_config_path = argv[3];
    if (!options.application_config_path.empty() && options.application_config_path[0] == '-') {
        throw std::logic_error("application config path is required");
    }
    if (!options.pipeline_config_path.empty() && options.pipeline_config_path[0] == '-') {
        throw std::logic_error("pipeline config path is required");
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

void configure_logging_if_requested(const LoggingConfig &logging_config, const char *arg0, std::string &resolved_path) {
    if (!logging_config.enabled) {
        resolved_path.clear();
        return;
    }

    resolved_path = resolve_config_path(logging_config.config_file, arg0);
    const auto configuration_status = log4cxx::xml::DOMConfigurator::configureAndWatch(resolved_path);
    if (configuration_status == log4cxx::spi::ConfigurationStatus::NotConfigured) {
        throw std::logic_error("unable to configure logging subsystem from application.logging.config_file: " + resolved_path);
    }
}

StartupContext build_startup_context(const int argc, char *argv[]) {
    const auto options = parse_cli_options(argc, argv);
    const auto application_config_path = resolve_config_path(options.application_config_path, argv[0]);
    const auto pipeline_config_path = resolve_config_path(options.pipeline_config_path, argv[0]);

    StartupContext context{};
    context.cli = options;
    context.resolved_application_config_path = application_config_path;
    context.resolved_pipeline_config_path = pipeline_config_path;
    context.config = load_dp1_config(application_config_path, pipeline_config_path);
    configure_logging_if_requested(context.config.application.logging, argv[0], context.resolved_log_config_path);
    context.calibration = make_default_calibration_state();
    return context;
}

int run_startup(const int argc, char *argv[]) {
    const auto context = build_startup_context(argc, argv);
    const auto result = run_runtime_skeleton(context);
    return result.exit_code;
}

} // namespace dp1v2
