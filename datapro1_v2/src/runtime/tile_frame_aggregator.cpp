#include "dp1v2/runtime/tile_frame_aggregator.hpp"

#include <sstream>

namespace dp1v2 {
namespace {

std::string makeCountReason(
    const char* prefix,
    const TileFrameAggregationSummary& summary)
{
    std::ostringstream stream;
    stream << prefix
           << ": total_tiles=" << summary.total_tiles
           << " completed_tiles=" << summary.completed_tile_count
           << " failed_tiles=" << summary.failed_tile_count
           << " unsupported_tiles=" << summary.unsupported_tile_count;
    return stream.str();
}

void countTileResult(
    const TileResult& result,
    TileFrameAggregationSummary& summary)
{
    switch (result.status) {
    case TileResultStatus::Completed:
        ++summary.completed_tile_count;
        break;
    case TileResultStatus::Failed:
        ++summary.failed_tile_count;
        break;
    case TileResultStatus::Unsupported:
        ++summary.unsupported_tile_count;
        break;
    case TileResultStatus::Skipped:
        ++summary.skipped_tile_count;
        break;
    case TileResultStatus::Disabled:
        ++summary.disabled_tile_count;
        break;
    }

    summary.candidate_count += result.candidates.size();
    summary.segment_count += result.segments.size();
    summary.validated_object_count += result.objects.size();
    summary.measurement_count += result.measurements.size();
}

bool requiresUnsupportedMergeBehavior(const TileAggregationConfig& config)
{
    return config.mode == TileAggregationMode::MeasurementsAndDebugMerge
        || config.merge_enabled
        || config.deduplicate_overlap;
}

void applyAggregationMode(
    const TileAggregationConfig& config,
    TileFrameAggregationSummary& summary)
{
    if (!requiresUnsupportedMergeBehavior(config)) {
        return;
    }

    summary.terminal_status = FrameTerminalStatus::Failed;
    if (config.mode == TileAggregationMode::MeasurementsAndDebugMerge || config.merge_enabled) {
        summary.reason = "tile debug merge is not implemented";
    } else {
        summary.reason = "tile overlap deduplication is not implemented";
    }
}

void applyFrameStatusPolicy(
    const TileExecutionConfig& config,
    TileFrameAggregationSummary& summary)
{
    if (summary.terminal_status == FrameTerminalStatus::Failed) {
        return;
    }

    if (summary.unsupported_tile_count > 0) {
        summary.terminal_status = FrameTerminalStatus::Failed;
        summary.reason = makeCountReason("tile_frame_aggregation_unsupported", summary);
        return;
    }

    if (summary.failed_tile_count == 0) {
        summary.terminal_status = FrameTerminalStatus::Completed;
        return;
    }

    switch (config.frame_status_policy) {
    case TileFrameStatusPolicy::FailedIfAnyRequiredTileFailed:
        summary.terminal_status = FrameTerminalStatus::Failed;
        summary.reason = makeCountReason("tile_frame_aggregation_failed", summary);
        break;
    case TileFrameStatusPolicy::PartialIfSomeTilesFailed:
        if (summary.completed_tile_count > 0) {
            summary.terminal_status = FrameTerminalStatus::Completed;
            summary.reason = makeCountReason("tile_frame_aggregation_partial", summary);
        } else {
            summary.terminal_status = FrameTerminalStatus::Failed;
            summary.reason = makeCountReason("tile_frame_aggregation_failed", summary);
        }
        break;
    }
}

} // namespace

TileFrameAggregationSummary TileFrameAggregator::aggregate(
    const std::vector<TileResult>& tile_results,
    const TileAggregationConfig& aggregation_config,
    const TileExecutionConfig& execution_config) const
{
    TileFrameAggregationSummary summary{};
    summary.total_tiles = tile_results.size();

    for (const TileResult& result : tile_results) {
        countTileResult(result, summary);
    }

    applyAggregationMode(aggregation_config, summary);
    applyFrameStatusPolicy(execution_config, summary);

    return summary;
}

} // namespace dp1v2
