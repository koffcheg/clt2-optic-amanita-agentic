#pragma once

#include "dp1v2/result/result_sink.hpp"
#include "dp1v2/runtime/runtime.hpp"
#include "dp1v2/source/source.hpp"

namespace dp1v2 {

struct SingleFramePipelineResult {
    FrameLifecycleResult lifecycle;
    ResultSinkOutcome sink;
};

SingleFramePipelineResult process_single_frame(const RawFrameEnvelope &envelope, int cam_index);

} // namespace dp1v2
