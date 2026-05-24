#pragma once

#include <cstddef>
#include <cstdint>

#include "dp1v2/domain/tile_desc.hpp"
#include "dp1v2/domain/tile_raw_view.hpp"

namespace dp1v2 {

// Non-owning tile work item. The referenced PrepStage output and source frame
// storage must outlive the current tile execution.
struct TileTask {
    std::size_t task_index = 0;
    std::uint64_t frame_id = 0;
    int camera_id = -1;

    const TileDesc* desc = nullptr;
    const TileRawView* raw_view = nullptr;
};

} // namespace dp1v2
