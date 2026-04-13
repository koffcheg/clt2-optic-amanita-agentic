#pragma once

#include <cstdint>

namespace dp1v2 {

enum class PixelType {
    MONO8,
    MONO10,
    MONO12,
    MONO14,
    MONO16,
};

struct FramePacket {
    PixelType pixel_type = PixelType::MONO16;
    std::uint64_t frame_id = 0;
};

} // namespace dp1v2
