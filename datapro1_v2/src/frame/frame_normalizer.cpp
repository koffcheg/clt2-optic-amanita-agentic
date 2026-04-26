#include "dp1v2/frame/frame_normalizer.hpp"

namespace dp1v2 {

FramePacketBuildResult normalize_raw_frame_envelope(const RawFrameEnvelope &envelope) {
    return make_frame_packet(envelope.frame, envelope.header_hint, envelope.received_steady_ts);
}

} // namespace dp1v2
