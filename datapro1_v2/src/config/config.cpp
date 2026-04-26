#include "dp1v2/config/config.hpp"

#include <algorithm>
#include <array>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <limits.h>
#include <stdexcept>
#include <unistd.h>

#include "m_json_cfg_reader.h"
#include "m_json_unique_ptr.h"

namespace {

constexpr std::array<const char *, 6> kAllowedSources{
    "campro",
    "camera",
    "webcam",
    "ipcam",
    "videofile",
    "imagefile",
};

std::string trim_copy(std::string value) {
    const auto begin = value.find_first_not_of(" \t\r\n");
    if (begin == std::string::npos) {
        return {};
    }
    const auto end = value.find_last_not_of(" \t\r\n");
    return value.substr(begin, end - begin + 1);
}

std::filesystem::path get_executable_path() {
    char buffer[PATH_MAX]{};
    const auto length = ::readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
    if (length <= 0) {
        return std::filesystem::current_path();
    }
    buffer[length] = '\0';
    return std::filesystem::path(buffer);
}

std::filesystem::path find_repo_root() {
    auto current = get_executable_path().parent_path();
    while (!current.empty()) {
        if (std::filesystem::exists(current / "AGENTS.md") &&
            std::filesystem::exists(current / "CMakeLists.txt")) {
            return current;
        }
        if (current == current.root_path()) {
            break;
        }
        current = current.parent_path();
    }
    return std::filesystem::current_path();
}

bool try_read_resources_root(const std::filesystem::path &cfg_file,
                             std::filesystem::path &result,
                             const std::filesystem::path &repo_root) {
    std::ifstream input(cfg_file);
    if (!input) {
        return false;
    }

    std::string line;
    while (std::getline(input, line)) {
        line = trim_copy(line);
        if (line.empty() || line[0] == '#') {
            continue;
        }

        const auto sep_pos = line.find('=');
        if (sep_pos == std::string::npos) {
            continue;
        }

        const auto key = trim_copy(line.substr(0, sep_pos));
        const auto value = trim_copy(line.substr(sep_pos + 1));
        if (key != "resources_root" || value.empty()) {
            continue;
        }

        std::filesystem::path candidate(value);
        if (candidate.is_relative()) {
            candidate = repo_root / candidate;
        }
        result = candidate.lexically_normal();
        return true;
    }

    return false;
}

std::filesystem::path get_resources_root() {
    if (const char *env_root = std::getenv("AMANITA_RESOURCES_DIR"); env_root && *env_root) {
        return std::filesystem::path(env_root);
    }

    const auto repo_root = find_repo_root();
    std::filesystem::path from_cfg;
    if (try_read_resources_root(repo_root / "amanita_resources.local.conf", from_cfg, repo_root)) {
        return from_cfg;
    }
    if (try_read_resources_root(repo_root / "amanita_resources.conf", from_cfg, repo_root)) {
        return from_cfg;
    }

    return (repo_root / "AmanitaResources").lexically_normal();
}

std::string resolve_resource_path(const std::string &raw_path) {
    static const std::string token{"${AMANITA_RESOURCES_DIR}"};
    if (raw_path.compare(0, token.size(), token) != 0) {
        return raw_path;
    }

    auto suffix = raw_path.substr(token.size());
    while (!suffix.empty() && suffix.front() == '/') {
        suffix.erase(suffix.begin());
    }

    return (get_resources_root() / suffix).lexically_normal().string();
}

bool is_allowed_source(const std::string &source) {
    return std::find(kAllowedSources.begin(), kAllowedSources.end(), source) != kAllowedSources.end();
}

bool source_requires_link(const std::string &source) {
    return source == "ipcam" || source == "videofile" || source == "imagefile";
}

dp1v2::SourceKind resolve_source_kind(const std::string &source) {
    if (source == "campro" || source == "camera") {
        return dp1v2::SourceKind::CamproIpc;
    }

    if (source == "webcam" || source == "ipcam" || source == "videofile" || source == "imagefile") {
        return dp1v2::SourceKind::UriFile;
    }

    throw std::logic_error("error on config file, unsupported source.source: " + source);
}

void validate_source(const dp1v2::SourceConfig &source_cfg) {
    if (!is_allowed_source(source_cfg.source)) {
        throw std::logic_error("error on config file, unsupported source.source: " + source_cfg.source);
    }

    if (source_requires_link(source_cfg.source) && source_cfg.link.empty()) {
        throw std::logic_error("error on config file, source.link is required for source: " + source_cfg.source);
    }

    if (source_cfg.ipc_receive_timeout_ms < dp1v2::kMinIpcReceiveTimeoutMs) {
        throw std::logic_error("error on config file, source.ipc_receive_timeout_ms must be >= 1");
    }
}

void validate_dp2_conn(const dp1v2::Dp2ConnConfig &dp2_cfg) {
    if (dp2_cfg.host.empty()) {
        throw std::logic_error("error on config file, dp2conn.host must be non-empty");
    }

    if (dp2_cfg.port < dp1v2::kMinTcpPort) {
        throw std::logic_error("error on config file, dp2conn.port must be in range [1..65535]");
    }

    if (dp2_cfg.reconnect_interval_s < dp1v2::kMinDp2ReconnectIntervalS) {
        throw std::logic_error("error on config file, dp2conn.reconnect_interval_s must be >= 1");
    }
}

void validate_artifacts(const dp1v2::ArtifactConfig &artifact_cfg) {
    if (artifact_cfg.enabled && artifact_cfg.out_folder.empty()) {
        throw std::logic_error("error on config file, artifacts.out_folder must be non-empty when artifacts.enabled is true");
    }

    if (artifact_cfg.enabled && artifact_cfg.data_bin_folder.empty()) {
        throw std::logic_error("error on config file, artifacts.data_bin_folder must be non-empty when artifacts.enabled is true");
    }
}

void validate_runtime_loop(const dp1v2::RuntimeLoopConfig &loop_cfg) {
    if (loop_cfg.max_empty_reads < dp1v2::kMinRuntimeMaxEmptyReads) {
        throw std::logic_error("error on config file, runtime_loop.max_empty_reads must be >= 1");
    }
}

void validate_calibration(const dp1v2::CalibrationConfig &calibration_cfg) {
    if (calibration_cfg.required && calibration_cfg.camera_settings_file.empty()) {
        throw std::logic_error("error on config file, calibration.camera_settings_file must be non-empty when calibration.required is true");
    }
}

} // namespace

namespace dp1v2 {

const char *source_kind_to_cstr(const SourceKind kind) {
    switch (kind) {
        case SourceKind::CamproIpc:
            return "campro_ipc";
        case SourceKind::UriFile:
            return "uri_file";
        default:
            return "unknown";
    }
}

RuntimeConfig load_runtime_config(const std::string &config_path) {
    auto root = json_unique_ptr_create(json_load_file(config_path.c_str(), 0, nullptr));
    if (!root) {
        throw std::logic_error("error on opening config file: " + config_path);
    }

    const auto *json_cfg = json_object_get(root.get(), "config");
    if (!json_cfg) {
        throw std::logic_error("error on config file, there is no root-node 'config'");
    }

    RuntimeConfig cfg{};
    {
        jansson_cfg_obj_reader source_reader(json_cfg, "source");
        cfg.source.source = source_reader.read_string_param("source");
        cfg.source.link = resolve_resource_path(source_reader.read_string_param("link", ""));
        cfg.source.ipc_receive_timeout_ms = source_reader.read_int_param("ipc_receive_timeout_ms", kDefaultIpcReceiveTimeoutMs);
        cfg.source.kind = resolve_source_kind(cfg.source.source);
    }
    {
        jansson_cfg_obj_reader dp2_reader(json_cfg, "dp2conn");
        cfg.dp2_conn.host = dp2_reader.read_string_param("host", kDefaultDp2Host);
        const int port = dp2_reader.read_int_param("port", kDefaultDp2Port);
        if (port < kMinTcpPort || port > kMaxTcpPort) {
            throw std::logic_error("error on config file, dp2conn.port must be in range [1..65535]");
        }
        cfg.dp2_conn.port = static_cast<std::uint16_t>(port);
        cfg.dp2_conn.reconnect_interval_s = dp2_reader.read_int_param("reconnect_interval_s", kDefaultDp2ReconnectIntervalS);
    }
    if (json_object_get(json_cfg, "artifacts")) {
        jansson_cfg_obj_reader artifact_reader(json_cfg, "artifacts");
        cfg.artifacts.enabled = artifact_reader.read_bool_param("enabled", false);
        cfg.artifacts.json_enabled = artifact_reader.read_bool_param("json_enabled", false);
        cfg.artifacts.binocular_enabled = artifact_reader.read_bool_param("binocular_enabled", false);
        cfg.artifacts.out_folder = resolve_resource_path(artifact_reader.read_string_param("out_folder", cfg.artifacts.out_folder));
        cfg.artifacts.data_bin_folder = artifact_reader.read_string_param("data_bin_folder", cfg.artifacts.data_bin_folder);
    }
    if (json_object_get(json_cfg, "runtime_loop")) {
        jansson_cfg_obj_reader loop_reader(json_cfg, "runtime_loop");
        const int max_frames = loop_reader.read_int_param("max_frames", static_cast<int>(kDefaultRuntimeMaxFrames));
        const int max_empty_reads = loop_reader.read_int_param("max_empty_reads", static_cast<int>(kDefaultRuntimeMaxEmptyReads));
        if (max_frames < 0) {
            throw std::logic_error("error on config file, runtime_loop.max_frames must be >= 0");
        }
        if (max_empty_reads < static_cast<int>(kMinRuntimeMaxEmptyReads)) {
            throw std::logic_error("error on config file, runtime_loop.max_empty_reads must be >= 1");
        }
        cfg.runtime_loop.max_frames = static_cast<std::uint64_t>(max_frames);
        cfg.runtime_loop.max_empty_reads = static_cast<std::uint64_t>(max_empty_reads);
    }
    if (json_object_get(json_cfg, "calibration")) {
        jansson_cfg_obj_reader calibration_reader(json_cfg, "calibration");
        cfg.calibration.required = calibration_reader.read_bool_param("required", false);
        cfg.calibration.camera_settings_file = resolve_resource_path(calibration_reader.read_string_param("camera_settings_file", ""));
    }

    validate_source(cfg.source);
    validate_dp2_conn(cfg.dp2_conn);
    validate_artifacts(cfg.artifacts);
    validate_runtime_loop(cfg.runtime_loop);
    validate_calibration(cfg.calibration);

    return cfg;
}

} // namespace dp1v2
