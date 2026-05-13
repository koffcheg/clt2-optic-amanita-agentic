#include "dp1v2/stages/inverse_median.hpp"

#include <algorithm>
#include <chrono>
#include <limits>
#include <stdexcept>
#include <type_traits>

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

InverseMedianPixelFormat inputPixelFormatForDepth(int input_depth)
{
    if (input_depth == CV_8U) {
        return InverseMedianPixelFormat::U8;
    }
    if (input_depth == CV_16U) {
        return InverseMedianPixelFormat::U16;
    }
    return InverseMedianPixelFormat::Unknown;
}

const char* inputPixelFormatNameForDepth(int input_depth)
{
    if (input_depth == CV_8U) {
        return "U8";
    }
    if (input_depth == CV_16U) {
        return "U16";
    }
    return "Unknown";
}

InverseMedianPixelFormat residualPixelFormatForDepth(int input_depth)
{
    if (input_depth == CV_8U) {
        return InverseMedianPixelFormat::S16;
    }
    if (input_depth == CV_16U) {
        return InverseMedianPixelFormat::S32;
    }
    return InverseMedianPixelFormat::Unknown;
}

bool routesEqual(const InverseMedianInputRoute& lhs, const InverseMedianInputRoute& rhs) noexcept
{
    return lhs.frame_size == rhs.frame_size && lhs.input_depth == rhs.input_depth && lhs.bit_depth == rhs.bit_depth &&
           lhs.range_min == rhs.range_min && lhs.range_max == rhs.range_max &&
           lhs.binning_factor == rhs.binning_factor && lhs.binning_owner == rhs.binning_owner;
}

CyclicFrameBufferMetadata makeHistoryBufferMetadata(const InverseMedianInputRoute& route, int frame_type)
{
    CyclicFrameBufferMetadata metadata{};
    metadata.frame_size = route.frame_size;
    metadata.frame_type = frame_type;
    metadata.input_bit_depth = route.bit_depth;
    metadata.range_min = static_cast<double>(route.range_min);
    metadata.range_max = static_cast<double>(route.range_max);
    metadata.pixel_format = inputPixelFormatNameForDepth(route.input_depth);
    metadata.range_policy = "RawSensorRange";
    return metadata;
}

void clearResultViews(InverseMedianResult& result) noexcept
{
    result.residual = nullptr;
    result.converted_residual = nullptr;
    result.median_frame = nullptr;
    result.residual_view = {};
    result.converted_residual_view = {};
    result.median_frame_view = {};
}

void clearResultTiming(InverseMedianResult& result) noexcept
{
    result.timing = {};
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
    reset(makeDefaultRoute(frame_size, input_depth));
}

void InverseMedianFilter::reset(const InverseMedianInputRoute& route)
{
    clear();
    if (!config_.enabled) {
        input_route_ = route;
        frame_size_ = route.frame_size;
        input_depth_ = route.input_depth;
        result_.status = InverseMedianStatus::Disabled;
        return;
    }

    validateResetArgs(route);

    input_route_ = route;
    frame_size_ = route.frame_size;
    input_depth_ = route.input_depth;
    input_type_ = CV_MAKETYPE(input_depth_, kSingleChannel);
    residual_depth_ = residualDepthForInput(input_depth_);
    residual_type_ = CV_MAKETYPE(residual_depth_, kSingleChannel);
    median_frame_updater_ = selectMedianFrameUpdater();

    frame_buffer_.resetStorage(window_size_, makeHistoryBufferMetadata(input_route_, input_type_));
    median_frame_.create(frame_size_, input_type_);
    residual_.create(frame_size_, residual_type_);

    if (config_.output_dynamic_range_mode != InverseMedianOutputMode::RawSigned) {
        converted_residual_.create(frame_size_, outputTypeForInputDepth(input_depth_));
    }

    initialized_ = true;
    result_.status = InverseMedianStatus::WarmingUp;
}

bool InverseMedianFilter::requiresReset(const InverseMedianInputRoute& route) const noexcept
{
    return !initialized_ || !routesEqual(input_route_, route);
}

void InverseMedianFilter::resetIfRouteChanged(const InverseMedianInputRoute& route)
{
    if (requiresReset(route)) {
        reset(route);
    }
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
    frame_index_ = 0;
    initialized_ = false;
    has_median_ = false;

    input_route_ = {};
    frame_buffer_.clear();
    median_frame_.release();
    residual_.release();
    converted_residual_.release();
    result_ = {};
}

const InverseMedianResult& InverseMedianFilter::processFrame(const cv::Mat& input_frame)
{
    using Clock = std::chrono::steady_clock;
    const auto total_start = Clock::now();
    clearResultTiming(result_);

    if (!config_.enabled) {
        result_.status = InverseMedianStatus::Disabled;
        result_.frame_index = frame_index_++;
        result_.median_updated = false;
        clearResultViews(result_);
        result_.timing.total = Clock::now() - total_start;
        return result_;
    }

    validateInputFrame(input_frame);

    const bool update_median = shouldUpdateMedian();
    if (update_median) {
        const auto median_start = Clock::now();
        storeSelectedFrame(input_frame);
        if (frame_buffer_.full()) {
            recomputeMedianFrame();
            has_median_ = true;
        }
        result_.timing.median_update = Clock::now() - median_start;
    }

    result_.frame_index = frame_index_;
    result_.median_updated = update_median && has_median_;
    clearResultViews(result_);

    if (!has_median_) {
        result_.status = InverseMedianStatus::WarmingUp;
        ++frame_index_;
        result_.timing.total = Clock::now() - total_start;
        return result_;
    }

    const auto residual_start = Clock::now();
    computeResidual(input_frame);
    result_.timing.residual = Clock::now() - residual_start;
    result_.status = InverseMedianStatus::Valid;
    result_.residual = &residual_;
    result_.residual_view = InverseMedianFrameView{
        .image = &residual_,
        .pixel_format = residualPixelFormatForDepth(input_depth_),
        .processing_domain = InverseMedianProcessingDomain::RadiometricResidual,
        .range_policy = InverseMedianRangePolicy::SignedResidual,
    };

    if (config_.output_dynamic_range_mode != InverseMedianOutputMode::RawSigned) {
        const auto conversion_start = Clock::now();
        convertResidual();
        result_.timing.conversion = Clock::now() - conversion_start;
        result_.converted_residual = &converted_residual_;
        result_.converted_residual_view = InverseMedianFrameView{
            .image = &converted_residual_,
            .pixel_format = inputPixelFormatForDepth(input_depth_),
            .processing_domain = InverseMedianProcessingDomain::RadiometricResidual,
            .range_policy = InverseMedianRangePolicy::ClippedToInputRange,
        };
    }

    if (config_.output_median_frame) {
        result_.median_frame = &median_frame_;
        result_.median_frame_view = InverseMedianFrameView{
            .image = &median_frame_,
            .pixel_format = inputPixelFormatForDepth(input_depth_),
            .processing_domain = InverseMedianProcessingDomain::Unknown,
            .range_policy = InverseMedianRangePolicy::Unknown,
        };
    }

    ++frame_index_;
    result_.timing.total = Clock::now() - total_start;
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

InverseMedianInputRoute InverseMedianFilter::makeDefaultRoute(const cv::Size& frame_size, int input_depth)
{
    InverseMedianInputRoute route{};
    route.frame_size = frame_size;
    route.input_depth = input_depth;
    route.binning_factor = 1;

    if (input_depth == CV_8U) {
        route.bit_depth = 8;
        route.range_min = 0;
        route.range_max = 255;
    } else if (input_depth == CV_16U) {
        route.bit_depth = 16;
        route.range_min = 0;
        route.range_max = 65535;
    }

    return route;
}

void InverseMedianFilter::validateResetArgs(const InverseMedianInputRoute& route) const
{
    if (route.frame_size.width <= 0 || route.frame_size.height <= 0) {
        throw std::invalid_argument("inverse_median frame size must be positive");
    }
    if (route.input_depth != CV_8U && route.input_depth != CV_16U) {
        throw std::invalid_argument("inverse_median supports only CV_8U and CV_16U input depth");
    }
    if (route.input_depth == CV_8U && route.bit_depth != 8) {
        throw std::invalid_argument("inverse_median CV_8U route requires bit_depth == 8");
    }
    if (route.input_depth == CV_16U &&
        route.bit_depth != 10 && route.bit_depth != 12 && route.bit_depth != 14 && route.bit_depth != 16) {
        throw std::invalid_argument("inverse_median CV_16U route requires bit_depth 10, 12, 14, or 16");
    }
    if (route.range_min < 0 || route.range_max <= route.range_min) {
        throw std::invalid_argument("inverse_median input route range must be positive and ordered");
    }
    if (route.binning_factor < 1) {
        throw std::invalid_argument("inverse_median input route binning_factor must be >= 1");
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
    frame_buffer_.resetState();
    has_median_ = false;
    result_.status = InverseMedianStatus::WarmingUp;
    result_.frame_index = frame_index_;
    result_.median_updated = false;
    clearResultViews(result_);
    clearResultTiming(result_);
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
    frame_buffer_.store(input_frame);
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
        } else {
            throw std::logic_error("unsupported inverse_median output mode during residual conversion");
        }
        return;
    }

    if (input_depth_ == CV_16U) {
        if (config_.output_dynamic_range_mode == InverseMedianOutputMode::ClipToInputRange) {
            clipResidualToInputRange<std::uint16_t>();
        } else {
            throw std::logic_error("unsupported inverse_median output mode during residual conversion");
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
        const Pixel* row0 = frame_buffer_.slots[0].ptr<Pixel>(y);
        const Pixel* row1 = frame_buffer_.slots[1].ptr<Pixel>(y);
        const Pixel* row2 = frame_buffer_.slots[2].ptr<Pixel>(y);
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
        const Pixel* row0 = frame_buffer_.slots[0].ptr<Pixel>(y);
        const Pixel* row1 = frame_buffer_.slots[1].ptr<Pixel>(y);
        const Pixel* row2 = frame_buffer_.slots[2].ptr<Pixel>(y);
        const Pixel* row3 = frame_buffer_.slots[3].ptr<Pixel>(y);
        const Pixel* row4 = frame_buffer_.slots[4].ptr<Pixel>(y);
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
