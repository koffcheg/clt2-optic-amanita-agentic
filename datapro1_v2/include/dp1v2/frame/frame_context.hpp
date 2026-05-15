#pragma once

#include "dp1v2/domain/frame_context.hpp"
#include "dp1v2/domain/frame_packet.hpp"

namespace dp1v2 {

FrameContext build_frame_context(const FramePacket &packet, int cam_index);

} // namespace dp1v2
