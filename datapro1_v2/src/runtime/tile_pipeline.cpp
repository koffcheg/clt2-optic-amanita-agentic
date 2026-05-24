#include "dp1v2/runtime/tile_pipeline.hpp"

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <limits>
#include <string>
#include <string_view>
#include <vector>

#include <opencv2/core.hpp>

#include "dp1v2/frame/frame_context.hpp"
#include "dp1v2/result/result_builder.hpp"

namespace dp1v2 {
namespace {

constexpr std::string_view kTileRoute = "tiles";
constexpr std::string_view kRadiometricStageKey = "radiometric_correction";

std::uint32_t clampToUint32(const std::size_t value)
{
    return value > std::numeric_limits<std::uint32_t>::max()
        ? std::numeric_limits<std::uint32_t>::max()
        : static_cast<std::uint32_t>(value);
}

StageStatusCode aggregateStageStatus(const TileFrameAggregationSummary& summary)
{
    if (summary.unsupported_tile_count > 0) {
        return StageStatusCode::Unsupported;
    }
    if (summary.failed_tile_count > 0 || summary.terminal_status == FrameTerminalStatus::Failed) {
        return StageStatusCode::Failed;
    }
    if (summary.disabled_tile_count > 0 && summary.completed_tile_count == 0) {
        return StageStatusCode::Disabled;
    }
    if (summary.skipped_tile_count > 0 && summary.completed_tile_count == 0) {
        return StageStatusCode::Skipped;
    }
    return StageStatusCode::Completed;
}

std::string makeTileSummaryReason(
    const TileExecutionSummary& execution,
    const TileFrameAggregationSummary& aggregation)
{
    if (!aggregation.reason.empty()) {
        return aggregation.reason;
    }

    std::string reason = "tile_pipeline_summary";
    reason += ": total_tiles=" + std::to_string(aggregation.total_tiles);
    reason += " completed_tiles=" + std::to_string(aggregation.completed_tile_count);
    reason += " failed_tiles=" + std::to_string(aggregation.failed_tile_count);
    reason += " unsupported_tiles=" + std::to_string(aggregation.unsupported_tile_count);
    reason += " executor_duration_ns=" + std::to_string(execution.duration_ns);
    return reason;
}

TileFrameAggregationSummary makeValidationAggregationSummary(const std::size_t tile_count)
{
    TileFrameAggregationSummary summary{};
    summary.total_tiles = tile_count;
    summary.terminal_status = FrameTerminalStatus::Failed;
    return summary;
}

TilePipelineResult makeFailureResult(
    FrameContext& context,
    const StageConfig& radiometric_config,
    const std::string_view reason,
    const std::size_t tile_count)
{
    set_frame_tile_count(context, clampToUint32(tile_count));
    record_stage_status(
        context,
        StageStatusUpdate{
            .stage_key = kRadiometricStageKey,
            .status = StageStatusCode::Failed,
            .variant = radiometric_config.variant,
            .level = radiometric_config.level,
            .route = kTileRoute,
            .reason = reason,
        });
    record_diagnostic(context, "tile_pipeline.validation_failed", reason);

    TileFrameAggregationSummary aggregation = makeValidationAggregationSummary(tile_count);
    aggregation.reason = std::string(reason);

    return TilePipelineResult{
        .lifecycle = FrameLifecycleResult{
            .status = FrameTerminalStatus::Failed,
            .reason = std::string(reason),
        },
        .sink = ResultSinkOutcome{},
        .execution = TileExecutionSummary{},
        .aggregation = aggregation,
    };
}

bool rectWithin(const cv::Rect& inner, const cv::Rect& outer)
{
    return inner.width > 0
        && inner.height > 0
        && (inner & outer) == inner;
}

std::string validatePrepOutput(const PrepTilesOutput& output)
{
    if (output.tiles.size() != output.tile_views.size()) {
        return "prep tiles and tile views size mismatch";
    }

    for (std::size_t index = 0; index < output.tiles.size(); ++index) {
        const TileDesc& desc = output.tiles[index];
        const TileRawView& view = output.tile_views[index];
        if (desc.tile_id != view.tile_id) {
            return "prep tile id mismatch at task_index=" + std::to_string(index);
        }
        if (view.image.empty()) {
            return "prep tile view image is empty at task_index=" + std::to_string(index);
        }
        if (view.geometry.width != view.image.cols || view.geometry.height != view.image.rows) {
            return "prep tile view geometry does not match image at task_index="
                + std::to_string(index);
        }

        const cv::Rect image_rect(0, 0, view.image.cols, view.image.rows);
        if (!rectWithin(view.valid_area, image_rect)) {
            return "prep tile view valid area is outside image at task_index="
                + std::to_string(index);
        }
        if (!rectWithin(desc.valid_area, image_rect)) {
            return "prep tile desc valid area is outside image at task_index="
                + std::to_string(index);
        }
    }

    return {};
}

std::vector<TileTask> buildTileTasks(const TilePipelineArgs& args)
{
    std::vector<TileTask> tasks;
    tasks.reserve(args.prep_output.tiles.size());

    for (std::size_t index = 0; index < args.prep_output.tiles.size(); ++index) {
        tasks.push_back(TileTask{
            .task_index = index,
            .frame_id = args.frame_id,
            .camera_id = args.camera_id,
            .desc = &args.prep_output.tiles[index],
            .raw_view = &args.prep_output.tile_views[index],
        });
    }

    return tasks;
}

void recordTileSummary(
    FrameContext& context,
    const StageConfig& radiometric_config,
    const TileExecutionSummary& execution,
    const TileFrameAggregationSummary& aggregation,
    const std::chrono::steady_clock::time_point start_time,
    const std::chrono::steady_clock::time_point end_time)
{
    context.profiling.cardinality.candidate_count = clampToUint32(aggregation.candidate_count);
    context.profiling.cardinality.measurement_count = clampToUint32(aggregation.measurement_count);

    const StageStatusCode status = aggregateStageStatus(aggregation);
    const std::string reason = makeTileSummaryReason(execution, aggregation);

    record_stage_timing(
        context,
        kRadiometricStageKey,
        status,
        radiometric_config.variant,
        radiometric_config.level,
        context.input_format,
        context.input_format,
        start_time,
        end_time,
        reason);
    record_stage_status(
        context,
        StageStatusUpdate{
            .stage_key = kRadiometricStageKey,
            .status = status,
            .variant = radiometric_config.variant,
            .level = radiometric_config.level,
            .route = kTileRoute,
            .reason = reason,
        });
    record_diagnostic(context, "tile_pipeline.summary", reason);
}

} // namespace

TilePipeline::TilePipeline(
    RadiometricStage& radiometric_stage,
    TileExecutor& executor,
    TileFrameAggregator& aggregator,
    VisualizationSink& visualization_sink)
    : radiometric_stage_(radiometric_stage),
      executor_(executor),
      aggregator_(aggregator),
      visualization_sink_(visualization_sink)
{
}

TilePipelineResult TilePipeline::process(const TilePipelineArgs& args)
{
    (void)args.resolved_pipeline_config;
    (void)visualization_sink_;

    const std::size_t tile_count = args.prep_output.tiles.size();
    set_frame_tile_count(args.frame_context, clampToUint32(tile_count));

    const StageConfig& radiometric_config = args.pipeline_config.stages.radiometric;
    const std::string validation_error = validatePrepOutput(args.prep_output);
    if (!validation_error.empty()) {
        return makeFailureResult(
            args.frame_context,
            radiometric_config,
            validation_error,
            tile_count);
    }

    const std::vector<TileTask> tasks = buildTileTasks(args);
    std::vector<TileResult> results(tasks.size());

    TileProcessor processor(radiometric_stage_, args.pipeline_config);

    const auto execution_start = std::chrono::steady_clock::now();
    const TileExecutionSummary execution_summary = executor_.execute(
        tasks,
        results,
        processor,
        args.tiles_config.execution);
    const auto execution_end = std::chrono::steady_clock::now();

    const TileFrameAggregationSummary aggregation_summary = aggregator_.aggregate(
        results,
        args.tiles_config.aggregation,
        args.tiles_config.execution);

    recordTileSummary(
        args.frame_context,
        radiometric_config,
        execution_summary,
        aggregation_summary,
        execution_start,
        execution_end);

    if (aggregation_summary.terminal_status == FrameTerminalStatus::Failed) {
        return TilePipelineResult{
            .lifecycle = FrameLifecycleResult{
                .status = FrameTerminalStatus::Failed,
                .reason = aggregation_summary.reason.empty()
                    ? "tile_pipeline_failed"
                    : aggregation_summary.reason,
            },
            .sink = ResultSinkOutcome{},
            .execution = execution_summary,
            .aggregation = aggregation_summary,
        };
    }

    const auto result = build_empty_result(args.frame_context);
    const auto sink = publish_result_to_sinks(result);

    return TilePipelineResult{
        .lifecycle = FrameLifecycleResult{
            .status = sink.ok() ? FrameTerminalStatus::Completed : FrameTerminalStatus::Failed,
            .reason = sink.reason,
        },
        .sink = sink,
        .execution = execution_summary,
        .aggregation = aggregation_summary,
    };
}

} // namespace dp1v2
