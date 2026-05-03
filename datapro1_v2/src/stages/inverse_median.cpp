#include "dp1v2/stages/inverse_median.hpp"

#include <algorithm>
#include <limits>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace dp1v2 {
namespace {

constexpr int kSingleChannel = 1;

template <typename Pixel>
void cswap(Pixel& first, Pixel& second)
{
    if (second < first) {
        std::swap(first, second);
    }
}

template <typename Pixel>
Pixel fixedK3Median(Pixel a, Pixel b, Pixel c)
{
    cswap(a, b);
    cswap(b, c);
    cswap(a, b);
    return b;
}

template <typename Pixel>
Pixel fixedK5Median(Pixel a, Pixel b, Pixel c, Pixel d, Pixel e)
{
    cswap(a, b);
    cswap(d, e);
    cswap(c, e);
    cswap(c, d);
    cswap(b, e);
    cswap(a, d);
    cswap(a, c);
    cswap(b, d);
    cswap(b, c);
    return c;
}

template <typename OutputPixel>
OutputPixel clampToOutputRange(std::int64_t value)
{
    const std::int64_t min_value = static_cast<std::int64_t>(std::numeric_limits<OutputPixel>::min());
    const std::int64_t max_value = static_cast<std::int64_t>(std::numeric_limits<OutputPixel>::max());
    return static_cast<OutputPixel>(std::clamp(value, min_value, max_value));
}

template <typename ResidualPixel>
std::pair<ResidualPixel, ResidualPixel> findResidualRange(const cv::Mat& residual)
{
    ResidualPixel min_value = std::numeric_limits<ResidualPixel>::max();
    ResidualPixel max_value = std::numeric_limits<ResidualPixel>::lowest();

    for (int y = 0; y < residual.rows; ++y) {
        const ResidualPixel* row = residual.ptr<ResidualPixel>(y);
        for (int x = 0; x < residual.cols; ++x) {
            min_value = std::min(min_value, row[x]);
            max_value = std::max(max_value, row[x]);
        }
    }

    return {min_value, max_value};
}

int outputTypeForInputDepth(int input_depth)
{
    if (input_depth == CV_8U) {
        return CV_8UC1;
    }
    if (input_depth == CV_16U) {
        return CV_16UC1;
    }
    throw std::invalid_argument("inverse_median supports only CV_8U and CV_16U input depth");
}

}  // namespace

InverseMedianFilter::InverseMedianFilter(InverseMedianConfig config) : config_(config)
{
    if (config_.stride < 1) {
        throw std::invalid_argument("inverse_median stride must be >= 1");
    }
    window_size_ = windowSize(config_.mode);
}

InverseMedianFilter::InverseMedianFilter(InverseMedianConfig config, const cv::Size& frame_size, int input_depth)
    : InverseMedianFilter(config)
{
    reset(frame_size, input_depth);
}

const InverseMedianConfig& InverseMedianFilter::config() const noexcept
{
    return config_;
}

void InverseMedianFilter::reset(const cv::Size& frame_size, int input_depth)
{
    clear();
    if (!config_.enabled) {
        frame_size_ = frame_size;
        input_depth_ = input_depth;
        result_.status = InverseMedianStatus::Disabled;
        return;
    }

    validateResetArgs(frame_size, input_depth);

    frame_size_ = frame_size;
    input_depth_ = input_depth;
    input_type_ = CV_MAKETYPE(input_depth_, kSingleChannel);
    residual_depth_ = residualDepthForInput(input_depth_);
    residual_type_ = CV_MAKETYPE(residual_depth_, kSingleChannel);
    median_frame_updater_ = selectMedianFrameUpdater();

    frame_ring_.resize(static_cast<std::size_t>(window_size_));
    for (cv::Mat& frame : frame_ring_) {
        frame.create(frame_size_, input_type_);
    }

    median_frame_.create(frame_size_, input_type_);
    residual_.create(frame_size_, residual_type_);

    if (config_.output_dynamic_range_mode != InverseMedianOutputMode::RawSigned) {
        converted_residual_.create(frame_size_, outputTypeForInputDepth(input_depth_));
    }

    initialized_ = true;
    result_.status = InverseMedianStatus::WarmingUp;
}

void InverseMedianFilter::updateStride(int stride)
{
    if (stride < 1) {
        throw std::invalid_argument("inverse_median stride must be >= 1");
    }
    if (stride == config_.stride) {
        return;
    }

    config_.stride = stride;
    if (initialized_ && config_.enabled) {
        resetStreamingState();
    }
}

void InverseMedianFilter::clear() noexcept
{
    frame_size_ = {};
    input_depth_ = -1;
    input_type_ = -1;
    residual_depth_ = -1;
    residual_type_ = -1;
    median_frame_updater_ = nullptr;
    next_slot_ = 0;
    selected_count_ = 0;
    frame_index_ = 0;
    initialized_ = false;
    has_median_ = false;

    frame_ring_.clear();
    median_frame_.release();
    residual_.release();
    converted_residual_.release();
    result_ = {};
}

const InverseMedianResult& InverseMedianFilter::processFrame(const cv::Mat& input_frame)
{
    if (!config_.enabled) {
        result_.status = InverseMedianStatus::Disabled;
        result_.frame_index = frame_index_++;
        result_.median_updated = false;
        result_.residual = nullptr;
        result_.converted_residual = nullptr;
        result_.median_frame = nullptr;
        return result_;
    }

    validateInputFrame(input_frame);

    const bool update_median = shouldUpdateMedian();
    if (update_median) {
        storeSelectedFrame(input_frame);
        if (selected_count_ >= window_size_) {
            recomputeMedianFrame();
            has_median_ = true;
        }
    }

    result_.frame_index = frame_index_;
    result_.median_updated = update_median && has_median_;
    result_.median_frame = nullptr;
    result_.converted_residual = nullptr;

    if (!has_median_) {
        result_.status = InverseMedianStatus::WarmingUp;
        result_.residual = nullptr;
        ++frame_index_;
        return result_;
    }

    computeResidual(input_frame);
    result_.status = InverseMedianStatus::Valid;
    result_.residual = &residual_;

    if (config_.output_dynamic_range_mode != InverseMedianOutputMode::RawSigned) {
        convertResidual();
        result_.converted_residual = &converted_residual_;
    }

    if (config_.output_median_frame) {
        result_.median_frame = &median_frame_;
    }

    ++frame_index_;
    return result_;
}

int InverseMedianFilter::windowSize(InverseMedianMode mode)
{
    switch (mode) {
        case InverseMedianMode::FixedK3:
            return 3;
        case InverseMedianMode::FixedK5:
            return 5;
    }
    throw std::invalid_argument("unsupported inverse_median mode");
}

int InverseMedianFilter::residualDepthForInput(int input_depth)
{
    if (input_depth == CV_8U) {
        return CV_16S;
    }
    if (input_depth == CV_16U) {
        return CV_32S;
    }
    throw std::invalid_argument("inverse_median supports only CV_8U and CV_16U input depth");
}

void InverseMedianFilter::validateResetArgs(const cv::Size& frame_size, int input_depth) const
{
    if (frame_size.width <= 0 || frame_size.height <= 0) {
        throw std::invalid_argument("inverse_median frame size must be positive");
    }
    if (input_depth != CV_8U && input_depth != CV_16U) {
        throw std::invalid_argument("inverse_median supports only CV_8U and CV_16U input depth");
    }
}

void InverseMedianFilter::validateInputFrame(const cv::Mat& input_frame) const
{
    if (!initialized_) {
        throw std::logic_error("inverse_median must be reset before processFrame");
    }
    if (input_frame.empty()) {
        throw std::invalid_argument("inverse_median input frame must not be empty");
    }
    if (input_frame.size() != frame_size_) {
        throw std::invalid_argument("inverse_median input frame size does not match reset size");
    }
    if (input_frame.type() != input_type_) {
        throw std::invalid_argument("inverse_median input frame type does not match reset type");
    }
}

void InverseMedianFilter::resetStreamingState() noexcept
{
    next_slot_ = 0;
    selected_count_ = 0;
    has_median_ = false;
    result_.status = InverseMedianStatus::WarmingUp;
    result_.frame_index = frame_index_;
    result_.median_updated = false;
    result_.residual = nullptr;
    result_.converted_residual = nullptr;
    result_.median_frame = nullptr;
}

InverseMedianFilter::MedianFrameUpdater InverseMedianFilter::selectMedianFrameUpdater() const
{
    if (input_depth_ == CV_8U) {
        if (config_.mode == InverseMedianMode::FixedK3) {
            return &InverseMedianFilter::recomputeMedianFrameK3U8;
        }
        if (config_.mode == InverseMedianMode::FixedK5) {
            return &InverseMedianFilter::recomputeMedianFrameK5U8;
        }
    }

    if (input_depth_ == CV_16U) {
        if (config_.mode == InverseMedianMode::FixedK3) {
            return &InverseMedianFilter::recomputeMedianFrameK3U16;
        }
        if (config_.mode == InverseMedianMode::FixedK5) {
            return &InverseMedianFilter::recomputeMedianFrameK5U16;
        }
    }

    throw std::logic_error("unsupported inverse_median mode or input depth during median updater selection");
}

bool InverseMedianFilter::shouldUpdateMedian() const noexcept
{
    return frame_index_ % static_cast<std::uint64_t>(config_.stride) == 0;
}

void InverseMedianFilter::storeSelectedFrame(const cv::Mat& input_frame)
{
    input_frame.copyTo(frame_ring_[static_cast<std::size_t>(next_slot_)]);
    next_slot_ = (next_slot_ + 1) % window_size_;
    selected_count_ = std::min(selected_count_ + 1, window_size_);
}

void InverseMedianFilter::recomputeMedianFrame()
{
    if (median_frame_updater_ == nullptr) {
        throw std::logic_error("inverse_median median updater is not initialized");
    }
    (this->*median_frame_updater_)();
}

void InverseMedianFilter::computeResidual(const cv::Mat& input_frame)
{
    if (input_depth_ == CV_8U) {
        computeResidualTyped<std::uint8_t, std::int16_t>(input_frame);
        return;
    }
    if (input_depth_ == CV_16U) {
        computeResidualTyped<std::uint16_t, std::int32_t>(input_frame);
        return;
    }
    throw std::logic_error("unsupported inverse_median input depth during residual computation");
}

void InverseMedianFilter::convertResidual()
{
    if (input_depth_ == CV_8U) {
        if (config_.output_dynamic_range_mode == InverseMedianOutputMode::ClipToInputRange) {
            clipResidualToInputRange<std::uint8_t>();
        } else if (config_.output_dynamic_range_mode == InverseMedianOutputMode::ShiftToPositive) {
            shiftResidualToPositive<std::uint8_t>();
        } else if (config_.output_dynamic_range_mode == InverseMedianOutputMode::ScaleToInputRange) {
            scaleResidualToInputRange<std::uint8_t>();
        }
        return;
    }

    if (input_depth_ == CV_16U) {
        if (config_.output_dynamic_range_mode == InverseMedianOutputMode::ClipToInputRange) {
            clipResidualToInputRange<std::uint16_t>();
        } else if (config_.output_dynamic_range_mode == InverseMedianOutputMode::ShiftToPositive) {
            shiftResidualToPositive<std::uint16_t>();
        } else if (config_.output_dynamic_range_mode == InverseMedianOutputMode::ScaleToInputRange) {
            scaleResidualToInputRange<std::uint16_t>();
        }
        return;
    }

    throw std::logic_error("unsupported inverse_median input depth during residual conversion");
}

void InverseMedianFilter::recomputeMedianFrameK3U8()
{
    recomputeMedianFrameK3Typed<std::uint8_t>();
}

void InverseMedianFilter::recomputeMedianFrameK5U8()
{
    recomputeMedianFrameK5Typed<std::uint8_t>();
}

void InverseMedianFilter::recomputeMedianFrameK3U16()
{
    recomputeMedianFrameK3Typed<std::uint16_t>();
}

void InverseMedianFilter::recomputeMedianFrameK5U16()
{
    recomputeMedianFrameK5Typed<std::uint16_t>();
}

template <typename Pixel>
void InverseMedianFilter::recomputeMedianFrameK3Typed()
{
    for (int y = 0; y < frame_size_.height; ++y) {
        const Pixel* row0 = frame_ring_[0].ptr<Pixel>(y);
        const Pixel* row1 = frame_ring_[1].ptr<Pixel>(y);
        const Pixel* row2 = frame_ring_[2].ptr<Pixel>(y);
        Pixel* median_row = median_frame_.ptr<Pixel>(y);

        for (int x = 0; x < frame_size_.width; ++x) {
            median_row[x] = fixedK3Median(row0[x], row1[x], row2[x]);
        }
    }
}

template <typename Pixel>
void InverseMedianFilter::recomputeMedianFrameK5Typed()
{
    for (int y = 0; y < frame_size_.height; ++y) {
        const Pixel* row0 = frame_ring_[0].ptr<Pixel>(y);
        const Pixel* row1 = frame_ring_[1].ptr<Pixel>(y);
        const Pixel* row2 = frame_ring_[2].ptr<Pixel>(y);
        const Pixel* row3 = frame_ring_[3].ptr<Pixel>(y);
        const Pixel* row4 = frame_ring_[4].ptr<Pixel>(y);
        Pixel* median_row = median_frame_.ptr<Pixel>(y);

        for (int x = 0; x < frame_size_.width; ++x) {
            median_row[x] = fixedK5Median(row0[x], row1[x], row2[x], row3[x], row4[x]);
        }
    }
}

template <typename InputPixel, typename ResidualPixel>
void InverseMedianFilter::computeResidualTyped(const cv::Mat& input_frame)
{
    for (int y = 0; y < frame_size_.height; ++y) {
        const InputPixel* input_row = input_frame.ptr<InputPixel>(y);
        const InputPixel* median_row = median_frame_.ptr<InputPixel>(y);
        ResidualPixel* residual_row = residual_.ptr<ResidualPixel>(y);

        for (int x = 0; x < frame_size_.width; ++x) {
            residual_row[x] = static_cast<ResidualPixel>(input_row[x]) - static_cast<ResidualPixel>(median_row[x]);
        }
    }
}

template <typename OutputPixel>
void InverseMedianFilter::clipResidualToInputRange()
{
    using ResidualPixel = std::conditional_t<std::is_same_v<OutputPixel, std::uint8_t>, std::int16_t, std::int32_t>;

    for (int y = 0; y < frame_size_.height; ++y) {
        const ResidualPixel* residual_row = residual_.ptr<ResidualPixel>(y);
        OutputPixel* output_row = converted_residual_.ptr<OutputPixel>(y);

        for (int x = 0; x < frame_size_.width; ++x) {
            output_row[x] = clampToOutputRange<OutputPixel>(residual_row[x]);
        }
    }
}

template <typename OutputPixel>
void InverseMedianFilter::shiftResidualToPositive()
{
    using ResidualPixel = std::conditional_t<std::is_same_v<OutputPixel, std::uint8_t>, std::int16_t, std::int32_t>;

    const auto [min_value, max_value] = findResidualRange<ResidualPixel>(residual_);
    (void)max_value;

    for (int y = 0; y < frame_size_.height; ++y) {
        const ResidualPixel* residual_row = residual_.ptr<ResidualPixel>(y);
        OutputPixel* output_row = converted_residual_.ptr<OutputPixel>(y);

        for (int x = 0; x < frame_size_.width; ++x) {
            const std::int64_t shifted =
                static_cast<std::int64_t>(residual_row[x]) - static_cast<std::int64_t>(min_value);
            output_row[x] = clampToOutputRange<OutputPixel>(shifted);
        }
    }
}

template <typename OutputPixel>
void InverseMedianFilter::scaleResidualToInputRange()
{
    using ResidualPixel = std::conditional_t<std::is_same_v<OutputPixel, std::uint8_t>, std::int16_t, std::int32_t>;

    const auto [min_value, max_value] = findResidualRange<ResidualPixel>(residual_);
    const std::int64_t residual_range = static_cast<std::int64_t>(max_value) - static_cast<std::int64_t>(min_value);
    const double output_max = static_cast<double>(std::numeric_limits<OutputPixel>::max());

    if (residual_range == 0) {
        converted_residual_.setTo(cv::Scalar(0));
        return;
    }

    const double alpha = output_max / static_cast<double>(residual_range);
    for (int y = 0; y < frame_size_.height; ++y) {
        const ResidualPixel* residual_row = residual_.ptr<ResidualPixel>(y);
        OutputPixel* output_row = converted_residual_.ptr<OutputPixel>(y);

        for (int x = 0; x < frame_size_.width; ++x) {
            const double scaled = static_cast<double>(residual_row[x]) * alpha;
            output_row[x] = static_cast<OutputPixel>(std::clamp(scaled, 0.0, output_max));
        }
    }
}

std::string toString(InverseMedianMode mode)
{
    switch (mode) {
        case InverseMedianMode::FixedK3:
            return "FixedK3";
        case InverseMedianMode::FixedK5:
            return "FixedK5";
    }
    return "Unknown";
}

std::string toString(InverseMedianOutputMode mode)
{
    switch (mode) {
        case InverseMedianOutputMode::RawSigned:
            return "RawSigned";
        case InverseMedianOutputMode::ClipToInputRange:
            return "ClipToInputRange";
        case InverseMedianOutputMode::ShiftToPositive:
            return "ShiftToPositive";
        case InverseMedianOutputMode::ScaleToInputRange:
            return "ScaleToInputRange";
    }
    return "Unknown";
}

std::string toString(InverseMedianStatus status)
{
    switch (status) {
        case InverseMedianStatus::Disabled:
            return "Disabled";
        case InverseMedianStatus::WarmingUp:
            return "WarmingUp";
        case InverseMedianStatus::Valid:
            return "Valid";
    }
    return "Unknown";
}

}  // namespace dp1v2
