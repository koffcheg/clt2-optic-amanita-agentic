#pragma once

#include <cstdint>

namespace dp1v2 {

constexpr int kDefaultIpcReceiveTimeoutMs = 500;
constexpr int kMinIpcReceiveTimeoutMs = 1;

constexpr const char *kDefaultDp2Host = "127.0.0.1";
constexpr std::uint16_t kDefaultDp2Port = 11511;
constexpr int kDefaultDp2ReconnectIntervalS = 3;
constexpr int kMinDp2ReconnectIntervalS = 1;

constexpr std::uint64_t kDefaultRuntimeMaxFrames = 1;
constexpr std::uint64_t kDefaultRuntimeMaxEmptyReads = 1;
constexpr std::uint64_t kMinRuntimeMaxEmptyReads = 1;

constexpr int kMinTcpPort = 1;
constexpr int kMaxTcpPort = 65535;

} // namespace dp1v2
