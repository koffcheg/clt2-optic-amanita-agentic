#include "dp1v2/config/config.hpp"

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <limits.h>
#include <limits>
#include <set>
#include <stdexcept>
#include <string_view>
#include <unistd.h>
#include <vector>

#include <jansson.h>

#include "m_json_unique_ptr.h"

namespace {

constexpr std::string_view kSupportedSchemaVersion = "1.0";

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

bool contains(const std::vector<std::string_view> &values, const std::string_view value) {
    return std::find(values.begin(), values.end(), value) != values.end();
}

void reject_unknown_keys(const json_t *object,
                         const std::vector<std::string_view> &allowed,
                         const std::string &context) {
    const char *key = nullptr;
    json_t *value = nullptr;
    json_object_foreach(const_cast<json_t *>(object), key, value) {
        (void)value;
        if (!contains(allowed, key)) {
            throw std::logic_error("error on config file, " + context + " has unknown key: " + key);
        }
    }
}

const json_t *read_required_object(const json_t *object, const char *key, const std::string &context) {
    const json_t *value = json_object_get(object, key);
    if (!json_is_object(value)) {
        throw std::logic_error("error on config file, " + context + "." + key + " is required object");
    }
    return value;
}

std::string read_required_string(const json_t *object, const char *key, const std::string &context) {
    const json_t *value = json_object_get(object, key);
    if (!json_is_string(value)) {
        throw std::logic_error("error on config file, " + context + "." + key + " is required string");
    }
    return json_string_value(value);
}

std::string read_optional_string(const json_t *object,
                                 const char *key,
                                 const std::string &default_value,
                                 const std::string &context) {
    const json_t *value = json_object_get(object, key);
    if (!value) {
        return default_value;
    }
    if (!json_is_string(value)) {
        throw std::logic_error("error on config file, " + context + "." + key + " must be string");
    }
    return json_string_value(value);
}

bool read_required_bool(const json_t *object, const char *key, const std::string &context) {
    const json_t *value = json_object_get(object, key);
    if (!json_is_boolean(value)) {
        throw std::logic_error("error on config file, " + context + "." + key + " is required bool");
    }
    return json_boolean_value(value) != 0;
}

int checked_json_integer_to_int(const json_t *value, const std::string &field) {
    const json_int_t integer_value = json_integer_value(value);
    if (integer_value < std::numeric_limits<int>::min() ||
        integer_value > std::numeric_limits<int>::max()) {
        throw std::logic_error("error on config file, " + field + " is outside int range");
    }
    return static_cast<int>(integer_value);
}

int read_required_int(const json_t *object, const char *key, const std::string &context) {
    const json_t *value = json_object_get(object, key);
    if (!json_is_integer(value)) {
        throw std::logic_error("error on config file, " + context + "." + key + " is required integer");
    }
    return checked_json_integer_to_int(value, context + "." + key);
}

int read_optional_int(const json_t *object, const char *key, const int default_value, const std::string &context) {
    const json_t *value = json_object_get(object, key);
    if (!value) {
        return default_value;
    }
    if (!json_is_integer(value)) {
        throw std::logic_error("error on config file, " + context + "." + key + " must be integer");
    }
    return checked_json_integer_to_int(value, context + "." + key);
}

double read_required_number(const json_t *object, const char *key, const std::string &context) {
    const json_t *value = json_object_get(object, key);
    if (!json_is_number(value)) {
        throw std::logic_error("error on config file, " + context + "." + key + " is required number");
    }
    return json_number_value(value);
}


dp1v2::ParameterValue parse_parameter_value(const json_t *json_value, const std::string &context);

dp1v2::ParameterMap parse_parameter_map(const json_t *json_object, const std::string &context) {
    dp1v2::ParameterMap parameters;
    const char *key = nullptr;
    json_t *value = nullptr;
    json_object_foreach(const_cast<json_t *>(json_object), key, value) {
        parameters.emplace(key, parse_parameter_value(value, context + "." + key));
    }
    return parameters;
}

dp1v2::ParameterValue parse_parameter_value(const json_t *json_value, const std::string &context) {
    dp1v2::ParameterValue result{};
    if (json_is_null(json_value)) {
        result.value = nullptr;
        return result;
    }
    if (json_is_boolean(json_value)) {
        result.value = json_boolean_value(json_value) != 0;
        return result;
    }
    if (json_is_integer(json_value)) {
        result.value = checked_json_integer_to_int(json_value, context);
        return result;
    }
    if (json_is_real(json_value)) {
        result.value = json_real_value(json_value);
        return result;
    }
    if (json_is_string(json_value)) {
        result.value = std::string(json_string_value(json_value));
        return result;
    }
    if (json_is_array(json_value)) {
        auto array = std::make_shared<dp1v2::ParameterArray>();
        const auto size = json_array_size(json_value);
        array->reserve(size);
        for (std::size_t i = 0; i < size; ++i) {
            array->push_back(parse_parameter_value(json_array_get(json_value, i), context + "[]"));
        }
        result.value = array;
        return result;
    }
    if (json_is_object(json_value)) {
        result.value = std::make_shared<dp1v2::ParameterMap>(parse_parameter_map(json_value, context));
        return result;
    }
    throw std::logic_error("error on config file, unsupported parameter value at " + context);
}

const dp1v2::ParameterMap *require_parameter_object(const dp1v2::ParameterMap &parameters,
                                                   const std::string &key,
                                                   const std::string &context) {
    const auto it = parameters.find(key);
    if (it == parameters.end()) {
        throw std::logic_error("error on config file, " + context + "." + key + " is required object");
    }
    const auto *object = std::get_if<dp1v2::ParameterValue::MapPtr>(&it->second.value);
    if (object == nullptr || !(*object)) {
        throw std::logic_error("error on config file, " + context + "." + key + " is required object");
    }
    return object->get();
}

bool read_optional_parameter_bool(const dp1v2::ParameterMap &parameters,
                                  const std::string &key,
                                  const bool default_value,
                                  const std::string &context) {
    const auto it = parameters.find(key);
    if (it == parameters.end()) {
        return default_value;
    }
    const auto *value = std::get_if<bool>(&it->second.value);
    if (value == nullptr) {
        throw std::logic_error("error on config file, " + context + "." + key + " must be bool");
    }
    return *value;
}

int read_optional_parameter_int(const dp1v2::ParameterMap &parameters,
                                const std::string &key,
                                const int default_value,
                                const std::string &context) {
    const auto it = parameters.find(key);
    if (it == parameters.end()) {
        return default_value;
    }
    const auto *value = std::get_if<int>(&it->second.value);
    if (value == nullptr) {
        throw std::logic_error("error on config file, " + context + "." + key + " must be integer");
    }
    return *value;
}

std::string read_optional_parameter_string(const dp1v2::ParameterMap &parameters,
                                           const std::string &key,
                                           const std::string &default_value,
                                           const std::string &context) {
    const auto it = parameters.find(key);
    if (it == parameters.end()) {
        return default_value;
    }
    const auto *value = std::get_if<std::string>(&it->second.value);
    if (value == nullptr) {
        throw std::logic_error("error on config file, " + context + "." + key + " must be string");
    }
    return *value;
}

void reject_unknown_parameter_keys(const dp1v2::ParameterMap &parameters,
                                   const std::vector<std::string_view> &allowed,
                                   const std::string &context) {
    for (const auto &[key, value] : parameters) {
        (void)value;
        if (!contains(allowed, key)) {
            throw std::logic_error("error on config file, " + context + " has unknown key: " + key);
        }
    }
}

std::vector<std::string> read_required_string_array(const json_t *object,
                                                    const char *key,
                                                    const std::string &context) {
    const json_t *value = json_object_get(object, key);
    if (!json_is_array(value)) {
        throw std::logic_error("error on config file, " + context + "." + key + " is required array");
    }

    std::vector<std::string> result;
    const auto size = json_array_size(value);
    result.reserve(size);
    for (std::size_t i = 0; i < size; ++i) {
        const json_t *item = json_array_get(value, i);
        if (!json_is_string(item)) {
            throw std::logic_error("error on config file, " + context + "." + key + " must contain strings");
        }
        result.emplace_back(json_string_value(item));
    }
    return result;
}

void validate_schema_version(const std::string &version, const std::string &context) {
    if (version != kSupportedSchemaVersion) {
        throw std::logic_error("error on config file, unsupported " + context + ".schema_version: " + version);
    }
}

void ensure_non_empty(const std::string &value, const std::string &field) {
    if (value.empty()) {
        throw std::logic_error("error on config file, " + field + " must be non-empty");
    }
}

void ensure_no_duplicates(const std::vector<std::string> &values, const std::string &field) {
    std::set<std::string> seen;
    for (const auto &value : values) {
        if (!seen.insert(value).second) {
            throw std::logic_error("error on config file, " + field + " contains duplicate value: " + value);
        }
    }
}

dp1v2::FrameSourceMode parse_frame_source_mode(const std::string &value) {
    if (value == "file") {
        return dp1v2::FrameSourceMode::File;
    }
    if (value == "camerapro_sim" || value == "camerapro") {
        throw std::logic_error("error on config file, source.mode is reserved for future implementation: " + value);
    }
    throw std::logic_error("error on config file, unsupported source.mode: " + value);
}

dp1v2::DP2ConnectionMode parse_dp2_mode(const std::string &value) {
    if (value == "disabled") {
        return dp1v2::DP2ConnectionMode::Disabled;
    }
    if (value == "local") {
        return dp1v2::DP2ConnectionMode::Local;
    }
    if (value == "network") {
        return dp1v2::DP2ConnectionMode::Network;
    }
    throw std::logic_error("error on config file, unsupported dp2.mode: " + value);
}

dp1v2::PixelFormat parse_pixel_format(const std::string &value, const std::string &field) {
    if (value == "U8") {
        return dp1v2::PixelFormat::U8;
    }
    if (value == "U16") {
        return dp1v2::PixelFormat::U16;
    }
    if (value == "F32") {
        return dp1v2::PixelFormat::F32;
    }
    if (value == "MaskU8") {
        return dp1v2::PixelFormat::MaskU8;
    }
    if (value == "S16") {
        return dp1v2::PixelFormat::S16;
    }
    if (value == "S32") {
        return dp1v2::PixelFormat::S32;
    }
    throw std::logic_error("error on config file, unsupported " + field + ": " + value);
}

dp1v2::InputBitDepth parse_input_bit_depth(const json_t *object, const char *key, const std::string &context) {
    const json_t *value = json_object_get(object, key);
    if (!json_is_integer(value)) {
        throw std::logic_error("error on config file, " + context + "." + key + " is required integer bit depth");
    }
    const int bit_depth = checked_json_integer_to_int(value, context + "." + key);

    switch (bit_depth) {
        case 8:
            return dp1v2::InputBitDepth::Bit8;
        case 10:
            return dp1v2::InputBitDepth::Bit10;
        case 12:
            return dp1v2::InputBitDepth::Bit12;
        case 14:
            return dp1v2::InputBitDepth::Bit14;
        case 16:
            return dp1v2::InputBitDepth::Bit16;
        default:
            throw std::logic_error("error on config file, unsupported " + context + "." + key + ": " + std::to_string(bit_depth));
    }
}

dp1v2::InverseMedianMode parse_inverse_median_mode(const std::string &value) {
    if (value == "FixedK3") {
        return dp1v2::InverseMedianMode::FixedK3;
    }
    if (value == "FixedK5") {
        return dp1v2::InverseMedianMode::FixedK5;
    }
    throw std::logic_error("error on config file, unsupported inverse_median.mode: " + value);
}

dp1v2::InverseMedianOutputMode parse_inverse_median_output_mode(const std::string &value) {
    if (value == "RawSigned") {
        return dp1v2::InverseMedianOutputMode::RawSigned;
    }
    if (value == "ClipToInputRange") {
        return dp1v2::InverseMedianOutputMode::ClipToInputRange;
    }
    throw std::logic_error("error on config file, unsupported inverse_median.output_dynamic_range_mode: " + value);
}

bool is_valid_level(const std::string &value) {
    static const std::vector<std::string_view> levels{"L0", "L1", "L2", "L3", "Lx"};
    return contains(levels, value);
}

bool is_allowed_variant(const std::string_view stage, const std::string_view variant) {
    static const std::vector<std::string_view> prep{"full_frame", "roi", "tiles", "adaptive_roi"};
    static const std::vector<std::string_view> radiometric{
        "mean_subtraction", "gaussian_subtraction", "median", "inverse_median",
        "adaptive_background", "band_pass", "per_tile_background"};
    static const std::vector<std::string_view> enhancement{"gaussian", "dog", "bilateral", "guided", "multi_scale"};
    static const std::vector<std::string_view> matched_filter{
        "gaussian", "kernel", "template", "adaptive_kernel", "psf_fit"};
    static const std::vector<std::string_view> candidate{
        "global_threshold", "adaptive_threshold", "absdiff", "mog2", "knn"};
    static const std::vector<std::string_view> segmentation{
        "single_morphology", "open_close_contours", "connected_components", "multi_step_morphology"};
    static const std::vector<std::string_view> object_filtering{"area", "geom_basic", "shape_photometry"};
    static const std::vector<std::string_view> measurement{
        "centroid_bbox", "photometry_basic", "rotated_bbox_moments_subpixel"};

    if (stage == "prep") {
        return contains(prep, variant);
    }
    if (stage == "radiometric") {
        return contains(radiometric, variant);
    }
    if (stage == "enhancement") {
        return contains(enhancement, variant);
    }
    if (stage == "matched_filter") {
        return contains(matched_filter, variant);
    }
    if (stage == "candidate_extraction") {
        return contains(candidate, variant);
    }
    if (stage == "segmentation") {
        return contains(segmentation, variant);
    }
    if (stage == "object_filtering") {
        return contains(object_filtering, variant);
    }
    if (stage == "measurement") {
        return contains(measurement, variant);
    }
    return false;
}

void validate_logging_level(const std::string &level, const std::string &field) {
    static const std::vector<std::string_view> levels{"TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"};
    if (!contains(levels, level)) {
        throw std::logic_error("error on config file, unsupported " + field + ": " + level);
    }
}

void validate_profiling_level(const std::string &level) {
    static const std::vector<std::string_view> levels{"P0", "P1", "P2", "P3", "P4", "P5"};
    if (!contains(levels, level)) {
        throw std::logic_error("error on config file, unsupported profiling level: " + level);
    }
}

dp1v2::LoggingConfig parse_logging_config(const json_t *logging_json) {
    reject_unknown_keys(logging_json,
                        {"enabled", "config_file", "default_level", "realtime_profile", "structured_messages",
                         "sanitize_external_strings", "max_field_length", "max_messages_per_frame",
                         "max_messages_per_tile", "mdc", "sampling", "async", "logger_overrides"},
                        "application.logging");

    dp1v2::LoggingConfig config{};
    config.enabled = read_required_bool(logging_json, "enabled", "application.logging");
    config.config_file = read_required_string(logging_json, "config_file", "application.logging");
    config.default_level = read_required_string(logging_json, "default_level", "application.logging");
    config.realtime_profile = read_required_string(logging_json, "realtime_profile", "application.logging");
    config.structured_messages = read_required_bool(logging_json, "structured_messages", "application.logging");
    config.sanitize_external_strings = read_required_bool(logging_json, "sanitize_external_strings", "application.logging");
    config.max_field_length = read_required_int(logging_json, "max_field_length", "application.logging");
    config.max_messages_per_frame = read_required_int(logging_json, "max_messages_per_frame", "application.logging");
    config.max_messages_per_tile = read_required_int(logging_json, "max_messages_per_tile", "application.logging");

    const json_t *mdc_json = read_required_object(logging_json, "mdc", "application.logging");
    reject_unknown_keys(mdc_json, {"enabled", "fields"}, "application.logging.mdc");
    config.mdc.enabled = read_required_bool(mdc_json, "enabled", "application.logging.mdc");
    config.mdc.fields = read_required_string_array(mdc_json, "fields", "application.logging.mdc");

    const json_t *sampling_json = read_required_object(logging_json, "sampling", "application.logging");
    reject_unknown_keys(sampling_json,
                        {"frame_summary_every_n", "rate_limit_per_event_per_sec", "duplicate_suppression"},
                        "application.logging.sampling");
    config.sampling.frame_summary_every_n = read_required_int(sampling_json, "frame_summary_every_n", "application.logging.sampling");
    config.sampling.rate_limit_per_event_per_sec = read_required_int(sampling_json, "rate_limit_per_event_per_sec", "application.logging.sampling");
    config.sampling.duplicate_suppression = read_required_bool(sampling_json, "duplicate_suppression", "application.logging.sampling");

    const json_t *async_json = read_required_object(logging_json, "async", "application.logging");
    reject_unknown_keys(async_json, {"enabled", "buffer_size", "blocking", "discard_policy"}, "application.logging.async");
    config.async.enabled = read_required_bool(async_json, "enabled", "application.logging.async");
    config.async.buffer_size = read_required_int(async_json, "buffer_size", "application.logging.async");
    config.async.blocking = read_required_bool(async_json, "blocking", "application.logging.async");
    config.async.discard_policy = read_required_string(async_json, "discard_policy", "application.logging.async");

    if (const json_t *overrides_json = json_object_get(logging_json, "logger_overrides")) {
        if (!json_is_array(overrides_json)) {
            throw std::logic_error("error on config file, application.logging.logger_overrides must be array");
        }
        const auto size = json_array_size(overrides_json);
        config.logger_overrides.reserve(size);
        for (std::size_t i = 0; i < size; ++i) {
            const json_t *override_json = json_array_get(overrides_json, i);
            if (!json_is_object(override_json)) {
                throw std::logic_error("error on config file, logger override must be object");
            }
            reject_unknown_keys(override_json, {"logger", "level"}, "application.logging.logger_overrides[]");
            config.logger_overrides.push_back(dp1v2::LoggerLevelOverride{
                .logger = read_required_string(override_json, "logger", "application.logging.logger_overrides[]"),
                .level = read_required_string(override_json, "level", "application.logging.logger_overrides[]"),
            });
        }
    }

    ensure_non_empty(config.config_file, "application.logging.config_file");
    validate_logging_level(config.default_level, "application.logging.default_level");
    if (config.realtime_profile != "rt_safe" && config.realtime_profile != "diagnostic") {
        throw std::logic_error("error on config file, unsupported application.logging.realtime_profile: " + config.realtime_profile);
    }
    if (!config.structured_messages) {
        throw std::logic_error("error on config file, application.logging.structured_messages=false is not canonical for this task");
    }
    if (!config.sanitize_external_strings) {
        throw std::logic_error("error on config file, application.logging.sanitize_external_strings=false is not canonical");
    }
    if (config.max_field_length <= 0) {
        throw std::logic_error("error on config file, application.logging.max_field_length must be > 0");
    }
    if (config.max_messages_per_frame < 0 || config.max_messages_per_tile < 0) {
        throw std::logic_error("error on config file, logging message limits must be >= 0");
    }
    if (config.max_messages_per_frame > 0 && config.max_messages_per_tile > config.max_messages_per_frame) {
        throw std::logic_error("error on config file, application.logging.max_messages_per_tile must be <= max_messages_per_frame");
    }
    ensure_no_duplicates(config.mdc.fields, "application.logging.mdc.fields");
    if (config.sampling.frame_summary_every_n < 0 || config.sampling.rate_limit_per_event_per_sec < 0) {
        throw std::logic_error("error on config file, logging sampling limits must be >= 0");
    }
    if (config.async.buffer_size <= 0) {
        throw std::logic_error("error on config file, application.logging.async.buffer_size must be > 0");
    }
    if (config.realtime_profile == "rt_safe" && config.async.blocking) {
        throw std::logic_error("error on config file, application.logging.async.blocking must be false for rt_safe");
    }
    static const std::vector<std::string_view> discard_policies{
        "drop_debug_and_summarize", "block", "drop_new_debug"};
    if (!contains(discard_policies, config.async.discard_policy)) {
        throw std::logic_error("error on config file, unsupported application.logging.async.discard_policy: " + config.async.discard_policy);
    }

    std::vector<std::string> override_loggers;
    override_loggers.reserve(config.logger_overrides.size());
    for (const auto &override : config.logger_overrides) {
        ensure_non_empty(override.logger, "application.logging.logger_overrides.logger");
        validate_logging_level(override.level, "application.logging.logger_overrides.level");
        override_loggers.push_back(override.logger);
    }
    ensure_no_duplicates(override_loggers, "application.logging.logger_overrides.logger");

    return config;
}

dp1v2::ProfilingConfig parse_profiling_config(const json_t *profiling_json) {
    reject_unknown_keys(profiling_json,
                        {"enabled", "mode", "levels", "aggregation_window_frames", "raw_trace",
                         "operation_timing", "reports", "logging_bridge", "external_trace"},
                        "application.profiling");

    dp1v2::ProfilingConfig config{};
    config.enabled = read_required_bool(profiling_json, "enabled", "application.profiling");
    config.mode = read_required_string(profiling_json, "mode", "application.profiling");
    config.levels = read_required_string_array(profiling_json, "levels", "application.profiling");
    config.aggregation_window_frames = read_required_int(profiling_json, "aggregation_window_frames", "application.profiling");

    const json_t *raw_trace_json = read_required_object(profiling_json, "raw_trace", "application.profiling");
    reject_unknown_keys(raw_trace_json, {"enabled", "max_frames", "max_events_per_frame"}, "application.profiling.raw_trace");
    config.raw_trace.enabled = read_required_bool(raw_trace_json, "enabled", "application.profiling.raw_trace");
    config.raw_trace.max_frames = read_required_int(raw_trace_json, "max_frames", "application.profiling.raw_trace");
    config.raw_trace.max_events_per_frame = read_required_int(raw_trace_json, "max_events_per_frame", "application.profiling.raw_trace");

    const json_t *timing_json = read_required_object(profiling_json, "operation_timing", "application.profiling");
    reject_unknown_keys(timing_json,
                        {"enabled", "include_format_conversions", "include_memory_copies", "include_allocations"},
                        "application.profiling.operation_timing");
    config.operation_timing.enabled = read_required_bool(timing_json, "enabled", "application.profiling.operation_timing");
    config.operation_timing.include_format_conversions = read_required_bool(timing_json, "include_format_conversions", "application.profiling.operation_timing");
    config.operation_timing.include_memory_copies = read_required_bool(timing_json, "include_memory_copies", "application.profiling.operation_timing");
    config.operation_timing.include_allocations = read_required_bool(timing_json, "include_allocations", "application.profiling.operation_timing");

    const json_t *reports_json = read_required_object(profiling_json, "reports", "application.profiling");
    reject_unknown_keys(reports_json,
                        {"emit_frame_reports", "emit_window_summary", "emit_run_summary", "format", "output_dir"},
                        "application.profiling.reports");
    config.reports.emit_frame_reports = read_required_bool(reports_json, "emit_frame_reports", "application.profiling.reports");
    config.reports.emit_window_summary = read_required_bool(reports_json, "emit_window_summary", "application.profiling.reports");
    config.reports.emit_run_summary = read_required_bool(reports_json, "emit_run_summary", "application.profiling.reports");
    config.reports.format = read_required_string(reports_json, "format", "application.profiling.reports");
    config.reports.output_dir = read_optional_string(reports_json, "output_dir", "", "application.profiling.reports");

    const json_t *bridge_json = read_required_object(profiling_json, "logging_bridge", "application.profiling");
    reject_unknown_keys(bridge_json,
                        {"emit_aggregated_summaries", "summary_every_n_frames", "emit_budget_warnings"},
                        "application.profiling.logging_bridge");
    config.logging_bridge.emit_aggregated_summaries = read_required_bool(bridge_json, "emit_aggregated_summaries", "application.profiling.logging_bridge");
    config.logging_bridge.summary_every_n_frames = read_required_int(bridge_json, "summary_every_n_frames", "application.profiling.logging_bridge");
    config.logging_bridge.emit_budget_warnings = read_required_bool(bridge_json, "emit_budget_warnings", "application.profiling.logging_bridge");

    const json_t *external_json = read_required_object(profiling_json, "external_trace", "application.profiling");
    reject_unknown_keys(external_json, {"enabled", "backend"}, "application.profiling.external_trace");
    config.external_trace.enabled = read_required_bool(external_json, "enabled", "application.profiling.external_trace");
    config.external_trace.backend = read_required_string(external_json, "backend", "application.profiling.external_trace");

    static const std::vector<std::string_view> modes{"disabled", "lightweight", "detailed", "external_trace"};
    if (!contains(modes, config.mode)) {
        throw std::logic_error("error on config file, unsupported application.profiling.mode: " + config.mode);
    }
    if (config.mode == "external_trace" || config.external_trace.enabled || config.external_trace.backend != "none") {
        throw std::logic_error("error on config file, external profiling trace is outside the approved config task");
    }
    ensure_no_duplicates(config.levels, "application.profiling.levels");
    for (const auto &level : config.levels) {
        validate_profiling_level(level);
    }
    if (config.aggregation_window_frames <= 0) {
        throw std::logic_error("error on config file, application.profiling.aggregation_window_frames must be > 0");
    }
    if (config.raw_trace.max_frames < 0 || config.raw_trace.max_events_per_frame <= 0) {
        throw std::logic_error("error on config file, invalid application.profiling.raw_trace bounds");
    }
    if (config.raw_trace.enabled && config.raw_trace.max_frames <= 0) {
        throw std::logic_error("error on config file, raw_trace.enabled requires max_frames > 0");
    }
    if (!config.operation_timing.include_format_conversions || !config.operation_timing.include_memory_copies) {
        throw std::logic_error("error on config file, operation timing must include conversions and memory copies for canonical DP1");
    }
    static const std::vector<std::string_view> report_formats{"json", "csv", "none"};
    if (!contains(report_formats, config.reports.format)) {
        throw std::logic_error("error on config file, unsupported application.profiling.reports.format: " + config.reports.format);
    }
    if (config.logging_bridge.summary_every_n_frames < 0) {
        throw std::logic_error("error on config file, profiling logging bridge interval must be >= 0");
    }

    return config;
}

dp1v2::SourceConfig parse_source_config(const json_t *source_json) {
    reject_unknown_keys(source_json, {"mode", "file"}, "application.source");

    dp1v2::SourceConfig config{};
    config.mode = parse_frame_source_mode(read_required_string(source_json, "mode", "application.source"));
    const json_t *file_json = read_required_object(source_json, "file", "application.source");
    reject_unknown_keys(file_json, {"path", "recursive", "repeat"}, "application.source.file");
    config.file.path = resolve_resource_path(read_required_string(file_json, "path", "application.source.file"));
    config.file.recursive = read_required_bool(file_json, "recursive", "application.source.file");
    config.file.repeat = read_required_bool(file_json, "repeat", "application.source.file");
    ensure_non_empty(config.file.path, "application.source.file.path");
    return config;
}

dp1v2::DP2ConnectionConfig parse_dp2_config(const json_t *dp2_json) {
    reject_unknown_keys(dp2_json, {"enabled", "mode", "host", "port", "reconnect_interval_s"}, "application.dp2");

    dp1v2::DP2ConnectionConfig config{};
    config.enabled = read_required_bool(dp2_json, "enabled", "application.dp2");
    config.mode = parse_dp2_mode(read_required_string(dp2_json, "mode", "application.dp2"));
    config.host = read_optional_string(dp2_json, "host", dp1v2::kDefaultDp2Host, "application.dp2");
    const int port = read_optional_int(dp2_json, "port", dp1v2::kDefaultDp2Port, "application.dp2");
    if (port < dp1v2::kMinTcpPort || port > dp1v2::kMaxTcpPort) {
        throw std::logic_error("error on config file, application.dp2.port must be in range [1..65535]");
    }
    config.port = static_cast<std::uint16_t>(port);
    config.reconnect_interval_s = read_optional_int(dp2_json, "reconnect_interval_s", dp1v2::kDefaultDp2ReconnectIntervalS, "application.dp2");

    if (config.enabled && config.mode == dp1v2::DP2ConnectionMode::Disabled) {
        throw std::logic_error("error on config file, application.dp2.enabled=true requires local or network mode");
    }
    if (!config.enabled && config.mode != dp1v2::DP2ConnectionMode::Disabled) {
        throw std::logic_error("error on config file, application.dp2.enabled=false requires disabled mode");
    }
    if (config.enabled) {
        ensure_non_empty(config.host, "application.dp2.host");
    }
    if (config.reconnect_interval_s < dp1v2::kMinDp2ReconnectIntervalS) {
        throw std::logic_error("error on config file, application.dp2.reconnect_interval_s must be >= 1");
    }
    return config;
}

dp1v2::ApplicationConfig parse_application_config(const json_t *application_json) {
    reject_unknown_keys(application_json, {"schema_version", "source", "logging", "profiling", "dp2"}, "application");

    dp1v2::ApplicationConfig config{};
    config.schema_version = read_required_string(application_json, "schema_version", "application");
    validate_schema_version(config.schema_version, "application");
    config.source = parse_source_config(read_required_object(application_json, "source", "application"));
    config.logging = parse_logging_config(read_required_object(application_json, "logging", "application"));
    config.profiling = parse_profiling_config(read_required_object(application_json, "profiling", "application"));
    config.dp2 = parse_dp2_config(read_required_object(application_json, "dp2", "application"));
    return config;
}

dp1v2::PixelRange parse_pixel_range(const json_t *range_json) {
    reject_unknown_keys(range_json, {"min_value", "max_value", "black_level", "saturation_level"}, "pipeline.input_route.pixel_range");

    dp1v2::PixelRange range{};
    range.min_value = read_required_number(range_json, "min_value", "pipeline.input_route.pixel_range");
    range.max_value = read_required_number(range_json, "max_value", "pipeline.input_route.pixel_range");
    range.black_level = read_required_number(range_json, "black_level", "pipeline.input_route.pixel_range");
    range.saturation_level = read_required_number(range_json, "saturation_level", "pipeline.input_route.pixel_range");
    if (!(range.min_value <= range.black_level && range.black_level <= range.saturation_level &&
          range.saturation_level <= range.max_value && range.max_value > range.min_value)) {
        throw std::logic_error("error on config file, pipeline.input_route.pixel_range must be ordered and positive");
    }
    return range;
}

dp1v2::InputRouteConfig parse_input_route(const json_t *route_json) {
    reject_unknown_keys(route_json, {"pixel_format", "bit_depth", "pixel_range"}, "pipeline.input_route");

    dp1v2::InputRouteConfig config{};
    config.pixel_format = parse_pixel_format(read_required_string(route_json, "pixel_format", "pipeline.input_route"),
                                             "pipeline.input_route.pixel_format");
    if (config.pixel_format != dp1v2::PixelFormat::U8 && config.pixel_format != dp1v2::PixelFormat::U16) {
        throw std::logic_error("error on config file, pipeline.input_route.pixel_format must be U8 or U16");
    }
    config.bit_depth = parse_input_bit_depth(route_json, "bit_depth", "pipeline.input_route");
    if (config.pixel_format == dp1v2::PixelFormat::U8 && config.bit_depth != dp1v2::InputBitDepth::Bit8) {
        throw std::logic_error("error on config file, U8 input_route requires bit_depth 8");
    }
    if (config.pixel_format == dp1v2::PixelFormat::U16 && config.bit_depth == dp1v2::InputBitDepth::Bit8) {
        throw std::logic_error("error on config file, U16 input_route requires bit_depth 10, 12, 14, or 16");
    }
    config.pixel_range = parse_pixel_range(read_required_object(route_json, "pixel_range", "pipeline.input_route"));
    return config;
}

dp1v2::InverseMedianParametersConfig resolve_inverse_median_parameters(const dp1v2::ParameterMap &parameters) {
    const auto *inverse_parameters = require_parameter_object(
        parameters, "inverse_median", "pipeline.pipeline.radiometric.parameters");
    reject_unknown_parameter_keys(*inverse_parameters,
                                  {"enabled", "mode", "stride", "output_median_frame", "output_dynamic_range_mode"},
                                  "pipeline.pipeline.radiometric.parameters.inverse_median");

    dp1v2::InverseMedianParametersConfig config{};
    config.enabled = read_optional_parameter_bool(
        *inverse_parameters, "enabled", true, "pipeline.pipeline.radiometric.parameters.inverse_median");
    config.mode = parse_inverse_median_mode(read_optional_parameter_string(
        *inverse_parameters, "mode", "FixedK3", "pipeline.pipeline.radiometric.parameters.inverse_median"));
    config.stride = read_optional_parameter_int(
        *inverse_parameters, "stride", 1, "pipeline.pipeline.radiometric.parameters.inverse_median");
    config.output_median_frame = read_optional_parameter_bool(
        *inverse_parameters, "output_median_frame", false, "pipeline.pipeline.radiometric.parameters.inverse_median");
    config.output_dynamic_range_mode = parse_inverse_median_output_mode(read_optional_parameter_string(
        *inverse_parameters, "output_dynamic_range_mode", "RawSigned", "pipeline.pipeline.radiometric.parameters.inverse_median"));
    if (config.stride < 1) {
        throw std::logic_error("error on config file, inverse_median.stride must be >= 1");
    }
    return config;
}

dp1v2::StageConfig parse_stage_config(const json_t *stages_json, const char *stage_name) {
    const std::string context = std::string("pipeline.pipeline.") + stage_name;
    const json_t *stage_json = read_required_object(stages_json, stage_name, "pipeline.pipeline");
    reject_unknown_keys(stage_json, {"enabled", "variant", "level", "parameters"}, context);

    dp1v2::StageConfig config{};
    config.enabled = read_required_bool(stage_json, "enabled", context);
    config.variant = read_required_string(stage_json, "variant", context);
    config.level = read_required_string(stage_json, "level", context);
    const json_t *parameters_json = read_required_object(stage_json, "parameters", context);
    config.parameters = parse_parameter_map(parameters_json, context + ".parameters");

    if (!is_allowed_variant(stage_name, config.variant)) {
        throw std::logic_error("error on config file, unsupported " + context + ".variant: " + config.variant);
    }
    if (!is_valid_level(config.level)) {
        throw std::logic_error("error on config file, unsupported " + context + ".level: " + config.level);
    }
    return config;
}

dp1v2::PipelineStagesConfig parse_pipeline_stages(const json_t *stages_json) {
    reject_unknown_keys(stages_json,
                        {"prep", "radiometric", "enhancement", "matched_filter", "candidate_extraction",
                         "segmentation", "object_filtering", "measurement"},
                        "pipeline.pipeline");

    dp1v2::PipelineStagesConfig stages{};
    stages.prep = parse_stage_config(stages_json, "prep");
    stages.radiometric = parse_stage_config(stages_json, "radiometric");
    stages.enhancement = parse_stage_config(stages_json, "enhancement");
    stages.matched_filter = parse_stage_config(stages_json, "matched_filter");
    stages.candidate_extraction = parse_stage_config(stages_json, "candidate_extraction");
    stages.segmentation = parse_stage_config(stages_json, "segmentation");
    stages.object_filtering = parse_stage_config(stages_json, "object_filtering");
    stages.measurement = parse_stage_config(stages_json, "measurement");
    return stages;
}

dp1v2::PipelineConfig parse_pipeline_config(const json_t *pipeline_json) {
    reject_unknown_keys(pipeline_json, {"schema_version", "profile", "input_route", "pipeline"}, "pipeline");

    dp1v2::PipelineConfig config{};
    config.schema_version = read_required_string(pipeline_json, "schema_version", "pipeline");
    validate_schema_version(config.schema_version, "pipeline");
    config.profile = read_required_string(pipeline_json, "profile", "pipeline");
    static const std::vector<std::string_view> profiles{"RT-5", "RT-20", "custom"};
    if (!contains(profiles, config.profile)) {
        throw std::logic_error("error on config file, unsupported pipeline.profile: " + config.profile);
    }
    config.input_route = parse_input_route(read_required_object(pipeline_json, "input_route", "pipeline"));
    config.stages = parse_pipeline_stages(read_required_object(pipeline_json, "pipeline", "pipeline"));
    return config;
}


dp1v2::RadiometricResolvedConfig resolve_radiometric_config(const dp1v2::StageConfig &stage) {
    dp1v2::RadiometricResolvedConfig resolved{};
    if (stage.variant == "inverse_median") {
        resolved.inverse_median = resolve_inverse_median_parameters(stage.parameters);
    }
    return resolved;
}

dp1v2::ResolvedPipelineConfig resolve_pipeline_config(const dp1v2::PipelineConfig &pipeline) {
    dp1v2::ResolvedPipelineConfig resolved{};
    resolved.radiometric = resolve_radiometric_config(pipeline.stages.radiometric);
    return resolved;
}

} // namespace

namespace dp1v2 {

ApplicationConfig load_application_config(const std::string &config_path) {
    auto root = json_unique_ptr_create(json_load_file(config_path.c_str(), 0, nullptr));
    if (!root) {
        throw std::logic_error("error on opening application config file: " + config_path);
    }
    if (!json_is_object(root.get())) {
        throw std::logic_error("error on application config file, root must be object");
    }
    if (json_object_get(root.get(), "config") || json_object_get(root.get(), "application") ||
        json_object_get(root.get(), "pipeline") || json_object_get(root.get(), "dp2conn")) {
        throw std::logic_error("error on application config file, root must be ApplicationConfig, not a wrapper or legacy shape");
    }
    return parse_application_config(root.get());
}

PipelineConfig load_pipeline_config(const std::string &config_path) {
    auto root = json_unique_ptr_create(json_load_file(config_path.c_str(), 0, nullptr));
    if (!root) {
        throw std::logic_error("error on opening pipeline config file: " + config_path);
    }
    if (!json_is_object(root.get())) {
        throw std::logic_error("error on pipeline config file, root must be object");
    }
    if (json_object_get(root.get(), "config") || json_object_get(root.get(), "application") ||
        json_object_get(root.get(), "dp2conn")) {
        throw std::logic_error("error on pipeline config file, root must be PipelineConfig, not a wrapper or legacy shape");
    }
    return parse_pipeline_config(root.get());
}

Dp1Config load_dp1_config(const std::string &application_config_path, const std::string &pipeline_config_path) {
    Dp1Config config{};
    config.application = load_application_config(application_config_path);
    config.pipeline = load_pipeline_config(pipeline_config_path);
    config.resolved_pipeline = resolve_pipeline_config(config.pipeline);
    return config;
}

} // namespace dp1v2
