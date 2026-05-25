#include "dp1v2/stages/prep_stage.hpp"

#include <algorithm>
#include <cmath>
#include <limits>

namespace dp1v2 {
namespace {

constexpr const char *kFullFrameVariant = "full_frame";
constexpr const char *kTilesVariant = "tiles";

struct TileGrid {
    int columns = 0;
    int rows = 0;
};

StageOutcome<PrepFullFrameOutput> fullFrameFailure(
    const StageExecutionStatus status,
    const std::string &reason)
{
    return StageOutcome<PrepFullFrameOutput>{
        .status = status,
        .reason = reason,
    };
}

StageOutcome<PrepTilesOutput> tilesFailure(
    const StageExecutionStatus status,
    const std::string &reason)
{
    return StageOutcome<PrepTilesOutput>{
        .status = status,
        .reason = reason,
    };
}

std::optional<std::string> validateCanonicalFrame(const CanonicalFrame &frame)
{
    if (frame.image.empty()) {
        return "canonical frame image is empty";
    }

    if (frame.image.channels() != 1) {
        return "canonical frame image must be single-channel";
    }

    if (frame.geometry.width != frame.image.cols || frame.geometry.height != frame.image.rows ||
        frame.geometry.width <= 0 || frame.geometry.height <= 0) {
        return "canonical frame geometry does not match image payload";
    }

    if (frame.coordinate_space != CoordinateSpace::FrameGlobal) {
        return "canonical frame coordinate space must be frame-global";
    }

    return std::nullopt;
}

TileGrid calculateTileGridFromCount(
    const int tile_count,
    const int frame_width,
    const int frame_height)
{
    const double frame_aspect =
        static_cast<double>(frame_width) / static_cast<double>(frame_height);

    int best_columns = tile_count;
    int best_rows = 1;
    double best_score = std::numeric_limits<double>::infinity();

    for (int rows = 1; rows <= tile_count; ++rows) {
        if (tile_count % rows != 0) {
            continue;
        }

        const int columns = tile_count / rows;
        const double grid_aspect =
            static_cast<double>(columns) / static_cast<double>(rows);
        const double score = std::abs(std::log(grid_aspect / frame_aspect));

        if (score < best_score) {
            best_score = score;
            best_columns = columns;
            best_rows = rows;
        }
    }

    return TileGrid{
        .columns = best_columns,
        .rows = best_rows,
    };
}

// User-facing prep tile parameter validation is performed by the config parser.
// This guard protects PrepStage from manually constructed invalid resolved
// configs in tests or future direct callers, and prevents invalid tile loops or
// ROI geometry.
std::optional<std::string> validateResolvedTilesConfigInvariant(
    const PrepTilesParametersConfig &tiles_config)
{
    if (tiles_config.tile_count.has_value() && *tiles_config.tile_count <= 0) {
        return "prep tiles tile_count must be positive";
    }

    if (tiles_config.overlap_x < 0 || tiles_config.overlap_y < 0) {
        return "prep tiles overlap must be non-negative";
    }

    if (tiles_config.tile_count.has_value()) {
        return std::nullopt;
    }

    if (tiles_config.tile_width <= 0 || tiles_config.tile_height <= 0) {
        return "prep tiles dimensions must be positive";
    }

    if (tiles_config.overlap_x >= tiles_config.tile_width) {
        return "prep tiles overlap_x must be smaller than tile_width";
    }

    if (tiles_config.overlap_y >= tiles_config.tile_height) {
        return "prep tiles overlap_y must be smaller than tile_height";
    }

    return std::nullopt;
}

std::optional<std::string> appendTileForCore(
    PrepTilesOutput &output,
    const CanonicalFrame &frame,
    const cv::Rect &frame_rect,
    const PrepTilesParametersConfig &tiles_config,
    int &tile_id,
    const cv::Rect &core,
    const bool validate_resolved_overlap)
{
    if (validate_resolved_overlap && tiles_config.overlap_x >= core.width) {
        return "prep tiles overlap_x must be smaller than resolved tile core width";
    }

    if (validate_resolved_overlap && tiles_config.overlap_y >= core.height) {
        return "prep tiles overlap_y must be smaller than resolved tile core height";
    }

    const cv::Rect expanded(
        core.x - tiles_config.overlap_x,
        core.y - tiles_config.overlap_y,
        core.width + 2 * tiles_config.overlap_x,
        core.height + 2 * tiles_config.overlap_y);

    const cv::Rect roi_with_border = expanded & frame_rect;
    const cv::Rect valid_area(
        core.x - roi_with_border.x,
        core.y - roi_with_border.y,
        core.width,
        core.height);

    TileDesc desc{};
    desc.frame_id = frame.frame_id;
    desc.tile_id = tile_id++;
    desc.roi_with_border = roi_with_border;
    desc.valid_area = valid_area;
    desc.origin_in_frame = roi_with_border.tl();
    desc.coordinate_space = CoordinateSpace::TileLocal;

    TileRawView view{};
    view.frame_id = frame.frame_id;
    view.camera_id = frame.camera_id;
    view.tile_id = desc.tile_id;
    view.image = frame.image(desc.roi_with_border);
    view.pixel_format = frame.pixel_format;
    view.bit_depth = frame.bit_depth;
    view.pixel_range = frame.pixel_range;
    view.geometry = FrameGeometry{
        .width = desc.roi_with_border.width,
        .height = desc.roi_with_border.height,
        .origin_px = desc.roi_with_border.tl(),
    };
    view.origin_in_frame = desc.origin_in_frame;
    view.valid_area = desc.valid_area;
    view.coordinate_space = CoordinateSpace::TileLocal;

    output.tiles.push_back(desc);
    output.tile_views.push_back(view);
    return std::nullopt;
}

} // namespace

PrepStage::PrepStage(PrepResolvedConfig resolved_config)
    : tiles_config_(resolved_config.tiles)
{
}

StageCapabilities PrepStage::capabilities() const noexcept
{
    return StageCapabilities{
        .supports_full_frame = true,
        .supports_tiles = tiles_config_.has_value(),
        .supports_roi = false,
        .supports_adaptive_roi = false,
    };
}

StageOutcome<PrepFullFrameOutput> PrepStage::process(
    const PrepFullFrameInput &input,
    FrameContext &context,
    const StageConfig &config)
{
    (void)context;

    if (!config.enabled) {
        return fullFrameFailure(StageExecutionStatus::Disabled, "prep stage is disabled");
    }

    if (config.variant != kFullFrameVariant) {
        return fullFrameFailure(
            StageExecutionStatus::Unsupported,
            "unsupported prep full-frame variant: " + config.variant);
    }

    const CanonicalFrame &frame = input.frame;
    if (const auto validation_error = validateCanonicalFrame(frame)) {
        return fullFrameFailure(StageExecutionStatus::Failed, *validation_error);
    }

    return StageOutcome<PrepFullFrameOutput>{
        .status = StageExecutionStatus::Completed,
        .output = PrepFullFrameOutput{.frame = &input.frame},
        .reason = "",
    };
}

StageOutcome<PrepTilesOutput> PrepStage::process(
    const PrepTilesInput &input,
    FrameContext &context,
    const StageConfig &config)
{
    (void)context;

    if (!config.enabled) {
        return tilesFailure(StageExecutionStatus::Disabled, "prep stage is disabled");
    }

    if (config.variant != kTilesVariant) {
        return tilesFailure(
            StageExecutionStatus::Unsupported,
            "unsupported prep tiles variant: " + config.variant);
    }

    if (!tiles_config_.has_value()) {
        return tilesFailure(
            StageExecutionStatus::Failed,
            "prep tiles resolved configuration is missing");
    }

    const CanonicalFrame &frame = input.frame;
    if (const auto validation_error = validateCanonicalFrame(frame)) {
        return tilesFailure(StageExecutionStatus::Failed, *validation_error);
    }

    const PrepTilesParametersConfig &tiles_config = *tiles_config_;
    if (const auto config_error = validateResolvedTilesConfigInvariant(tiles_config)) {
        return tilesFailure(StageExecutionStatus::Failed, *config_error);
    }

    const int frame_width = frame.geometry.width;
    const int frame_height = frame.geometry.height;
    const cv::Rect frame_rect(0, 0, frame_width, frame_height);

    PrepTilesOutput output{};
    int tile_id = 0;

    if (tiles_config.tile_count.has_value()) {
        const TileGrid grid = calculateTileGridFromCount(
            *tiles_config.tile_count,
            frame_width,
            frame_height);

        output.tiles.reserve(static_cast<std::size_t>(grid.columns * grid.rows));
        output.tile_views.reserve(static_cast<std::size_t>(grid.columns * grid.rows));

        for (int row = 0; row < grid.rows; ++row) {
            const int y0 = row * frame_height / grid.rows;
            const int y1 = (row + 1) * frame_height / grid.rows;

            for (int col = 0; col < grid.columns; ++col) {
                const int x0 = col * frame_width / grid.columns;
                const int x1 = (col + 1) * frame_width / grid.columns;

                const cv::Rect core(
                    x0,
                    y0,
                    x1 - x0,
                    y1 - y0);

                if (const auto tile_error = appendTileForCore(
                        output, frame, frame_rect, tiles_config, tile_id, core, true)) {
                    return tilesFailure(StageExecutionStatus::Failed, *tile_error);
                }
            }
        }

        return StageOutcome<PrepTilesOutput>{
            .status = StageExecutionStatus::Completed,
            .output = output,
            .reason = "",
        };
    }

    const int tile_columns = (frame_width + tiles_config.tile_width - 1) / tiles_config.tile_width;
    const int tile_rows = (frame_height + tiles_config.tile_height - 1) / tiles_config.tile_height;
    output.tiles.reserve(static_cast<std::size_t>(tile_columns * tile_rows));
    output.tile_views.reserve(static_cast<std::size_t>(tile_columns * tile_rows));

    for (int y = 0; y < frame_height; y += tiles_config.tile_height) {
        for (int x = 0; x < frame_width; x += tiles_config.tile_width) {
            const cv::Rect core(
                x,
                y,
                std::min(tiles_config.tile_width, frame_width - x),
                std::min(tiles_config.tile_height, frame_height - y));

            if (const auto tile_error = appendTileForCore(
                    output, frame, frame_rect, tiles_config, tile_id, core, false)) {
                return tilesFailure(StageExecutionStatus::Failed, *tile_error);
            }
        }
    }

    return StageOutcome<PrepTilesOutput>{
        .status = StageExecutionStatus::Completed,
        .output = output,
        .reason = "",
    };
}
} // namespace dp1v2
