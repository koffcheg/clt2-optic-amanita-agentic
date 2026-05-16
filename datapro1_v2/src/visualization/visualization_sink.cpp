#include "dp1v2/visualization/visualization_sink.hpp"

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>
#include <utility>

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

namespace dp1v2 {
namespace {

constexpr const char* kSchemaVersion = "1.0";
constexpr const char* kRadiometricFrameFile = "radiometric_frame.png";

const char* stageExecutionStatusToCstr(const StageExecutionStatus status) {
    switch (status) {
        case StageExecutionStatus::Completed:
            return "completed";
        case StageExecutionStatus::Skipped:
            return "skipped";
        case StageExecutionStatus::Disabled:
            return "disabled";
        case StageExecutionStatus::Unsupported:
            return "unsupported";
        case StageExecutionStatus::Failed:
            return "failed";
        default:
            return "unknown";
    }
}

const char* pixelFormatToCstr(const PixelFormat format) {
    switch (format) {
        case PixelFormat::U8:
            return "U8";
        case PixelFormat::U16:
            return "U16";
        case PixelFormat::F32:
            return "F32";
        case PixelFormat::MaskU8:
            return "MaskU8";
        case PixelFormat::S16:
            return "S16";
        case PixelFormat::S32:
            return "S32";
        default:
            return "unknown";
    }
}

const char* cvDepthToCstr(const int depth) {
    switch (depth) {
        case CV_8U:
            return "CV_8U";
        case CV_16U:
            return "CV_16U";
        case CV_16S:
            return "CV_16S";
        case CV_32S:
            return "CV_32S";
        case CV_32F:
            return "CV_32F";
        default:
            return "unknown";
    }
}

const char* cvTypeToCstr(const int type) {
    switch (type) {
        case CV_8UC1:
            return "CV_8UC1";
        case CV_16UC1:
            return "CV_16UC1";
        case CV_16SC1:
            return "CV_16SC1";
        case CV_32SC1:
            return "CV_32SC1";
        case CV_32FC1:
            return "CV_32FC1";
        default:
            return "unknown";
    }
}

int expectedMatTypeFor(const PixelFormat format) {
    switch (format) {
        case PixelFormat::U8:
        case PixelFormat::MaskU8:
            return CV_8UC1;
        case PixelFormat::U16:
            return CV_16UC1;
        case PixelFormat::S16:
            return CV_16SC1;
        case PixelFormat::S32:
            return CV_32SC1;
        case PixelFormat::F32:
            return CV_32FC1;
        default:
            return -1;
    }
}

std::string describeMatTypeMismatch(const ProcessingFrame& frame, const int expected_type) {
    std::ostringstream reason;
    reason << "visualization pixel_format/type mismatch: expected pixel_format "
           << pixelFormatToCstr(frame.pixel_format);
    if (expected_type >= 0) {
        reason << " to use cv::Mat type " << cvTypeToCstr(expected_type)
               << "/depth " << cvDepthToCstr(CV_MAT_DEPTH(expected_type))
               << "/channels " << CV_MAT_CN(expected_type);
    } else {
        reason << " has no supported visualization cv::Mat type";
    }
    reason << "; actual cv::Mat type " << cvTypeToCstr(frame.image.type())
           << "/depth " << cvDepthToCstr(frame.image.depth())
           << "/channels " << frame.image.channels();
    return reason.str();
}


const char* processingDomainToCstr(const ProcessingDomain domain) {
    switch (domain) {
        case ProcessingDomain::RadiometricResidual:
            return "RadiometricResidual";
        case ProcessingDomain::RadiometricCorrected:
            return "RadiometricCorrected";
        case ProcessingDomain::EnhancedFrame:
            return "EnhancedFrame";
        case ProcessingDomain::DetectorResponse:
            return "DetectorResponse";
        default:
            return "unknown";
    }
}

const char* rangePolicyToCstr(const RangePolicy policy) {
    switch (policy) {
        case RangePolicy::Unknown:
            return "Unknown";
        case RangePolicy::RawSensorRange:
            return "RawSensorRange";
        case RangePolicy::SignedResidual:
            return "SignedResidual";
        case RangePolicy::ClippedToInputRange:
            return "ClippedToInputRange";
        case RangePolicy::NormalizedFloat:
            return "NormalizedFloat";
        case RangePolicy::DetectorResponse:
            return "DetectorResponse";
        default:
            return "unknown";
    }
}

std::string jsonEscape(const std::string_view text) {
    std::ostringstream out;
    for (const char c : text) {
        switch (c) {
            case '\\':
                out << "\\\\";
                break;
            case '"':
                out << "\\\"";
                break;
            case '\n':
                out << "\\n";
                break;
            case '\r':
                out << "\\r";
                break;
            case '\t':
                out << "\\t";
                break;
            default:
                out << c;
                break;
        }
    }
    return out.str();
}

std::filesystem::path frameDirectory(const std::string& output_dir, const std::uint64_t frame_id) {
    std::ostringstream frame_dir;
    frame_dir << "frame_" << std::setw(6) << std::setfill('0') << frame_id;
    return std::filesystem::path(output_dir) / frame_dir.str();
}

const char* rendererNameFor(const ProcessingFrame& frame) {
    switch (frame.pixel_format) {
        case PixelFormat::U8:
        case PixelFormat::U16:
            return "passthrough";
        case PixelFormat::S16:
        case PixelFormat::S32:
            return "clip_to_input_range_png_preview";
        case PixelFormat::F32:
            return "normalize_preview";
        case PixelFormat::MaskU8:
            return "passthrough_0_255";
        default:
            return "json_only";
    }
}

cv::Mat renderSignedResidualPreview(const cv::Mat& image, const PixelRange& range) {
    const double positive_max = std::max(1.0, range.max_value);
    cv::Mat preview(image.size(), CV_8UC1);
    for (int y = 0; y < image.rows; ++y) {
        auto* out = preview.ptr<std::uint8_t>(y);
        for (int x = 0; x < image.cols; ++x) {
            double value = 0.0;
            if (image.type() == CV_16SC1) {
                value = static_cast<double>(image.at<std::int16_t>(y, x));
            } else {
                value = static_cast<double>(image.at<std::int32_t>(y, x));
            }
            value = std::clamp(value, 0.0, positive_max);
            out[x] = static_cast<std::uint8_t>((value / positive_max) * 255.0);
        }
    }
    return preview;
}

cv::Mat renderFramePreview(const ProcessingFrame& frame) {
    if (frame.image.empty()) {
        return {};
    }

    if (frame.pixel_format == PixelFormat::U8 || frame.pixel_format == PixelFormat::U16) {
        return frame.image;
    }
    if (frame.pixel_format == PixelFormat::MaskU8) {
        return frame.image;
    }
    if (frame.pixel_format == PixelFormat::S16 || frame.pixel_format == PixelFormat::S32) {
        return renderSignedResidualPreview(frame.image, frame.value_range);
    }
    if (frame.pixel_format == PixelFormat::F32) {
        cv::Mat normalized;
        cv::normalize(frame.image, normalized, 0, 255, cv::NORM_MINMAX, CV_8UC1);
        return normalized;
    }
    return {};
}

bool stageListed(const std::vector<std::string>& stages, const std::string_view stage) {
    return std::find_if(stages.begin(), stages.end(), [stage](const std::string& item) {
        return item.size() == stage.size() && std::equal(item.begin(), item.end(), stage.begin());
    }) != stages.end();
}

} // namespace

VisualizationSink::VisualizationSink(VisualizationConfig config)
    : config_(std::move(config)) {}

bool VisualizationSink::enabled_for_stage(const std::string_view stage) const {
    return config_.enabled && stageListed(config_.stages, stage);
}

bool VisualizationSink::should_write_frame(const FrameContext& context) const {
    if (config_.every_n_frames < 1) {
        return false;
    }
    if (context.frame_id % static_cast<std::uint64_t>(config_.every_n_frames) != 0) {
        return false;
    }
    return config_.max_frames == 0 || frames_written_ < static_cast<std::size_t>(config_.max_frames);
}

void VisualizationSink::write_stage_output(
    const FrameContext& context,
    const std::string_view stage,
    const StageOutcome<RadiometricFullFrameOutput>& outcome) {
    if (!enabled_for_stage(stage) || !should_write_frame(context)) {
        return;
    }

    const auto frame_dir = frameDirectory(config_.output_dir, context.frame_id);
    std::filesystem::create_directories(frame_dir);

    const bool has_image = outcome.status == StageExecutionStatus::Completed && !outcome.output.frame.image.empty();
    bool wrote_png = false;
    bool render_mismatch = false;
    std::string manifest_reason = outcome.reason;
    if (has_image) {
        const ProcessingFrame& frame = outcome.output.frame;
        const int expected_type = expectedMatTypeFor(frame.pixel_format);
        if (expected_type < 0 || frame.image.type() != expected_type) {
            render_mismatch = true;
            manifest_reason = describeMatTypeMismatch(frame, expected_type);
        } else {
            const cv::Mat preview = renderFramePreview(frame);
            if (!preview.empty()) {
                wrote_png = cv::imwrite((frame_dir / kRadiometricFrameFile).string(), preview);
            }
        }
    }

    std::ofstream manifest(frame_dir / (std::string(stage) + ".json"));
    if (!manifest.is_open()) {
        return;
    }

    manifest << "{\n";
    manifest << "  \"schema_version\": \"" << kSchemaVersion << "\",\n";
    manifest << "  \"frame_id\": " << context.frame_id << ",\n";
    manifest << "  \"camera_id\": " << context.camera_id << ",\n";
    manifest << "  \"stage\": \"" << jsonEscape(stage) << "\",\n";
    manifest << "  \"status\": \"" << (render_mismatch ? "failed" : stageExecutionStatusToCstr(outcome.status)) << "\",\n";
    manifest << "  \"reason\": \"" << jsonEscape(manifest_reason) << "\",\n";
    manifest << "  \"outputs\": [";
    if (wrote_png) {
        const ProcessingFrame& frame = outcome.output.frame;
        manifest << "\n";
        manifest << "    {\n";
        manifest << "      \"name\": \"frame\",\n";
        manifest << "      \"domain\": \"processing\",\n";
        manifest << "      \"processing_domain\": \"" << processingDomainToCstr(frame.processing_domain) << "\",\n";
        manifest << "      \"pixel_format\": \"" << pixelFormatToCstr(frame.pixel_format) << "\",\n";
        manifest << "      \"range_policy\": \"" << rangePolicyToCstr(frame.range_policy) << "\",\n";
        manifest << "      \"file\": \"" << kRadiometricFrameFile << "\",\n";
        manifest << "      \"renderer\": \"" << rendererNameFor(frame) << "\"\n";
        manifest << "    }\n";
    }
    manifest << "  ]\n";
    manifest << "}\n";

    ++frames_written_;
}

} // namespace dp1v2
