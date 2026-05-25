#include "dp1v2/runtime/runtime_threading.hpp"

#include <opencv2/core.hpp>

namespace dp1v2 {

void apply_tile_route_opencv_thread_limit(const ResolvedPipelineConfig& config)
{
    if (!config.prep.tiles.has_value()) {
        return;
    }

    const int opencv_num_threads = config.prep.tiles->execution.opencv_num_threads;
    if (opencv_num_threads > 0) {
        cv::setNumThreads(opencv_num_threads);
    }
}

} // namespace dp1v2
