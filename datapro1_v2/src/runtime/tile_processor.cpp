#include "dp1v2/runtime/tile_processor.hpp"

#include <chrono>
#include <exception>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "dp1v2/domain/tile_context.hpp"

namespace dp1v2 {
namespace {

constexpr std::string_view kTileRoute = "tiles";
constexpr std::string_view kRadiometricStageKey = "radiometric_correction";

StageStatusCode toStageStatusCode(const StageExecutionStatus status)
{
    switch (status) {
    case StageExecutionStatus::Completed:
        return StageStatusCode::Completed;
    case StageExecutionStatus::Skipped:
        return StageStatusCode::Skipped;
    case StageExecutionStatus::Disabled:
        return StageStatusCode::Disabled;
    case StageExecutionStatus::Unsupported:
        return StageStatusCode::Unsupported;
    case StageExecutionStatus::Failed:
        return StageStatusCode::Failed;
    }

    return StageStatusCode::Failed;
}

TileResultStatus toTileResultStatus(const StageExecutionStatus status)
{
    switch (status) {
    case StageExecutionStatus::Completed:
        return TileResultStatus::Completed;
    case StageExecutionStatus::Skipped:
        return TileResultStatus::Skipped;
    case StageExecutionStatus::Disabled:
        return TileResultStatus::Disabled;
    case StageExecutionStatus::Unsupported:
        return TileResultStatus::Unsupported;
    case StageExecutionStatus::Failed:
        return TileResultStatus::Failed;
    }

    return TileResultStatus::Failed;
}

std::uint64_t selectFrameId(const TileTask& task)
{
    if (task.frame_id != 0) {
        return task.frame_id;
    }
    if (task.desc != nullptr && task.desc->frame_id != 0) {
        return task.desc->frame_id;
    }
    if (task.raw_view != nullptr) {
        return task.raw_view->frame_id;
    }
    return 0;
}

int selectCameraId(const TileTask& task)
{
    if (task.camera_id >= 0) {
        return task.camera_id;
    }
    if (task.raw_view != nullptr) {
        return task.raw_view->camera_id;
    }
    return -1;
}

int selectTileId(const TileTask& task)
{
    if (task.desc != nullptr) {
        return task.desc->tile_id;
    }
    if (task.raw_view != nullptr) {
        return task.raw_view->tile_id;
    }
    return -1;
}

cv::Rect selectValidArea(const TileTask& task)
{
    if (task.desc != nullptr) {
        return task.desc->valid_area;
    }
    if (task.raw_view != nullptr) {
        return task.raw_view->valid_area;
    }
    return {};
}

TileResult makeInitialResult(const TileTask& task)
{
    TileResult result{};
    result.frame_id = selectFrameId(task);
    result.camera_id = selectCameraId(task);
    result.tile_id = selectTileId(task);
    result.valid_area = selectValidArea(task);
    return result;
}

void appendTileStageStatus(
    TileContext& tile_context,
    const StageConfig& config,
    const StageStatusCode status,
    const std::string_view reason)
{
    StageStatus stage_status{};
    stage_status.stage_key = std::string(kRadiometricStageKey);
    stage_status.status = status;
    stage_status.variant = config.variant;
    stage_status.level = config.level;
    stage_status.route = std::string(kTileRoute);
    stage_status.reason = std::string(reason);
    tile_context.stage_statuses.resize(tile_context.stage_statuses.size() + 1);
    tile_context.stage_statuses.back() = std::move(stage_status);
}

void appendTileStageTiming(
    TileContext& tile_context,
    const StageConfig& config,
    const StageStatusCode status,
    const PixelFormat input_format,
    const PixelFormat output_format,
    const std::chrono::steady_clock::time_point start_time,
    const std::chrono::steady_clock::time_point end_time,
    const std::string_view reason)
{
    StageTiming timing{};
    timing.stage_key = std::string(kRadiometricStageKey);
    timing.status = status;
    timing.variant = config.variant;
    timing.level = config.level;
    timing.input_format = input_format;
    timing.output_format = output_format;
    timing.start_time = start_time;
    timing.end_time = end_time;
    timing.duration_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
        end_time - start_time).count();
    timing.reason = std::string(reason);
    tile_context.stage_timings.resize(tile_context.stage_timings.size() + 1);
    tile_context.stage_timings.back() = std::move(timing);
}

void moveTileContextToResult(TileContext& tile_context, TileResult& result)
{
    result.stage_statuses = std::move(tile_context.stage_statuses);
    result.diagnostics = std::move(tile_context.diagnostics);
    result.stage_timings = std::move(tile_context.stage_timings);
}

TileResult makeInvalidTaskResult(const TileTask& task, const std::string_view reason)
{
    TileResult result = makeInitialResult(task);
    result.status = TileResultStatus::Failed;
    result.error_code = "invalid_tile_task";
    result.reason = std::string(reason);
    result.diagnostics = std::vector<DiagnosticMessage>{
        {
            .code = "tile.invalid_task",
            .message = result.reason,
        },
    };
    return result;
}

} // namespace

TileProcessor::TileProcessor(
    RadiometricStage& radiometric_stage,
    Stage2BoundaryAdapter& stage2_boundary_adapter,
    const PipelineConfig& pipeline_config)
    : radiometric_stage_(radiometric_stage),
      stage2_boundary_adapter_(stage2_boundary_adapter),
      pipeline_config_(pipeline_config)
{
}

TileResult TileProcessor::process(
    const TileTask& task,
    Stage2BoundaryWorkspace& stage2_workspace) const
{
    TileResult result = makeInitialResult(task);

    try {
        if (task.desc == nullptr) {
            return makeInvalidTaskResult(task, "tile task is missing TileDesc");
        }
        if (task.raw_view == nullptr) {
            return makeInvalidTaskResult(task, "tile task is missing TileRawView");
        }

        TileContext tile_context{};
        tile_context.frame_id = result.frame_id;
        tile_context.tile_id = result.tile_id;

        const StageConfig& radiometric_config = pipeline_config_.stages.radiometric;
        const auto radiometric_start = std::chrono::steady_clock::now();
        const auto outcome = radiometric_stage_.process(
            RadiometricTileInput{.tile = *task.raw_view},
            tile_context,
            radiometric_config);
        const auto radiometric_end = std::chrono::steady_clock::now();

        std::optional<Stage2TileSelection> stage2_selection;
        switch (outcome.status) {
        case StageExecutionStatus::Completed:
        case StageExecutionStatus::Disabled:
        case StageExecutionStatus::Skipped:
            stage2_selection = stage2_boundary_adapter_.selectTileOutput(
                task.task_index,
                *task.raw_view,
                outcome,
                stage2_workspace);
            break;
        case StageExecutionStatus::Failed:
        case StageExecutionStatus::Unsupported:
            break;
        }

        const StageStatusCode stage_status = toStageStatusCode(outcome.status);
        const PixelFormat output_format = stage2_selection.has_value()
            ? stage2_selection->frame.pixel_format
            : task.raw_view->pixel_format;
        appendTileStageTiming(
            tile_context,
            radiometric_config,
            stage_status,
            task.raw_view->pixel_format,
            output_format,
            radiometric_start,
            radiometric_end,
            outcome.reason);
        appendTileStageStatus(tile_context, radiometric_config, stage_status, outcome.reason);

        result.status = toTileResultStatus(outcome.status);
        if (result.status == TileResultStatus::Failed) {
            result.error_code = "radiometric_tile_stage_failed";
        } else if (result.status == TileResultStatus::Unsupported) {
            result.error_code = "radiometric_tile_stage_unsupported";
        }
        result.reason = outcome.reason;
        moveTileContextToResult(tile_context, result);
        return result;
    } catch (const std::exception& ex) {
        result.status = TileResultStatus::Failed;
        result.error_code = "tile_processing_exception";
        result.reason = ex.what();
        return result;
    } catch (...) {
        result.status = TileResultStatus::Failed;
        result.error_code = "tile_processing_unknown_exception";
        result.reason = "unknown tile processing exception";
        return result;
    }
}

} // namespace dp1v2
