#pragma once

#include <cstdint>
#include <string>

#include "dp1v2/config/defaults.hpp"

namespace dp1v2 {

enum class SourceKind {
    CamproIpc,
    UriFile,
};

struct SourceConfig {
    SourceKind kind = SourceKind::UriFile;
    std::string source;
    std::string link;
    int ipc_receive_timeout_ms = kDefaultIpcReceiveTimeoutMs;
};

struct Dp2ConnConfig {
    std::string host = kDefaultDp2Host;
    std::uint16_t port = kDefaultDp2Port;
    int reconnect_interval_s = kDefaultDp2ReconnectIntervalS;
};

struct ArtifactConfig {
    bool enabled = false;
    bool json_enabled = false;
    bool binocular_enabled = false;
    std::string out_folder = "datapro1_v2_output";
    std::string data_bin_folder = "data_bin";
};

struct RuntimeLoopConfig {
    std::uint64_t max_frames = kDefaultRuntimeMaxFrames;
    std::uint64_t max_empty_reads = kDefaultRuntimeMaxEmptyReads;
};

struct CalibrationConfig {
    bool required = false;
    std::string camera_settings_file;
};

struct RuntimeConfig {
    SourceConfig source;
    Dp2ConnConfig dp2_conn;
    ArtifactConfig artifacts;
    RuntimeLoopConfig runtime_loop;
    CalibrationConfig calibration;
};

const char *source_kind_to_cstr(SourceKind kind);
RuntimeConfig load_runtime_config(const std::string &config_path);

} // namespace dp1v2
