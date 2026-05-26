#include <gtest/gtest.h>

#include <chrono>
#include <cstdint>

#include <opencv2/core.hpp>

#include "dp1v2/frame/frame_context.hpp"

namespace {

dp1v2::FramePacket makeFramePacket()
{
    cv::Mat image(3, 4, CV_16UC1);
    image.setTo(cv::Scalar(42));

    dp1v2::FramePacket packet{};
    packet.frame_id = 17;
    packet.camera_id = 3;
    packet.source_id = "source-a";
    packet.image = image;
    packet.pixel_format = dp1v2::PixelFormat::U16;
    packet.bit_depth = dp1v2::InputBitDepth::Bit16;
    packet.geometry = dp1v2::FrameGeometry{.width = image.cols, .height = image.rows};
    return packet;
}

dp1v2::CanonicalFrame makeCanonicalFrameFromPacket(const dp1v2::FramePacket& packet)
{
    dp1v2::CanonicalFrame frame{};
    frame.frame_id = packet.frame_id;
    frame.camera_id = packet.camera_id;
    frame.source_id = packet.source_id;
    frame.image = packet.image;
    frame.image_ownership = dp1v2::CanonicalPayloadOwnership::BorrowedReadOnly;
    frame.pixel_format = packet.pixel_format;
    frame.bit_depth = packet.bit_depth;
    frame.geometry = packet.geometry;
    frame.parent_artifact_id = "raw_frame";
    frame.normalization.source = dp1v2::NormalizationSource::Stage0;
    frame.normalization.binned = false;
    return frame;
}

dp1v2::ProcessingFrame makeRadiometricFrame()
{
    cv::Mat image(3, 4, CV_16SC1);
    image.setTo(cv::Scalar(-7));

    dp1v2::ProcessingFrame frame{};
    frame.frame_id = 17;
    frame.source_frame_id = 17;
    frame.image = image;
    frame.pixel_format = dp1v2::PixelFormat::S16;
    frame.processing_domain = dp1v2::ProcessingDomain::RadiometricResidual;
    frame.range_policy = dp1v2::RangePolicy::SignedResidual;
    frame.geometry = dp1v2::FrameGeometry{.width = image.cols, .height = image.rows};
    return frame;
}

} // namespace

TEST(FrameContextTest, RegisterRawFrameArtifactRecordsInputBoundaryPayload)
{
    const dp1v2::FramePacket packet = makeFramePacket();
    dp1v2::FrameContext context = dp1v2::build_frame_context(packet, 99);

    const dp1v2::FrameArtifactRef artifact =
        dp1v2::register_raw_frame_artifact(context, packet);

    EXPECT_EQ(artifact.id, "raw_frame");
    EXPECT_EQ(artifact.kind, dp1v2::FrameArtifactKind::RawFrame);
    EXPECT_EQ(artifact.domain, dp1v2::FrameArtifactDomain::Raw);
    EXPECT_EQ(artifact.ownership, dp1v2::FrameArtifactOwnership::OwnedByFramePacket);
    EXPECT_EQ(artifact.lifetime, dp1v2::FrameArtifactLifetime::InputBoundary);
    EXPECT_EQ(artifact.status, dp1v2::FrameArtifactStatus::Available);
    EXPECT_EQ(artifact.producer_stage, "input");
    EXPECT_TRUE(artifact.parent_artifact_id.empty());
    EXPECT_EQ(artifact.pixel_format, dp1v2::PixelFormat::U16);
    EXPECT_EQ(artifact.bit_depth, dp1v2::InputBitDepth::Bit16);
    EXPECT_EQ(artifact.geometry.width, 4);
    EXPECT_EQ(artifact.geometry.height, 3);

    const dp1v2::FrameArtifactRef *by_id =
        dp1v2::find_frame_artifact_by_id(context, "raw_frame");
    ASSERT_NE(by_id, nullptr);
    EXPECT_EQ(by_id->kind, dp1v2::FrameArtifactKind::RawFrame);
}

TEST(FrameContextTest, RegisterRadiometricArtifactIsStageOutputScopeMetadata)
{
    const dp1v2::FramePacket packet = makeFramePacket();
    const dp1v2::ProcessingFrame frame = makeRadiometricFrame();
    dp1v2::FrameContext context = dp1v2::build_frame_context(packet, 99);
    dp1v2::register_raw_frame_artifact(context, packet);

    const dp1v2::FrameArtifactRef artifact =
        dp1v2::register_radiometric_processing_artifact(context, frame);

    EXPECT_EQ(artifact.id, "radiometric.processing_frame");
    EXPECT_EQ(artifact.kind, dp1v2::FrameArtifactKind::ProcessingFrame);
    EXPECT_EQ(artifact.domain, dp1v2::FrameArtifactDomain::Processing);
    EXPECT_EQ(artifact.ownership, dp1v2::FrameArtifactOwnership::OwnedByStageOutput);
    EXPECT_EQ(artifact.lifetime, dp1v2::FrameArtifactLifetime::StageOutputScope);
    EXPECT_EQ(artifact.status, dp1v2::FrameArtifactStatus::MetadataOnly);
    EXPECT_EQ(artifact.semantic_name, "radiometric.processing_frame");
    EXPECT_EQ(artifact.producer_stage, "radiometric_correction");
    EXPECT_EQ(artifact.parent_artifact_id, "canonical_frame");
    EXPECT_EQ(artifact.pixel_format, dp1v2::PixelFormat::S16);
    EXPECT_EQ(artifact.bit_depth, dp1v2::InputBitDepth::Bit16);

    const dp1v2::FrameArtifactRef *by_stage =
        dp1v2::find_frame_artifact_by_stage(context, "radiometric_correction");
    ASSERT_NE(by_stage, nullptr);
    EXPECT_EQ(by_stage->status, dp1v2::FrameArtifactStatus::MetadataOnly);
}

TEST(FrameContextTest, RegisterProcessingFrameArtifactUsesCallerProvidedProvenance)
{
    const dp1v2::FramePacket packet = makeFramePacket();
    const dp1v2::ProcessingFrame frame = makeRadiometricFrame();
    dp1v2::FrameContext context = dp1v2::build_frame_context(packet, 99);

    const dp1v2::FrameArtifactRef artifact =
        dp1v2::register_processing_frame_artifact(
            context,
            frame,
            "custom.processing_frame",
            "custom.semantic",
            "custom_stage",
            "parent_frame");

    EXPECT_EQ(artifact.id, "custom.processing_frame");
    EXPECT_EQ(artifact.kind, dp1v2::FrameArtifactKind::ProcessingFrame);
    EXPECT_EQ(artifact.domain, dp1v2::FrameArtifactDomain::Processing);
    EXPECT_EQ(artifact.ownership, dp1v2::FrameArtifactOwnership::OwnedByStageOutput);
    EXPECT_EQ(artifact.lifetime, dp1v2::FrameArtifactLifetime::StageOutputScope);
    EXPECT_EQ(artifact.status, dp1v2::FrameArtifactStatus::MetadataOnly);
    EXPECT_EQ(artifact.semantic_name, "custom.semantic");
    EXPECT_EQ(artifact.producer_stage, "custom_stage");
    EXPECT_EQ(artifact.parent_artifact_id, "parent_frame");
    EXPECT_EQ(artifact.pixel_format, dp1v2::PixelFormat::S16);
    EXPECT_EQ(artifact.bit_depth, dp1v2::InputBitDepth::Bit16);
    EXPECT_EQ(artifact.geometry.width, 4);
    EXPECT_EQ(artifact.geometry.height, 3);
}

TEST(FrameContextTest, RegisterStage2BoundaryArtifactUsesStableIdAndProducer)
{
    const dp1v2::FramePacket packet = makeFramePacket();
    dp1v2::ProcessingFrame frame = makeRadiometricFrame();
    frame.pixel_format = dp1v2::PixelFormat::U8;
    frame.processing_domain = dp1v2::ProcessingDomain::RawIntensity;
    dp1v2::FrameContext context = dp1v2::build_frame_context(packet, 99);

    const dp1v2::FrameArtifactRef artifact =
        dp1v2::register_stage2_boundary_processing_artifact(
            context,
            frame,
            "radiometric_bypass",
            "canonical_frame");

    EXPECT_EQ(artifact.id, "stage2.processing_frame");
    EXPECT_EQ(artifact.semantic_name, "stage2.processing_frame");
    EXPECT_EQ(artifact.producer_stage, "radiometric_bypass");
    EXPECT_EQ(artifact.parent_artifact_id, "canonical_frame");
    EXPECT_EQ(artifact.status, dp1v2::FrameArtifactStatus::MetadataOnly);
    EXPECT_EQ(artifact.pixel_format, dp1v2::PixelFormat::U8);
}

TEST(FrameContextTest, RegisterCanonicalFrameArtifactPassThroughKeepsBorrowedInputBoundaryAvailable)
{
    const dp1v2::FramePacket packet = makeFramePacket();
    dp1v2::FrameContext context = dp1v2::build_frame_context(packet, 99);

    const dp1v2::CanonicalFrame frame = makeCanonicalFrameFromPacket(packet);
    const dp1v2::FrameArtifactRef artifact =
        dp1v2::register_canonical_frame_artifact(context, frame);

    EXPECT_EQ(artifact.id, "canonical_frame");
    EXPECT_EQ(artifact.ownership, dp1v2::FrameArtifactOwnership::BorrowedReadOnly);
    EXPECT_EQ(artifact.lifetime, dp1v2::FrameArtifactLifetime::InputBoundary);
    EXPECT_EQ(artifact.status, dp1v2::FrameArtifactStatus::Available);
}

TEST(FrameContextTest, RegisterCanonicalFrameArtifactBinnedUsesStageOutputMetadataOnly)
{
    const dp1v2::FramePacket packet = makeFramePacket();
    dp1v2::FrameContext context = dp1v2::build_frame_context(packet, 99);

    dp1v2::CanonicalFrame frame = makeCanonicalFrameFromPacket(packet);
    frame.image = cv::Mat(2, 2, CV_16UC1);
    frame.image_ownership = dp1v2::CanonicalPayloadOwnership::OwnedBinned;
    frame.geometry = dp1v2::FrameGeometry{.width = 2, .height = 2};
    frame.normalization.binned = true;
    frame.normalization.bin_factor_x = 2;
    frame.normalization.bin_factor_y = 2;
    frame.normalization.binning_mode = dp1v2::BinningMode::Average;

    const dp1v2::FrameArtifactRef artifact =
        dp1v2::register_canonical_frame_artifact(context, frame);

    EXPECT_EQ(artifact.ownership, dp1v2::FrameArtifactOwnership::OwnedByStageOutput);
    EXPECT_EQ(artifact.lifetime, dp1v2::FrameArtifactLifetime::StageOutputScope);
    EXPECT_EQ(artifact.status, dp1v2::FrameArtifactStatus::MetadataOnly);
    EXPECT_EQ(artifact.parent_artifact_id, "raw_frame");
    EXPECT_EQ(artifact.geometry.width, 2);
    EXPECT_EQ(artifact.geometry.height, 2);
}

TEST(FrameContextTest, RecordStageTimingStoresMinimalP2Timing)
{
    dp1v2::FrameContext context{};
    const auto start = std::chrono::steady_clock::now();
    const auto end = start + std::chrono::microseconds(25);

    const dp1v2::StageTiming timing = dp1v2::record_stage_timing(
        context,
        "radiometric_correction",
        dp1v2::StageStatusCode::Completed,
        "inverse_median",
        "L0",
        dp1v2::PixelFormat::U16,
        dp1v2::PixelFormat::S16,
        start,
        end);

    ASSERT_EQ(context.profiling.stage_timings.size(), 1U);
    EXPECT_EQ(timing.stage_key, "radiometric_correction");
    EXPECT_EQ(timing.status, dp1v2::StageStatusCode::Completed);
    EXPECT_EQ(timing.variant, "inverse_median");
    EXPECT_EQ(timing.level, "L0");
    EXPECT_EQ(timing.input_format, dp1v2::PixelFormat::U16);
    EXPECT_EQ(timing.output_format, dp1v2::PixelFormat::S16);
    EXPECT_EQ(timing.duration_ns, 25000);
    EXPECT_EQ(context.profiling.stage_timings.front().stage_key, "radiometric_correction");
}
