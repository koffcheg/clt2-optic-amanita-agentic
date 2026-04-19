#include "dp1v2/startup.hpp"

#include <filesystem>
#include <stdexcept>

#include "dp1v2/config.hpp"

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

    for (int i = 2; i < argc; ++i) {
        const std::string arg = argv[i];
        if (!arg.empty() && arg[0] == '-') {
            throw std::logic_error("unknown option: " + arg);
        }

        if (!options.config_path.empty()) {
            throw std::logic_error("config path is already set: " + options.config_path);
        }

        options.config_path = arg;
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

int run_startup(const int argc, char *argv[]) {
    const auto options = parse_cli_options(argc, argv);
    const auto config_path = resolve_config_path(options.config_path, argv[0]);
    (void)load_runtime_config(config_path);
    (void)options.cam_index;
    return 0;
}

} // namespace dp1v2
