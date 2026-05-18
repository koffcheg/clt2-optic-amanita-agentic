#pragma once

#include <cstddef>
#include <cstdint>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include "dp1v2/config/defaults.hpp"
#include "dp1v2/domain/pixel.hpp"

namespace dp1v2 {

enum class FrameSourceMode {
    File,
    CameraProSim,
    CameraPro,
};

struct FileSourceConfig {
    std::string path;
    bool recursive = false;
    bool repeat = false;
};

struct SourceConfig {
    FrameSourceMode mode = FrameSourceMode::File;
    FileSourceConfig file;
};

enum class DP2ConnectionMode {
    Disabled,
    Local,
    Network,
};

struct DP2ConnectionConfig {
    bool enabled = false;
    DP2ConnectionMode mode = DP2ConnectionMode::Disabled;
    std::string host = kDefaultDp2Host;
    std::uint16_t port = kDefaultDp2Port;
    int reconnect_interval_s = kDefaultDp2ReconnectIntervalS;
};

struct LoggingMdcConfig {
    bool enabled = true;
    std::vector<std::string> fields;
};

struct LoggingSamplingConfig {
    int frame_summary_every_n = 100;
    int rate_limit_per_event_per_sec = 1;
    bool duplicate_suppression = true;
};

struct LoggingAsyncConfig {
    bool enabled = true;
    int buffer_size = 1024;
    bool blocking = false;
    std::string discard_policy = "drop_debug_and_summarize";
};

struct LoggerLevelOverride {
    std::string logger;
    std::string level;
};

struct LoggingConfig {
    bool enabled = true;
    std::string config_file;
    std::string default_level = "INFO";
    std::string realtime_profile = "rt_safe";
    bool structured_messages = true;
    bool sanitize_external_strings = true;
    int max_field_length = 256;
    int max_messages_per_frame = 64;
    int max_messages_per_tile = 16;
    LoggingMdcConfig mdc;
    LoggingSamplingConfig sampling;
    LoggingAsyncConfig async;
    std::vector<LoggerLevelOverride> logger_overrides;
};

struct ProfilingRawTraceConfig {
    bool enabled = false;
    int max_frames = 0;
    int max_events_per_frame = 64;
};

struct ProfilingOperationTimingConfig {
    bool enabled = false;
    bool include_format_conversions = true;
    bool include_memory_copies = true;
    bool include_allocations = false;
};

struct ProfilingReportsConfig {
    bool emit_frame_reports = false;
    bool emit_window_summary = true;
    bool emit_run_summary = true;
    std::string format = "json";
    std::string output_dir;
};

struct ProfilingLoggingBridgeConfig {
    bool emit_aggregated_summaries = true;
    int summary_every_n_frames = 300;
    bool emit_budget_warnings = true;
};

struct ProfilingExternalTraceConfig {
    bool enabled = false;
    std::string backend = "none";
};

struct ProfilingConfig {
    bool enabled = true;
    std::string mode = "lightweight";
    std::vector<std::string> levels;
    int aggregation_window_frames = 300;
    ProfilingRawTraceConfig raw_trace;
    ProfilingOperationTimingConfig operation_timing;
    ProfilingReportsConfig reports;
    ProfilingLoggingBridgeConfig logging_bridge;
    ProfilingExternalTraceConfig external_trace;
};

struct VisualizationConfig {
    bool enabled = false;
    std::string output_dir = "datapro1_v2_output/visualization";
    std::string mode = "sync_file";
    int every_n_frames = 1;
    int max_frames = 0;
    std::vector<std::string> stages;
};

struct ApplicationConfig {
    std::string schema_version;
    SourceConfig source;
    LoggingConfig logging;
    ProfilingConfig profiling;
    VisualizationConfig visualization;
    DP2ConnectionConfig dp2;
};

struct InputRouteConfig {
    PixelFormat pixel_format = PixelFormat::U16;
    InputBitDepth bit_depth = InputBitDepth::Bit16;
    PixelRange pixel_range;
};

struct ParameterValue;
using ParameterArray = std::vector<ParameterValue>;
using ParameterMap = std::map<std::string, ParameterValue>;

struct ParameterValue {
    using ArrayPtr = std::shared_ptr<ParameterArray>;
    using MapPtr = std::shared_ptr<ParameterMap>;

    std::variant<std::nullptr_t, bool, int, double, std::string, ArrayPtr, MapPtr> value;
};

enum class InverseMedianMode {
    FixedK3,
    FixedK5,
};

enum class InverseMedianOutputMode {
    RawSigned,
    ClipToInputRange,
};

struct InverseMedianParametersConfig {
    bool enabled = true;
    InverseMedianMode mode = InverseMedianMode::FixedK3;
    int stride = 1;
    bool output_median_frame = false;
    InverseMedianOutputMode output_dynamic_range_mode = InverseMedianOutputMode::RawSigned;
};

struct StageConfig {
    bool enabled = false;
    std::string variant;
    std::string level;
    ParameterMap parameters;
};

struct RadiometricResolvedConfig {
    std::optional<InverseMedianParametersConfig> inverse_median;
};

struct ResolvedPipelineConfig {
    RadiometricResolvedConfig radiometric;
};

struct PipelineStagesConfig {
    StageConfig acquisition;
    StageConfig prep;
    StageConfig radiometric;
    StageConfig enhancement;
    StageConfig matched_filter;
    StageConfig candidate_extraction;
    StageConfig segmentation;
    StageConfig object_filtering;
    StageConfig measurement;
};

struct PipelineConfig {
    std::string schema_version;
    std::string profile;
    InputRouteConfig input_route;
    PipelineStagesConfig stages;
};

struct Dp1Config {
    ApplicationConfig application;
    PipelineConfig pipeline;
    ResolvedPipelineConfig resolved_pipeline;
};

ApplicationConfig load_application_config(const std::string &config_path);
PipelineConfig load_pipeline_config(const std::string &config_path);
Dp1Config load_dp1_config(const std::string &application_config_path, const std::string &pipeline_config_path);

} // namespace dp1v2
