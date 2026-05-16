#include <gtest/gtest.h>

#include <algorithm>
#include <cstdint>
#include <stdexcept>
#include <vector>

#include <opencv2/core.hpp>

#include "dp1v2/stages/radiometric_stage.inverse_median.hpp"

namespace {

cv::Mat makeU8Mat(int width, int height, const std::vector<std::uint8_t>& values)
{
    CV_Assert(static_cast<int>(values.size()) == width * height);
    cv::Mat mat(height, width, CV_8UC1);
    int index = 0;
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            mat.at<std::uint8_t>(y, x) = values[static_cast<std::size_t>(index++)];
        }
    }
    return mat;
}

cv::Mat makeU16Mat(int width, int height, const std::vector<std::uint16_t>& values)
{
    CV_Assert(static_cast<int>(values.size()) == width * height);
    cv::Mat mat(height, width, CV_16UC1);
    int index = 0;
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            mat.at<std::uint16_t>(y, x) = values[static_cast<std::size_t>(index++)];
        }
    }
    return mat;
}

template <typename Pixel>
Pixel matValue(const cv::Mat& mat, int y, int x)
{
    return mat.at<Pixel>(y, x);
}

cv::Mat referenceMedian(const std::vector<cv::Mat>& frames)
{
    CV_Assert(!frames.empty());
    const cv::Size size = frames.front().size();
    const int type = frames.front().type();
    cv::Mat median(size, type);

    for (const cv::Mat& frame : frames) {
        CV_Assert(frame.size() == size);
        CV_Assert(frame.type() == type);
    }

    for (int y = 0; y < size.height; ++y) {
        for (int x = 0; x < size.width; ++x) {
            std::vector<int> values;
            values.reserve(frames.size());
            for (const cv::Mat& frame : frames) {
                if (type == CV_8UC1) {
                    values.push_back(static_cast<int>(matValue<std::uint8_t>(frame, y, x)));
                } else {
                    values.push_back(static_cast<int>(matValue<std::uint16_t>(frame, y, x)));
                }
            }
            std::sort(values.begin(), values.end());
            const int median_value = values[values.size() / 2U];
            if (type == CV_8UC1) {
                median.at<std::uint8_t>(y, x) = static_cast<std::uint8_t>(median_value);
            } else {
                median.at<std::uint16_t>(y, x) = static_cast<std::uint16_t>(median_value);
            }
        }
    }

    return median;
}

cv::Mat referenceResidual(const cv::Mat& input, const cv::Mat& median)
{
    CV_Assert(input.size() == median.size());
    CV_Assert(input.type() == median.type());

    const int residual_type = input.type() == CV_8UC1 ? CV_16SC1 : CV_32SC1;
    cv::Mat residual(input.size(), residual_type);
    for (int y = 0; y < input.rows; ++y) {
        for (int x = 0; x < input.cols; ++x) {
            if (input.type() == CV_8UC1) {
                residual.at<std::int16_t>(y, x) =
                    static_cast<std::int16_t>(input.at<std::uint8_t>(y, x)) -
                    static_cast<std::int16_t>(median.at<std::uint8_t>(y, x));
            } else {
                residual.at<std::int32_t>(y, x) =
                    static_cast<std::int32_t>(input.at<std::uint16_t>(y, x)) -
                    static_cast<std::int32_t>(median.at<std::uint16_t>(y, x));
            }
        }
    }
    return residual;
}

void expectMatEqual(const cv::Mat& actual, const cv::Mat& expected)
{
    ASSERT_FALSE(actual.empty());
    ASSERT_EQ(actual.size(), expected.size());
    ASSERT_EQ(actual.type(), expected.type());

    for (int y = 0; y < actual.rows; ++y) {
        for (int x = 0; x < actual.cols; ++x) {
            switch (actual.type()) {
                case CV_8UC1:
                    EXPECT_EQ(actual.at<std::uint8_t>(y, x), expected.at<std::uint8_t>(y, x));
                    break;
                case CV_16UC1:
                    EXPECT_EQ(actual.at<std::uint16_t>(y, x), expected.at<std::uint16_t>(y, x));
                    break;
                case CV_16SC1:
                    EXPECT_EQ(actual.at<std::int16_t>(y, x), expected.at<std::int16_t>(y, x));
                    break;
                case CV_32SC1:
                    EXPECT_EQ(actual.at<std::int32_t>(y, x), expected.at<std::int32_t>(y, x));
                    break;
                default:
                    FAIL() << "unsupported matrix type in test helper";
            }
        }
    }
}

dp1v2::InverseMedianConfig makeConfig(
    dp1v2::InverseMedianMode mode,
    int stride,
    dp1v2::InverseMedianOutputMode output_mode = dp1v2::InverseMedianOutputMode::RawSigned)
{
    dp1v2::InverseMedianConfig config{};
    config.enabled = true;
    config.mode = mode;
    config.stride = stride;
    config.output_median_frame = true;
    config.output_dynamic_range_mode = output_mode;
    return config;
}

dp1v2::InverseMedianInputRoute makeU8Route(cv::Size size)
{
    dp1v2::InverseMedianInputRoute route{};
    route.frame_size = size;
    route.input_depth = CV_8U;
    route.bit_depth = 8;
    route.range_min = 0;
    route.range_max = 255;
    route.binning_factor = 1;
    route.binning_owner = dp1v2::InverseMedianBinningOwner::None;
    return route;
}

}  // namespace

TEST(InverseMedianFilter, ProcessFrame_WhenFixedK3U8Stride1_WarmsUpThenReturnsExactSignedResidual)
{
    const cv::Size size(2, 2);
    std::vector<cv::Mat> frames{
        makeU8Mat(2, 2, {1, 10, 100, 250}),
        makeU8Mat(2, 2, {3, 20, 80, 200}),
        makeU8Mat(2, 2, {2, 30, 90, 240}),
    };

    dp1v2::InverseMedianFilter filter(makeConfig(dp1v2::InverseMedianMode::FixedK3, 1), size, CV_8U);

    const dp1v2::InverseMedianResult& first = filter.processFrame(frames[0]);
    EXPECT_EQ(first.status, dp1v2::InverseMedianStatus::WarmingUp);
    EXPECT_EQ(first.frame_index, 0U);
    EXPECT_EQ(first.residual, nullptr);

    const dp1v2::InverseMedianResult& second = filter.processFrame(frames[1]);
    EXPECT_EQ(second.status, dp1v2::InverseMedianStatus::WarmingUp);
    EXPECT_EQ(second.frame_index, 1U);
    EXPECT_EQ(second.residual, nullptr);

    const dp1v2::InverseMedianResult& third = filter.processFrame(frames[2]);
    const cv::Mat expected_median = referenceMedian(frames);
    const cv::Mat expected_residual = referenceResidual(frames[2], expected_median);

    ASSERT_EQ(third.status, dp1v2::InverseMedianStatus::Valid);
    EXPECT_EQ(third.frame_index, 2U);
    EXPECT_TRUE(third.median_updated);
    ASSERT_NE(third.residual, nullptr);
    ASSERT_NE(third.median_frame, nullptr);
    EXPECT_EQ(third.residual->type(), CV_16SC1);
    EXPECT_EQ(third.residual_view.pixel_format, dp1v2::InverseMedianPixelFormat::S16);
    EXPECT_EQ(third.residual_view.range_policy, dp1v2::InverseMedianRangePolicy::SignedResidual);
    expectMatEqual(*third.median_frame, expected_median);
    expectMatEqual(*third.residual, expected_residual);
}

TEST(InverseMedianFilter, ProcessFrame_WhenFixedK3U16_ReturnsExactMedianAndInt32Residual)
{
    const cv::Size size(2, 2);
    std::vector<cv::Mat> frames{
        makeU16Mat(2, 2, {1000, 5000, 10000, 30000}),
        makeU16Mat(2, 2, {1200, 4500, 9000, 32000}),
        makeU16Mat(2, 2, {1100, 5500, 9500, 31000}),
    };

    dp1v2::InverseMedianFilter filter(makeConfig(dp1v2::InverseMedianMode::FixedK3, 1), size, CV_16U);
    const dp1v2::InverseMedianResult* result = nullptr;
    for (const cv::Mat& frame : frames) {
        result = &filter.processFrame(frame);
    }

    ASSERT_NE(result, nullptr);
    const cv::Mat expected_median = referenceMedian(frames);
    const cv::Mat expected_residual = referenceResidual(frames.back(), expected_median);

    ASSERT_EQ(result->status, dp1v2::InverseMedianStatus::Valid);
    EXPECT_TRUE(result->median_updated);
    ASSERT_NE(result->residual, nullptr);
    ASSERT_NE(result->median_frame, nullptr);
    EXPECT_EQ(result->residual->type(), CV_32SC1);
    EXPECT_EQ(result->residual_view.pixel_format, dp1v2::InverseMedianPixelFormat::S32);
    expectMatEqual(*result->median_frame, expected_median);
    expectMatEqual(*result->residual, expected_residual);
}

TEST(InverseMedianFilter, ProcessFrame_WhenFixedK5U8_ReturnsExactMedianAndInt16Residual)
{
    const cv::Size size(2, 2);
    std::vector<cv::Mat> frames{
        makeU8Mat(2, 2, {10, 100, 200, 250}),
        makeU8Mat(2, 2, {30, 120, 180, 240}),
        makeU8Mat(2, 2, {20, 110, 190, 245}),
        makeU8Mat(2, 2, {40, 130, 170, 230}),
        makeU8Mat(2, 2, {25, 125, 195, 235}),
    };

    dp1v2::InverseMedianFilter filter(makeConfig(dp1v2::InverseMedianMode::FixedK5, 1), size, CV_8U);
    const dp1v2::InverseMedianResult* result = nullptr;
    for (const cv::Mat& frame : frames) {
        result = &filter.processFrame(frame);
    }

    ASSERT_NE(result, nullptr);
    const cv::Mat expected_median = referenceMedian(frames);
    const cv::Mat expected_residual = referenceResidual(frames.back(), expected_median);

    ASSERT_EQ(result->status, dp1v2::InverseMedianStatus::Valid);
    EXPECT_TRUE(result->median_updated);
    ASSERT_NE(result->residual, nullptr);
    ASSERT_NE(result->median_frame, nullptr);
    EXPECT_EQ(result->residual->type(), CV_16SC1);
    EXPECT_EQ(result->residual_view.pixel_format, dp1v2::InverseMedianPixelFormat::S16);
    expectMatEqual(*result->median_frame, expected_median);
    expectMatEqual(*result->residual, expected_residual);
}

TEST(InverseMedianFilter, ProcessFrame_WhenFixedK5U16_ReturnsExactMedianAndInt32Residual)
{
    const cv::Size size(2, 2);
    std::vector<cv::Mat> frames{
        makeU16Mat(2, 2, {100, 1000, 10000, 50000}),
        makeU16Mat(2, 2, {300, 1200, 9000, 40000}),
        makeU16Mat(2, 2, {200, 1100, 9500, 45000}),
        makeU16Mat(2, 2, {400, 1300, 9700, 42000}),
        makeU16Mat(2, 2, {250, 1250, 9900, 43000}),
    };

    dp1v2::InverseMedianFilter filter(makeConfig(dp1v2::InverseMedianMode::FixedK5, 1), size, CV_16U);
    const dp1v2::InverseMedianResult* result = nullptr;
    for (const cv::Mat& frame : frames) {
        result = &filter.processFrame(frame);
    }

    ASSERT_NE(result, nullptr);
    const cv::Mat expected_median = referenceMedian(frames);
    const cv::Mat expected_residual = referenceResidual(frames.back(), expected_median);

    ASSERT_EQ(result->status, dp1v2::InverseMedianStatus::Valid);
    EXPECT_TRUE(result->median_updated);
    ASSERT_NE(result->residual, nullptr);
    ASSERT_NE(result->median_frame, nullptr);
    EXPECT_EQ(result->residual->type(), CV_32SC1);
    EXPECT_EQ(result->residual_view.pixel_format, dp1v2::InverseMedianPixelFormat::S32);
    expectMatEqual(*result->median_frame, expected_median);
    expectMatEqual(*result->residual, expected_residual);
}

TEST(InverseMedianFilter, ProcessFrame_WhenStride2SkipsFrame_UsesHoldLastMedian)
{
    const cv::Size size(1, 1);
    std::vector<cv::Mat> selected_frames{
        makeU8Mat(1, 1, {10}),
        makeU8Mat(1, 1, {30}),
        makeU8Mat(1, 1, {20}),
    };
    std::vector<cv::Mat> stream{
        selected_frames[0],
        makeU8Mat(1, 1, {200}),
        selected_frames[1],
        makeU8Mat(1, 1, {201}),
        selected_frames[2],
        makeU8Mat(1, 1, {25}),
    };

    dp1v2::InverseMedianFilter filter(makeConfig(dp1v2::InverseMedianMode::FixedK3, 2), size, CV_8U);

    for (std::size_t i = 0; i + 1U < stream.size(); ++i) {
        filter.processFrame(stream[i]);
    }

    const dp1v2::InverseMedianResult& skipped = filter.processFrame(stream.back());
    const cv::Mat expected_median = referenceMedian(selected_frames);
    const cv::Mat expected_residual = referenceResidual(stream.back(), expected_median);

    ASSERT_EQ(skipped.status, dp1v2::InverseMedianStatus::Valid);
    EXPECT_EQ(skipped.frame_index, 5U);
    EXPECT_FALSE(skipped.median_updated);
    ASSERT_NE(skipped.residual, nullptr);
    expectMatEqual(*skipped.residual, expected_residual);
}

TEST(InverseMedianFilter, ProcessFrame_WhenStride3SelectsEveryThirdFrame_ReturnsValidAfterSelectedWindow)
{
    const cv::Size size(1, 1);
    std::vector<cv::Mat> selected_frames{
        makeU8Mat(1, 1, {9}),
        makeU8Mat(1, 1, {3}),
        makeU8Mat(1, 1, {6}),
    };
    std::vector<cv::Mat> stream{
        selected_frames[0],
        makeU8Mat(1, 1, {100}),
        makeU8Mat(1, 1, {101}),
        selected_frames[1],
        makeU8Mat(1, 1, {102}),
        makeU8Mat(1, 1, {103}),
        selected_frames[2],
    };

    dp1v2::InverseMedianFilter filter(makeConfig(dp1v2::InverseMedianMode::FixedK3, 3), size, CV_8U);
    const dp1v2::InverseMedianResult* result = nullptr;
    for (const cv::Mat& frame : stream) {
        result = &filter.processFrame(frame);
    }

    const cv::Mat expected_median = referenceMedian(selected_frames);
    const cv::Mat expected_residual = referenceResidual(stream.back(), expected_median);

    ASSERT_NE(result, nullptr);
    ASSERT_EQ(result->status, dp1v2::InverseMedianStatus::Valid);
    EXPECT_EQ(result->frame_index, 6U);
    EXPECT_TRUE(result->median_updated);
    ASSERT_NE(result->residual, nullptr);
    expectMatEqual(*result->residual, expected_residual);
}

TEST(InverseMedianFilter, ProcessFrame_WhenRawSignedResidualIsNegative_PreservesNegativeValue)
{
    const cv::Size size(1, 1);
    std::vector<cv::Mat> stream{
        makeU8Mat(1, 1, {100}),
        makeU8Mat(1, 1, {100}),
        makeU8Mat(1, 1, {100}),
        makeU8Mat(1, 1, {50}),
    };

    dp1v2::InverseMedianFilter filter(makeConfig(dp1v2::InverseMedianMode::FixedK3, 1), size, CV_8U);
    const dp1v2::InverseMedianResult* result = nullptr;
    for (const cv::Mat& frame : stream) {
        result = &filter.processFrame(frame);
    }

    ASSERT_NE(result, nullptr);
    ASSERT_EQ(result->status, dp1v2::InverseMedianStatus::Valid);
    ASSERT_NE(result->residual, nullptr);
    ASSERT_EQ(result->residual->type(), CV_16SC1);
    EXPECT_EQ(result->residual->at<std::int16_t>(0, 0), -50);
    EXPECT_EQ(result->converted_residual, nullptr);
}

TEST(InverseMedianFilter, ProcessFrame_WhenClipToInputRange_ClampsNegativeAndKeepsPositiveResidual)
{
    const cv::Size size(2, 1);
    std::vector<cv::Mat> stream{
        makeU8Mat(2, 1, {100, 100}),
        makeU8Mat(2, 1, {100, 100}),
        makeU8Mat(2, 1, {100, 100}),
        makeU8Mat(2, 1, {50, 140}),
    };

    dp1v2::InverseMedianFilter filter(
        makeConfig(
            dp1v2::InverseMedianMode::FixedK3,
            1,
            dp1v2::InverseMedianOutputMode::ClipToInputRange),
        size,
        CV_8U);

    const dp1v2::InverseMedianResult* result = nullptr;
    for (const cv::Mat& frame : stream) {
        result = &filter.processFrame(frame);
    }

    ASSERT_NE(result, nullptr);
    ASSERT_EQ(result->status, dp1v2::InverseMedianStatus::Valid);
    ASSERT_NE(result->residual, nullptr);
    ASSERT_NE(result->converted_residual, nullptr);
    EXPECT_EQ(result->converted_residual->type(), CV_8UC1);
    EXPECT_EQ(result->converted_residual_view.pixel_format, dp1v2::InverseMedianPixelFormat::U8);
    EXPECT_EQ(result->converted_residual_view.range_policy, dp1v2::InverseMedianRangePolicy::ClippedToInputRange);

    const cv::Mat expected = makeU8Mat(2, 1, {0, 40});
    expectMatEqual(*result->converted_residual, expected);
}

TEST(InverseMedianFilter, ProcessFrame_WhenClipToInputRangeU16_ClampsNegativeAndKeepsPositiveResidual)
{
    const cv::Size size(2, 1);
    std::vector<cv::Mat> stream{
        makeU16Mat(2, 1, {1000, 1000}),
        makeU16Mat(2, 1, {1000, 1000}),
        makeU16Mat(2, 1, {1000, 1000}),
        makeU16Mat(2, 1, {500, 1400}),
    };

    dp1v2::InverseMedianFilter filter(
        makeConfig(
            dp1v2::InverseMedianMode::FixedK3,
            1,
            dp1v2::InverseMedianOutputMode::ClipToInputRange),
        size,
        CV_16U);

    const dp1v2::InverseMedianResult* result = nullptr;
    for (const cv::Mat& frame : stream) {
        result = &filter.processFrame(frame);
    }

    ASSERT_NE(result, nullptr);
    ASSERT_EQ(result->status, dp1v2::InverseMedianStatus::Valid);
    ASSERT_NE(result->residual, nullptr);
    ASSERT_NE(result->converted_residual, nullptr);
    EXPECT_EQ(result->residual->type(), CV_32SC1);
    EXPECT_EQ(result->converted_residual->type(), CV_16UC1);
    EXPECT_EQ(result->converted_residual_view.pixel_format, dp1v2::InverseMedianPixelFormat::U16);
    EXPECT_EQ(result->converted_residual_view.range_policy, dp1v2::InverseMedianRangePolicy::ClippedToInputRange);

    const cv::Mat expected = makeU16Mat(2, 1, {0, 400});
    expectMatEqual(*result->converted_residual, expected);
}

TEST(InverseMedianFilter, UpdateStride_WhenValueChanges_ResetsWarmupAndKeepsFrameIndexMonotonic)
{
    const cv::Size size(1, 1);
    dp1v2::InverseMedianFilter filter(makeConfig(dp1v2::InverseMedianMode::FixedK3, 1), size, CV_8U);

    filter.processFrame(makeU8Mat(1, 1, {10}));
    filter.processFrame(makeU8Mat(1, 1, {20}));
    const dp1v2::InverseMedianResult& valid = filter.processFrame(makeU8Mat(1, 1, {30}));
    ASSERT_EQ(valid.status, dp1v2::InverseMedianStatus::Valid);

    filter.updateStride(2);
    const dp1v2::InverseMedianResult& after_update = filter.processFrame(makeU8Mat(1, 1, {40}));

    EXPECT_EQ(after_update.status, dp1v2::InverseMedianStatus::WarmingUp);
    EXPECT_EQ(after_update.frame_index, 3U);
    EXPECT_FALSE(after_update.median_updated);
    EXPECT_EQ(after_update.residual, nullptr);
}

TEST(InverseMedianFilter, ResetIfRouteChanged_WhenBinningRouteChanges_ResetsHistory)
{
    const cv::Size size(1, 1);
    dp1v2::InverseMedianInputRoute route = makeU8Route(size);
    dp1v2::InverseMedianFilter filter(makeConfig(dp1v2::InverseMedianMode::FixedK3, 1));
    filter.reset(route);

    filter.processFrame(makeU8Mat(1, 1, {10}));
    filter.processFrame(makeU8Mat(1, 1, {20}));
    const dp1v2::InverseMedianResult& valid = filter.processFrame(makeU8Mat(1, 1, {30}));
    ASSERT_EQ(valid.status, dp1v2::InverseMedianStatus::Valid);

    dp1v2::InverseMedianInputRoute changed_route = route;
    changed_route.binning_factor = 2;

    EXPECT_TRUE(filter.requiresReset(changed_route));
    filter.resetIfRouteChanged(changed_route);
    const dp1v2::InverseMedianResult& after_reset = filter.processFrame(makeU8Mat(1, 1, {40}));

    EXPECT_EQ(after_reset.status, dp1v2::InverseMedianStatus::WarmingUp);
    EXPECT_EQ(after_reset.residual, nullptr);
}

TEST(InverseMedianFilter, ProcessFrame_WhenLifecycleOrInputIsInvalid_Throws)
{
    const cv::Size size(2, 2);
    dp1v2::InverseMedianFilter filter(makeConfig(dp1v2::InverseMedianMode::FixedK3, 1));

    EXPECT_THROW(filter.processFrame(makeU8Mat(2, 2, {1, 2, 3, 4})), std::logic_error);
    EXPECT_THROW(dp1v2::InverseMedianFilter(makeConfig(dp1v2::InverseMedianMode::FixedK3, 0)), std::invalid_argument);

    filter.reset(size, CV_8U);
    EXPECT_THROW(filter.processFrame(cv::Mat()), std::invalid_argument);
    EXPECT_THROW(filter.processFrame(makeU8Mat(1, 2, {1, 2})), std::invalid_argument);
    EXPECT_THROW(filter.processFrame(makeU16Mat(2, 2, {1, 2, 3, 4})), std::invalid_argument);

    dp1v2::InverseMedianInputRoute invalid_route = makeU8Route(size);
    invalid_route.binning_factor = 0;
    EXPECT_THROW(filter.reset(invalid_route), std::invalid_argument);
}

TEST(InverseMedianFilter, ProcessFrame_WhenFixedK3Stride2_ReachesFirstValidResidualAtCorrectedBudget)
{
    const cv::Size size(1, 1);
    std::vector<cv::Mat> stream{
        makeU8Mat(1, 1, {10}),
        makeU8Mat(1, 1, {100}),
        makeU8Mat(1, 1, {30}),
        makeU8Mat(1, 1, {101}),
        makeU8Mat(1, 1, {20}),
    };

    dp1v2::InverseMedianFilter filter(makeConfig(dp1v2::InverseMedianMode::FixedK3, 2), size, CV_8U);
    const dp1v2::InverseMedianResult* result = nullptr;
    for (const cv::Mat& frame : stream) {
        result = &filter.processFrame(frame);
    }

    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->frame_index, 4U);
    EXPECT_EQ(result->status, dp1v2::InverseMedianStatus::Valid);
    EXPECT_TRUE(result->median_updated);
    ASSERT_NE(result->residual, nullptr);
}
