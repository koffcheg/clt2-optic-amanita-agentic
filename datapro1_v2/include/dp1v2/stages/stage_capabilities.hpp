#pragma once

namespace dp1v2 {

struct StageCapabilities {
    bool supports_full_frame = false;
    bool supports_tiles = false;
    bool supports_roi = false;
    bool supports_adaptive_roi = false;
};

} // namespace dp1v2
