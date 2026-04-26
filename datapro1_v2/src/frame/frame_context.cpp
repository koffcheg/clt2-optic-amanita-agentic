#include "dp1v2/frame/frame_context.hpp"

namespace {

constexpr float kDefaultFallbackPixelSizeM = 5E-6F;
constexpr float kDefaultFallbackFocalLengthM = 0.5F;
constexpr float kDefaultFallbackExposureLengthSec = 0.0F;
constexpr float kDefaultFallbackTurretAngleDeg = 0.0F;
constexpr float kDefaultFallbackTurretAngularVelocityDegPerSec = 0.0F;

} // namespace

namespace dp1v2 {

FrameContext build_frame_context(const FramePacket &packet, const int cam_index) {
    FrameContext context{};
    context.packet = packet;

    context.data_cam.cam_index = packet.camera_id >= 0 ? packet.camera_id : cam_index;

    context.data_frame.index_frame = static_cast<int>(packet.frame_id);
    context.data_frame.dp1_spent_time = 0;
    context.data_frame.exposureStart = packet.exposure_start.value_or(DateTime{});
    context.data_frame.width = packet.frame.cols;
    context.data_frame.height = packet.frame.rows;
    context.data_frame.exposureLength = packet.exposure_length_sec.value_or(kDefaultFallbackExposureLengthSec);
    context.data_frame.focalLength = kDefaultFallbackFocalLengthM;
    context.data_frame.pixelWidth = kDefaultFallbackPixelSizeM;
    context.data_frame.pixelHeight = kDefaultFallbackPixelSizeM;
    context.data_frame.El = kDefaultFallbackTurretAngleDeg;
    context.data_frame.Az = kDefaultFallbackTurretAngleDeg;
    context.data_frame.V_el = kDefaultFallbackTurretAngularVelocityDegPerSec;
    context.data_frame.V_az = kDefaultFallbackTurretAngularVelocityDegPerSec;
    context.data_frame.turretInfoValid = false;

    return context;
}

} // namespace dp1v2
