#pragma once

#include "dp1v2/config/config.hpp"
#include "dp1v2/domain/canonical_frame.hpp"
#include "dp1v2/domain/frame_context.hpp"
#include "dp1v2/domain/frame_packet.hpp"
#include "dp1v2/stages/stage_outcome.hpp"

namespace dp1v2 {

struct InputNormalizationInput {
    const FramePacket &frame;
};

struct InputNormalizationOutput {
    CanonicalFrame frame;
};

struct InputNormalizationConfig {
    InputRouteConfig input_route;
    StageConfig stage;
    InputNormalizationResolvedConfig resolved;
};

class InputNormalizationStage final {
public:
    StageOutcome<InputNormalizationOutput> process(
        const InputNormalizationInput &input,
        FrameContext &context,
        const InputNormalizationConfig &config) const;
};

} // namespace dp1v2
