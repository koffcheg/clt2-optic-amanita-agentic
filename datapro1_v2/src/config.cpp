#include "dp1v2/config.hpp"

#include <algorithm>
#include <array>
#include <stdexcept>

#include "m_json_cfg_reader.h"
#include "m_json_unique_ptr.h"

namespace {

constexpr std::array<const char *, 2> kAllowedSources{
    "camera",
    "imagefile",
};

bool is_allowed_source(const std::string &source) {
    return std::find(kAllowedSources.begin(), kAllowedSources.end(), source) != kAllowedSources.end();
}

bool source_requires_link(const std::string &source) {
    return source == "imagefile";
}

void validate_source(const dp1v2::SourceConfig &source_cfg) {
    if (!is_allowed_source(source_cfg.source)) {
        throw std::logic_error("error on config file, unsupported source.source: " + source_cfg.source);
    }

    if (source_requires_link(source_cfg.source) && source_cfg.link.empty()) {
        throw std::logic_error("error on config file, source.link is required for source: " + source_cfg.source);
    }
}

void validate_dp2_conn(const dp1v2::Dp2ConnConfig &dp2_cfg) {
    if (dp2_cfg.host.empty()) {
        throw std::logic_error("error on config file, dp2conn.host must be non-empty");
    }

    if (dp2_cfg.port == 0) {
        throw std::logic_error("error on config file, dp2conn.port must be in range [1..65535]");
    }

    if (dp2_cfg.reconnect_interval_s < 1) {
        throw std::logic_error("error on config file, dp2conn.reconnect_interval_s must be >= 1");
    }
}

} // namespace

namespace dp1v2 {

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
        cfg.source.link = source_reader.read_string_param("link", "");
    }
    {
        jansson_cfg_obj_reader dp2_reader(json_cfg, "dp2conn");
        cfg.dp2_conn.host = dp2_reader.read_string_param("host", "127.0.0.1");
        cfg.dp2_conn.port = static_cast<std::uint16_t>(dp2_reader.read_int_param("port", 11511));
        cfg.dp2_conn.reconnect_interval_s = dp2_reader.read_int_param("reconnect_interval_s", 3);
    }

    validate_source(cfg.source);
    validate_dp2_conn(cfg.dp2_conn);

    return cfg;
}

} // namespace dp1v2
