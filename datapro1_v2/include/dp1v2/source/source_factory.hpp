#pragma once

#include <memory>

#include "dp1v2/config/config.hpp"
#include "dp1v2/source/source.hpp"

namespace dp1v2 {

std::unique_ptr<IFrameSource> create_frame_source(const SourceConfig &config, int cam_index);

} // namespace dp1v2
