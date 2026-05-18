#pragma once

#include "dp1v2/config/config.hpp"
#include "dp1v2/domain/canonical_frame.hpp"
#include "dp1v2/domain/frame_context.hpp"
#include "dp1v2/domain/frame_packet.hpp"
#include "dp1v2/stages/stage_outcome.hpp"

namespace dp1v2 {

struct AcquisitionInputNormalizationInput {
    const FramePacket &frame;
};

struct AcquisitionInputNormalizationOutput {
    CanonicalFrame frame;
};

class AcquisitionInputNormalizationStage final {
public:
    StageOutcome<AcquisitionInputNormalizationOutput> process(
        const AcquisitionInputNormalizationInput &input,
        FrameContext &context,
        const InputRouteConfig &input_route,
        const StageConfig &config) const;
};

} // namespace dp1v2
