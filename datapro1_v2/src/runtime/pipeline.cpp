#include "dp1v2/runtime/pipeline.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include "dp1v2/frame/frame_context.hpp"
#include "dp1v2/frame/frame_normalizer.hpp"
#include "dp1v2/result/result_builder.hpp"
#include "dp1v2/stages/radiometric_stage.hpp"

namespace dp1v2 {
namespace {

constexpr const char* kEvidenceDirectory = "datapro1_v2_output/evidence";
constexpr const char* kInverseMedianVariant = "inverse_median";

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

std::string jsonEscape(const std::string& text) {
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

std::string evidenceStem(const std::uint64_t frame_id) {
    std::ostringstream stem;
    stem << "frame_" << std::setw(6) << std::setfill('0') << frame_id << "_radiometric_inverse_median";
    return stem.str();
}

cv::Mat makeReviewImage(const cv::Mat& image) {
    if (image.empty()) {
        return {};
    }

    cv::Mat normalized;
    cv::normalize(image, normalized, 0, 255, cv::NORM_MINMAX, CV_8UC1);
    return normalized;
}

std::string writeEvidenceArtifact(
    const FramePacket& input_frame,
    const StageOutcome<RadiometricFullFrameOutput>& outcome) {
    namespace fs = std::filesystem;

    fs::create_directories(kEvidenceDirectory);
    const auto stem = evidenceStem(input_frame.frame_id);
    const auto json_path = fs::path(kEvidenceDirectory) / (stem + ".json");

    std::string image_path;
    if (outcome.status == StageExecutionStatus::Completed && !outcome.output.frame.image.empty()) {
        const auto png_path = fs::path(kEvidenceDirectory) / (stem + ".png");
        if (cv::imwrite(png_path.string(), makeReviewImage(outcome.output.frame.image))) {
            image_path = png_path.string();
        }
    }

    std::ofstream json(json_path);
    if (!json.is_open()) {
        return {};
    }

    json << "{\n";
    json << "  \"stage\": \"radiometric\",\n";
    json << "  \"variant\": \"inverse_median\",\n";
    json << "  \"route\": \"full_frame\",\n";
    json << "  \"status\": \"" << stageExecutionStatusToCstr(outcome.status) << "\",\n";
    json << "  \"reason\": \"" << jsonEscape(outcome.reason) << "\",\n";
    json << "  \"input\": {\n";
    json << "    \"frame_id\": " << input_frame.frame_id << ",\n";
    json << "    \"width\": " << input_frame.geometry.width << ",\n";
    json << "    \"height\": " << input_frame.geometry.height << ",\n";
    json << "    \"pixel_format\": \"" << pixelFormatToCstr(input_frame.pixel_format) << "\",\n";
    json << "    \"bit_depth\": " << static_cast<int>(input_frame.bit_depth) << "\n";
    json << "  }";

    if (outcome.status == StageExecutionStatus::Completed) {
        json << ",\n  \"output\": {\n";
        json << "    \"pixel_format\": \"" << pixelFormatToCstr(outcome.output.frame.pixel_format) << "\",\n";
        json << "    \"range_policy\": \"" << rangePolicyToCstr(outcome.output.frame.range_policy) << "\",\n";
        json << "    \"review_image\": \"" << jsonEscape(image_path) << "\"\n";
        json << "  }";
    }

    json << "\n}\n";
    return json_path.string();
}

bool shouldRunRadiometricStage(const StageConfig& radiometric_config) {
    return radiometric_config.enabled && radiometric_config.variant == kInverseMedianVariant;
}

} // namespace

SingleFramePipelineResult process_single_frame(
    const RawFrameEnvelope& envelope,
    const int cam_index,
    const PipelineConfig& pipeline_config,
    RadiometricStage& radiometric_stage) {
    const auto packet_result = make_frame_packet(
        envelope.frame,
        envelope.header_hint,
        pipeline_config.input_route,
        envelope.received_steady_ts);
    if (!packet_result.ok()) {
        return SingleFramePipelineResult{
            .lifecycle = FrameLifecycleResult{
                .status = FrameTerminalStatus::Failed,
                .reason = frame_packet_error_to_cstr(packet_result.error),
            },
            .sink = ResultSinkOutcome{},
        };
    }

    auto frame_context = build_frame_context(packet_result.packet, cam_index);
    frame_context.config_ref = &pipeline_config;

    std::string evidence_path;
    if (shouldRunRadiometricStage(pipeline_config.stages.radiometric)) {
        const auto radiometric_result = radiometric_stage.processFullFrame(
            RadiometricFullFrameInput{.frame = packet_result.packet},
            frame_context,
            pipeline_config.stages.radiometric);
        evidence_path = writeEvidenceArtifact(packet_result.packet, radiometric_result);
        if (radiometric_result.status == StageExecutionStatus::Failed ||
            radiometric_result.status == StageExecutionStatus::Unsupported) {
            return SingleFramePipelineResult{
                .lifecycle = FrameLifecycleResult{
                    .status = FrameTerminalStatus::Failed,
                    .reason = "radiometric_stage_failed",
                },
                .sink = ResultSinkOutcome{},
                .evidence_path = evidence_path,
            };
        }
    }

    const auto result = build_empty_result(frame_context);
    const auto sink = publish_result_to_sinks(result);

    return SingleFramePipelineResult{
        .lifecycle = FrameLifecycleResult{
            .status = sink.ok() ? FrameTerminalStatus::Completed : FrameTerminalStatus::Failed,
            .reason = sink.reason,
        },
        .sink = sink,
        .evidence_path = evidence_path,
    };
}

} // namespace dp1v2
