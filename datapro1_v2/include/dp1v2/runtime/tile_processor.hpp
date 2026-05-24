#pragma once

#include "dp1v2/config/config.hpp"
#include "dp1v2/domain/tile_result.hpp"
#include "dp1v2/runtime/tile_task.hpp"
#include "dp1v2/stages/radiometric_stage.hpp"

namespace dp1v2 {

class TileProcessor final {
public:
    TileProcessor(RadiometricStage& radiometric_stage, const PipelineConfig& pipeline_config);

    TileResult process(const TileTask& task) const;

private:
    RadiometricStage& radiometric_stage_;
    const PipelineConfig& pipeline_config_;
};

} // namespace dp1v2
