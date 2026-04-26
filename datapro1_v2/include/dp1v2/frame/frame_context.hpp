#pragma once

#include "dp1v2/frame/frame_packet.hpp"
#include "datarpoTypes.h"

namespace dp1v2 {

struct FrameContext {
    FramePacket packet;
    TDataCam data_cam;
    TDataFrame data_frame;
};

FrameContext build_frame_context(const FramePacket &packet, int cam_index);

} // namespace dp1v2
