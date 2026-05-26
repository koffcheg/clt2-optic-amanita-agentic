#pragma once

#include "dp1v2/config/config.hpp"
#include "dp1v2/domain/tile_result.hpp"
#include "dp1v2/runtime/stage2_boundary_adapter.hpp"
#include "dp1v2/runtime/tile_task.hpp"
#include "dp1v2/stages/radiometric_stage.hpp"

namespace dp1v2 {

class TileProcessor final {
public:
    TileProcessor(
        RadiometricStage& radiometric_stage,
        Stage2BoundaryAdapter& stage2_boundary_adapter,
        const PipelineConfig& pipeline_config);

    TileResult process(const TileTask& task, Stage2BoundaryWorkspace& stage2_workspace) const;

private:
    RadiometricStage& radiometric_stage_;
    Stage2BoundaryAdapter& stage2_boundary_adapter_;
    const PipelineConfig& pipeline_config_;
};

} // namespace dp1v2
