#include "dp1v2/frame/frame_context.hpp"

namespace dp1v2 {

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

} // namespace dp1v2
