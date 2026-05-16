#pragma once

#include <cstddef>
#include <string_view>

#include "dp1v2/config/config.hpp"
#include "dp1v2/domain/frame_context.hpp"
#include "dp1v2/stages/radiometric_stage.hpp"

namespace dp1v2 {

class VisualizationSink {
public:
    explicit VisualizationSink(VisualizationConfig config);

    [[nodiscard]] bool enabled_for_stage(std::string_view stage) const;

    void write_stage_output(
        const FrameContext& context,
        std::string_view stage,
        const StageOutcome<RadiometricFullFrameOutput>& outcome);

private:
    [[nodiscard]] bool should_write_frame(const FrameContext& context) const;

    VisualizationConfig config_;
    std::size_t frames_written_ = 0;
};

} // namespace dp1v2
