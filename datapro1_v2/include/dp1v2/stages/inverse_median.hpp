#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include <opencv2/core.hpp>

namespace dp1v2 {

enum class InverseMedianMode {
    FixedK3,
    FixedK5,
};

enum class InverseMedianOutputMode {
    RawSigned,
    ClipToInputRange,
    ShiftToPositive,
    ScaleToInputRange,
};

enum class InverseMedianStatus {
    Disabled,
    WarmingUp,
    Valid,
};

struct InverseMedianConfig {
    bool enabled = true;
    InverseMedianMode mode = InverseMedianMode::FixedK3;
    int stride = 1;
    bool output_median_frame = false;
    InverseMedianOutputMode output_dynamic_range_mode = InverseMedianOutputMode::RawSigned;
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
};

// Stateful temporal inverse median filter for single-channel CV_8U or CV_16U frames.
//
// reset() preallocates all frame, median, residual, and converted-output buffers.
// processFrame() does not mutate input_frame and returns views into internal buffers.
//
// Output formats:
// - residual: CV_16SC1 for CV_8UC1 input, CV_32SC1 for CV_16UC1 input.
// - converted_residual: CV_8UC1 or CV_16UC1 when output_dynamic_range_mode
//   is not RawSigned.
// - median_frame: same type as the input frame when output_median_frame is true.
//
// Errors are reported with std::invalid_argument for invalid frame/configuration values and
// std::logic_error for lifecycle misuse, such as calling processFrame() before reset().
class InverseMedianFilter {
public:
    explicit InverseMedianFilter(InverseMedianConfig config);

    const InverseMedianConfig& config() const noexcept;

    void reset(const cv::Size& frame_size, int input_depth);
    void clear() noexcept;

    const InverseMedianResult& processFrame(const cv::Mat& input_frame);

private:
    static int windowSize(InverseMedianMode mode);
    static int residualDepthForInput(int input_depth);

    void validateResetArgs(const cv::Size& frame_size, int input_depth) const;
    void validateInputFrame(const cv::Mat& input_frame) const;

    bool shouldUpdateMedian() const noexcept;
    void storeSelectedFrame(const cv::Mat& input_frame);
    void recomputeMedianFrame();
    void computeResidual(const cv::Mat& input_frame);
    void convertResidual();

    template <typename Pixel>
    void recomputeMedianFrameTyped();

    template <typename InputPixel, typename ResidualPixel>
    void computeResidualTyped(const cv::Mat& input_frame);

    template <typename OutputPixel>
    void clipResidualToInputRange();

    template <typename OutputPixel>
    void shiftResidualToPositive();

    template <typename OutputPixel>
    void scaleResidualToInputRange();

    InverseMedianConfig config_;
    cv::Size frame_size_;
    int input_depth_ = -1;
    int input_type_ = -1;
    int residual_depth_ = -1;
    int residual_type_ = -1;
    int window_size_ = 0;
    int next_slot_ = 0;
    int selected_count_ = 0;
    std::uint64_t frame_index_ = 0;
    bool initialized_ = false;
    bool has_median_ = false;

    std::vector<cv::Mat> frame_ring_;
    cv::Mat median_frame_;
    cv::Mat residual_;
    cv::Mat converted_residual_;
    InverseMedianResult result_;
};

std::string toString(InverseMedianMode mode);
std::string toString(InverseMedianOutputMode mode);
std::string toString(InverseMedianStatus status);

}  // namespace dp1v2
