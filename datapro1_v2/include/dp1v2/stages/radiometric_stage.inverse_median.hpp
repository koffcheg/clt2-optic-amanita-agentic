#pragma once

#include <chrono>
#include <cstdint>
#include <string>

#include <opencv2/core.hpp>

#include "dp1v2/config/config.hpp"
#include "dp1v2/runtime/cyclic_frame_buffer.hpp"

namespace dp1v2 {

enum class InverseMedianStatus {
    Disabled,
    WarmingUp,
    Valid,
};

enum class InverseMedianBinningOwner {
    None,
    Camera,
    Dp1,
};

enum class InverseMedianPixelFormat {
    Unknown,
    U8,
    U16,
    S16,
    S32,
};

enum class InverseMedianProcessingDomain {
    Unknown,
    RadiometricResidual,
};

enum class InverseMedianRangePolicy {
    Unknown,
    SignedResidual,
    ClippedToInputRange,
};

using InverseMedianConfig = InverseMedianParametersConfig;

struct InverseMedianProcessContext {
    std::uint64_t frame_index = 0;
};

struct InverseMedianInputRoute {
    cv::Size frame_size;
    int input_depth = -1;
    int bit_depth = 0;
    int range_min = 0;
    int range_max = 0;
    int binning_factor = 1;
    InverseMedianBinningOwner binning_owner = InverseMedianBinningOwner::None;
};

struct InverseMedianFrameView {
    const cv::Mat* image = nullptr;
    InverseMedianPixelFormat pixel_format = InverseMedianPixelFormat::Unknown;
    InverseMedianProcessingDomain processing_domain = InverseMedianProcessingDomain::Unknown;
    InverseMedianRangePolicy range_policy = InverseMedianRangePolicy::Unknown;
};

struct InverseMedianTiming {
    std::chrono::nanoseconds median_update{0};
    std::chrono::nanoseconds residual{0};
    std::chrono::nanoseconds conversion{0};
    std::chrono::nanoseconds total{0};
};

struct InverseMedianResult {
    InverseMedianStatus status = InverseMedianStatus::WarmingUp;
    std::uint64_t frame_index = 0;
    bool median_updated = false;
    // Views into buffers owned by InverseMedianFilter.
    // Valid until the next reset(), clear(), or processFrame().
    const cv::Mat* residual = nullptr;
    const cv::Mat* converted_residual = nullptr;
    const cv::Mat* median_frame = nullptr;
    InverseMedianFrameView residual_view;
    InverseMedianFrameView converted_residual_view;
    InverseMedianFrameView median_frame_view;
    InverseMedianTiming timing;
};

// Stateful temporal inverse median filter for single-channel CV_8U or CV_16U frames.
//
// The sized constructor or reset() preallocates all frame, median, residual,
// and converted-output buffers.
// processFrame() does not mutate input_frame and returns views into internal buffers.
//
// Output formats:
// - residual: CV_16SC1 for CV_8UC1 input, CV_32SC1 for CV_16UC1 input.
// - converted_residual: CV_8UC1 or CV_16UC1 when output_dynamic_range_mode
//   is not RawSigned.
// - median_frame: same type as the input frame when output_median_frame is true.
//
// Errors are reported with std::invalid_argument for invalid frame/configuration values and
// std::logic_error for lifecycle misuse, such as calling processFrame() before initialization.
class InverseMedianFilter {
public:
    explicit InverseMedianFilter(InverseMedianConfig config);
    InverseMedianFilter(InverseMedianConfig config, const cv::Size& frame_size, int input_depth);

    const InverseMedianConfig& config() const noexcept;

    void reset(const cv::Size& frame_size, int input_depth);
    void reset(const InverseMedianInputRoute& route);
    bool requiresReset(const InverseMedianInputRoute& route) const noexcept;
    void resetIfRouteChanged(const InverseMedianInputRoute& route);
    void updateStride(int stride);
    void clear() noexcept;

    const InverseMedianResult& processFrame(const cv::Mat& input_frame);
    const InverseMedianResult& processFrame(
        const cv::Mat& input_frame,
        const InverseMedianProcessContext& process_context);

private:
    using MedianFrameUpdater = void (InverseMedianFilter::*)();

    static int windowSize(InverseMedianMode mode);
    static int residualDepthForInput(int input_depth);
    static InverseMedianInputRoute makeDefaultRoute(const cv::Size& frame_size, int input_depth);

    void validateResetArgs(const InverseMedianInputRoute& route) const;
    void validateInputFrame(const cv::Mat& input_frame) const;

    void resetStreamingState() noexcept;
    MedianFrameUpdater selectMedianFrameUpdater() const;
    bool shouldUpdateMedian(std::uint64_t frame_index) const noexcept;
    void storeSelectedFrame(const cv::Mat& input_frame);
    void recomputeMedianFrame();
    void computeResidual(const cv::Mat& input_frame);
    void convertResidual();

    void recomputeMedianFrameK3U8();
    void recomputeMedianFrameK5U8();
    void recomputeMedianFrameK3U16();
    void recomputeMedianFrameK5U16();

    template <typename Pixel>
    void recomputeMedianFrameK3Typed();

    template <typename Pixel>
    void recomputeMedianFrameK5Typed();

    template <typename InputPixel, typename ResidualPixel>
    void computeResidualTyped(const cv::Mat& input_frame);

    template <typename OutputPixel>
    void clipResidualToInputRange();

    InverseMedianConfig config_;
    InverseMedianInputRoute input_route_;
    cv::Size frame_size_;
    int input_depth_ = -1;
    int input_type_ = -1;
    int residual_depth_ = -1;
    int residual_type_ = -1;
    int window_size_ = 0;
    MedianFrameUpdater median_frame_updater_ = nullptr;
    std::uint64_t frame_index_ = 0;
    bool initialized_ = false;
    bool has_median_ = false;

    CyclicFrameBuffer frame_buffer_;
    cv::Mat median_frame_;
    cv::Mat residual_;
    cv::Mat converted_residual_;
    InverseMedianResult result_;
};

std::string toString(InverseMedianMode mode);
std::string toString(InverseMedianOutputMode mode);
std::string toString(InverseMedianStatus status);

}  // namespace dp1v2
