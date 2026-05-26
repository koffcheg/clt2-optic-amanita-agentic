#include "dp1v2/runtime/stage2_boundary_adapter.hpp"

#include <stdexcept>
#include <string>

namespace dp1v2 {
namespace {

constexpr std::string_view kStage2BoundaryKey = "stage2.processing_frame";

void validateBypassRange(const PixelRange& range)
{
    if (range.max_value <= range.min_value) {
        throw std::logic_error("stage2 boundary bypass requires a valid PixelRange");
    }
}

void validateBypassImageType(const cv::Mat& image, const int expected_type)
{
    if (image.type() != expected_type) {
        throw std::logic_error("stage2 boundary bypass image type does not match PixelFormat");
    }
}

void convertRawU16ToU8(const cv::Mat& input, const PixelRange& range, cv::Mat& output)
{
    validateBypassImageType(input, CV_16UC1);
    validateBypassRange(range);
    output.create(input.size(), CV_8UC1);
    const double alpha = 255.0 / (range.max_value - range.min_value);
    const double beta = -range.min_value * alpha;
    input.convertTo(output, CV_8U, alpha, beta);
}

PixelRange makeU8Range()
{
    return PixelRange{
        .min_value = 0.0,
        .max_value = 255.0,
        .black_level = 0.0,
        .saturation_level = 255.0,
    };
}

const char* statusName(const StageExecutionStatus status)
{
    switch (status) {
    case StageExecutionStatus::Completed:
        return "Completed";
    case StageExecutionStatus::Skipped:
        return "Skipped";
    case StageExecutionStatus::Disabled:
        return "Disabled";
    case StageExecutionStatus::Unsupported:
        return "Unsupported";
    case StageExecutionStatus::Failed:
        return "Failed";
    }
    return "Unknown";
}

[[noreturn]] void throwInvalidBoundaryStatus(const StageExecutionStatus status)
{
    throw std::logic_error(
        std::string("stage2 boundary cannot select output for radiometric status ") +
        statusName(status));
}

} // namespace

std::string_view Stage2BoundaryAdapter::boundaryKey() const noexcept
{
    return kStage2BoundaryKey;
}

Stage2FullFrameSelection Stage2BoundaryAdapter::selectFullFrameOutput(
    const CanonicalFrame& input,
    const StageOutcome<RadiometricFullFrameOutput>& radiometric_result,
    Stage2BoundaryWorkspace& workspace) const
{
    switch (radiometric_result.status) {
    case StageExecutionStatus::Completed:
        return Stage2FullFrameSelection{
            .frame = radiometric_result.output.frame,
            .source = Stage2BoundarySource::RadiometricOutput,
            .is_bypass = false,
        };
    case StageExecutionStatus::Disabled:
        return Stage2FullFrameSelection{
            .frame = makeBypassFrame(input, workspace),
            .source = Stage2BoundarySource::RawBypassFromDisabledRadiometric,
            .is_bypass = true,
        };
    case StageExecutionStatus::Skipped:
        return Stage2FullFrameSelection{
            .frame = makeBypassFrame(input, workspace),
            .source = Stage2BoundarySource::RawBypassFromSkippedRadiometric,
            .is_bypass = true,
        };
    case StageExecutionStatus::Failed:
    case StageExecutionStatus::Unsupported:
        throwInvalidBoundaryStatus(radiometric_result.status);
    }

    throwInvalidBoundaryStatus(radiometric_result.status);
}

Stage2TileSelection Stage2BoundaryAdapter::selectTileOutput(
    const std::size_t task_index,
    const TileRawView& input,
    const StageOutcome<RadiometricTileOutput>& radiometric_result,
    Stage2BoundaryWorkspace& workspace) const
{
    switch (radiometric_result.status) {
    case StageExecutionStatus::Completed:
        return Stage2TileSelection{
            .frame = radiometric_result.output.frame,
            .source = Stage2BoundarySource::RadiometricOutput,
            .is_bypass = false,
        };
    case StageExecutionStatus::Disabled:
        return Stage2TileSelection{
            .frame = makeBypassTileFrame(task_index, input, workspace),
            .source = Stage2BoundarySource::RawBypassFromDisabledRadiometric,
            .is_bypass = true,
        };
    case StageExecutionStatus::Skipped:
        return Stage2TileSelection{
            .frame = makeBypassTileFrame(task_index, input, workspace),
            .source = Stage2BoundarySource::RawBypassFromSkippedRadiometric,
            .is_bypass = true,
        };
    case StageExecutionStatus::Failed:
    case StageExecutionStatus::Unsupported:
        throwInvalidBoundaryStatus(radiometric_result.status);
    }

    throwInvalidBoundaryStatus(radiometric_result.status);
}

ProcessingFrame Stage2BoundaryAdapter::makeBypassFrame(
    const CanonicalFrame& input,
    Stage2BoundaryWorkspace& workspace) const
{
    ProcessingFrame frame{};
    frame.frame_id = input.frame_id;
    frame.source_frame_id = input.frame_id;
    frame.pixel_format = PixelFormat::U8;
    frame.processing_domain = ProcessingDomain::RawIntensity;
    frame.range_policy = RangePolicy::RawSensorRange;
    frame.value_range = input.pixel_range;
    frame.geometry = input.geometry;
    frame.coordinate_space = input.coordinate_space;

    switch (input.pixel_format) {
    case PixelFormat::U8:
        validateBypassImageType(input.image, CV_8UC1);
        frame.image = input.image;
        return frame;
    case PixelFormat::U16:
        convertRawU16ToU8(input.image, input.pixel_range, workspace.full_frame_bypass_u8);
        frame.image = workspace.full_frame_bypass_u8;
        frame.value_range = makeU8Range();
        return frame;
    default:
        throw std::logic_error("stage2 boundary bypass supports only U8 and U16 full-frame input");
    }
}

TileProcessingFrame Stage2BoundaryAdapter::makeBypassTileFrame(
    const std::size_t task_index,
    const TileRawView& input,
    Stage2BoundaryWorkspace& workspace) const
{
    if (task_index >= workspace.tile_bypass_u8_by_task.size()) {
        throw std::out_of_range("stage2 boundary tile bypass task index is outside workspace");
    }

    TileProcessingFrame frame{};
    frame.frame_id = input.frame_id;
    frame.tile_id = input.tile_id;
    frame.pixel_format = PixelFormat::U8;
    frame.processing_domain = ProcessingDomain::RawIntensity;
    frame.range_policy = RangePolicy::RawSensorRange;
    frame.value_range = input.pixel_range;
    frame.geometry = input.geometry;
    frame.origin_in_frame = input.origin_in_frame;
    frame.valid_area = input.valid_area;
    frame.coordinate_space = input.coordinate_space;

    switch (input.pixel_format) {
    case PixelFormat::U8:
        validateBypassImageType(input.image, CV_8UC1);
        frame.image = input.image;
        return frame;
    case PixelFormat::U16:
        convertRawU16ToU8(
            input.image,
            input.pixel_range,
            workspace.tile_bypass_u8_by_task[task_index]);
        frame.image = workspace.tile_bypass_u8_by_task[task_index];
        frame.value_range = makeU8Range();
        return frame;
    default:
        throw std::logic_error("stage2 boundary bypass supports only U8 and U16 tile input");
    }
}

} // namespace dp1v2
