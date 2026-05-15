#pragma once

#include "dp1v2/frame/frame_packet_builder.hpp"
#include "dp1v2/source/source.hpp"

namespace dp1v2 {

FramePacketBuildResult normalize_raw_frame_envelope(const RawFrameEnvelope &envelope);

} // namespace dp1v2
