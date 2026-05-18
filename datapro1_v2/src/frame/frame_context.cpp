#include "dp1v2/frame/frame_context.hpp"

#include <algorithm>
#include <chrono>

namespace dp1v2 {
namespace {

constexpr const char *kRawFrameArtifactId = "raw_frame";
constexpr const char *kInputProducerStage = "input";
constexpr const char *kCanonicalFrameArtifactId = "canonical_frame";
constexpr const char *kAcquisitionProducerStage = "acquisition";
constexpr const char *kRadiometricProcessingArtifactId = "radiometric.processing_frame";
constexpr const char *kRadiometricProducerStage = "radiometric_correction";

FrameArtifactRef upsert_frame_artifact(FrameContext &context, const FrameArtifactRef &artifact) {
    const auto existing = std::find_if(
        context.artifacts.records.begin(),
        context.artifacts.records.end(),
        [&artifact](const FrameArtifactRef &record) { return record.id == artifact.id; });
    if (existing != context.artifacts.records.end()) {
        *existing = artifact;
        return *existing;
    }

    context.artifacts.records.push_back(artifact);
    return context.artifacts.records.back();
}

} // namespace

const FrameArtifactRef *find_frame_artifact_by_id(const FrameContext &context, const std::string_view id) {
    const auto existing = std::find_if(
        context.artifacts.records.begin(),
        context.artifacts.records.end(),
        [id](const FrameArtifactRef &record) { return record.id == id; });
    return existing != context.artifacts.records.end() ? &(*existing) : nullptr;
}

const FrameArtifactRef *find_frame_artifact_by_stage(
    const FrameContext &context,
    const std::string_view producer_stage) {
    const auto existing = std::find_if(
        context.artifacts.records.begin(),
        context.artifacts.records.end(),
        [producer_stage](const FrameArtifactRef &record) {
            return record.producer_stage == producer_stage;
        });
    return existing != context.artifacts.records.end() ? &(*existing) : nullptr;
}

StageTiming record_stage_timing(
    FrameContext &context,
    const std::string_view stage_key,
    const StageStatusCode status,
    const std::string_view variant,
    const std::string_view level,
    const PixelFormat input_format,
    const PixelFormat output_format,
    const std::chrono::steady_clock::time_point start_time,
    const std::chrono::steady_clock::time_point end_time,
    const std::string_view reason) {
    StageTiming timing{};
    timing.stage_key = std::string(stage_key);
    timing.status = status;
    timing.variant = std::string(variant);
    timing.level = std::string(level);
    timing.input_format = input_format;
    timing.output_format = output_format;
    timing.start_time = start_time;
    timing.end_time = end_time;
    timing.duration_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
        end_time - start_time).count();
    timing.reason = std::string(reason);
    context.profiling.stage_timings.push_back(timing);
    return context.profiling.stage_timings.back();
}

FrameContext build_frame_context(const FramePacket &packet, const int cam_index) {
    FrameContext context{};
    context.frame_id = packet.frame_id;
    context.camera_id = packet.camera_id >= 0 ? packet.camera_id : cam_index;
    context.source_id = packet.source_id;
    context.input_format = packet.pixel_format;
    context.input_bit_depth = packet.bit_depth;
    context.geometry = packet.geometry;
    if (packet.acquisition_time.has_value()) {
        context.acquisition_time = *packet.acquisition_time;
    }
    return context;
}

FrameArtifactRef register_raw_frame_artifact(FrameContext &context, const FramePacket &packet) {
    FrameArtifactRef artifact{};
    artifact.id = kRawFrameArtifactId;
    artifact.kind = FrameArtifactKind::RawFrame;
    artifact.domain = FrameArtifactDomain::Raw;
    artifact.ownership = FrameArtifactOwnership::OwnedByFramePacket;
    artifact.lifetime = FrameArtifactLifetime::InputBoundary;
    artifact.status = FrameArtifactStatus::Available;
    artifact.semantic_name = kRawFrameArtifactId;
    artifact.producer_stage = kInputProducerStage;
    artifact.pixel_format = packet.pixel_format;
    artifact.bit_depth = packet.bit_depth;
    artifact.geometry = packet.geometry;
    return upsert_frame_artifact(context, artifact);
}

FrameArtifactRef register_canonical_frame_artifact(FrameContext &context, const CanonicalFrame &frame) {
    FrameArtifactRef artifact{};
    artifact.id = kCanonicalFrameArtifactId;
    artifact.kind = FrameArtifactKind::CanonicalFrame;
    artifact.domain = FrameArtifactDomain::Raw;
    artifact.ownership = FrameArtifactOwnership::BorrowedReadOnly;
    artifact.lifetime = FrameArtifactLifetime::InputBoundary;
    artifact.status = FrameArtifactStatus::Available;
    artifact.semantic_name = kCanonicalFrameArtifactId;
    artifact.producer_stage = kAcquisitionProducerStage;
    artifact.parent_artifact_id = frame.parent_artifact_id.empty() ? kRawFrameArtifactId : frame.parent_artifact_id;
    artifact.pixel_format = frame.pixel_format;
    artifact.bit_depth = frame.bit_depth;
    artifact.geometry = frame.geometry;
    return upsert_frame_artifact(context, artifact);
}

FrameArtifactRef register_radiometric_processing_artifact(
    FrameContext &context,
    const ProcessingFrame &frame) {
    FrameArtifactRef artifact{};
    artifact.id = kRadiometricProcessingArtifactId;
    artifact.kind = FrameArtifactKind::ProcessingFrame;
    artifact.domain = FrameArtifactDomain::Processing;
    artifact.ownership = FrameArtifactOwnership::OwnedByStageOutput;
    artifact.lifetime = FrameArtifactLifetime::StageOutputScope;
    artifact.status = FrameArtifactStatus::MetadataOnly;
    artifact.semantic_name = kRadiometricProcessingArtifactId;
    artifact.producer_stage = kRadiometricProducerStage;
    artifact.parent_artifact_id = kRawFrameArtifactId;
    artifact.pixel_format = frame.pixel_format;
    artifact.bit_depth = context.input_bit_depth;
    artifact.geometry = frame.geometry;
    return upsert_frame_artifact(context, artifact);
}

} // namespace dp1v2
