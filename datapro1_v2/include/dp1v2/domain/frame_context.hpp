#pragma once

#include <cstdint>
#include <chrono>
#include <string>
#include <vector>

#include "dp1v2/domain/geometry.hpp"
#include "dp1v2/domain/pixel.hpp"
#include "dp1v2/domain/time.hpp"
#include "dp1v2/domain/status.hpp"

namespace dp1v2 {

struct PipelineConfig;

enum class FrameArtifactKind : std::uint8_t {
    RawFrame,
    ProcessingFrame,
    BinaryMask,
    CandidateSet,
    SegmentSet,
    ValidatedObjectSet,
    MeasurementSet,
    Visualization,
    Diagnostics,
    Profiling,
};

enum class FrameArtifactDomain : std::uint8_t {
    Raw,
    Processing,
    Mask,
    Struct,
    Measurement,
    Visualization,
    Runtime,
};

enum class FrameArtifactOwnership : std::uint8_t {
    OwnedByFramePacket,
    OwnedByStageOutput,
    BorrowedReadOnly,
    ExternalTransport,
    MetadataOnly,
    ExpiredReference,
};

enum class FrameArtifactLifetime : std::uint8_t {
    InputBoundary,
    StageOutputScope,
    FrameBoundary,
    Persisted,
    MetadataOnly,
};

enum class FrameArtifactStatus : std::uint8_t {
    Available,
    MetadataOnly,
    Expired,
    Failed,
};

// Registry records describe provenance and payload lifetime. They do not extend
// cv::Mat or stage-output object lifetime unless a future record explicitly owns it.
struct FrameArtifactRef {
    std::string id;
    FrameArtifactKind kind = FrameArtifactKind::RawFrame;
    FrameArtifactDomain domain = FrameArtifactDomain::Raw;
    FrameArtifactOwnership ownership = FrameArtifactOwnership::MetadataOnly;
    FrameArtifactLifetime lifetime = FrameArtifactLifetime::MetadataOnly;
    FrameArtifactStatus status = FrameArtifactStatus::MetadataOnly;
    std::string semantic_name;
    std::string producer_stage;
    std::string parent_artifact_id;
    PixelFormat pixel_format = PixelFormat::U16;
    InputBitDepth bit_depth = InputBitDepth::Bit16;
    FrameGeometry geometry;
};

struct FrameArtifactRegistry {
    std::vector<FrameArtifactRef> records;
};

struct StageTiming {
    std::string stage_key;
    StageStatusCode status = StageStatusCode::NotStarted;
    std::string variant;
    std::string level;
    PixelFormat input_format = PixelFormat::U16;
    PixelFormat output_format = PixelFormat::U16;
    std::chrono::steady_clock::time_point start_time{};
    std::chrono::steady_clock::time_point end_time{};
    std::int64_t duration_ns = 0;
    std::string reason;
};

struct FrameProfiling {
    std::vector<StageTiming> stage_timings;
};

struct FrameContext {
    std::uint64_t frame_id = 0;
    int camera_id = -1;
    std::string source_id;
    std::string pipeline_run_id;
    const PipelineConfig *config_ref = nullptr;
    int worker_count = 0;
    PixelFormat input_format = PixelFormat::U16;
    InputBitDepth input_bit_depth = InputBitDepth::Bit16;
    FrameGeometry geometry;
    TimestampRef acquisition_time;
    std::vector<StageStatus> stage_statuses;
    FrameProfiling profiling;
    FrameArtifactRegistry artifacts;
    std::vector<DiagnosticMessage> diagnostics;
};

} // namespace dp1v2
