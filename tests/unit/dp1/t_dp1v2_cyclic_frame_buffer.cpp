#include <gtest/gtest.h>

#include <stdexcept>

#include <opencv2/core.hpp>

#include "dp1v2/runtime/cyclic_frame_buffer.hpp"

namespace {

cv::Mat makeU8Frame(int width, int height, std::uint8_t value)
{
    return cv::Mat(height, width, CV_8UC1, cv::Scalar(value));
}

dp1v2::CyclicFrameBufferMetadata makeMetadata(cv::Size frame_size, int frame_type)
{
    dp1v2::CyclicFrameBufferMetadata metadata{};
    metadata.frame_size = frame_size;
    metadata.frame_type = frame_type;
    metadata.input_bit_depth = 8;
    metadata.range_min = 0.0;
    metadata.range_max = 255.0;
    metadata.pixel_format = "U8";
    metadata.range_policy = "RawSensorRange";
    return metadata;
}

void expectMatFilledWith(const cv::Mat& mat, std::uint8_t expected)
{
    ASSERT_EQ(mat.type(), CV_8UC1);
    for (int y = 0; y < mat.rows; ++y) {
        for (int x = 0; x < mat.cols; ++x) {
            EXPECT_EQ(mat.at<std::uint8_t>(y, x), expected);
        }
    }
}

}  // namespace

TEST(CyclicFrameBuffer, ResetStorage_WhenValidMetadata_PreallocatesSlotsAndResetsState)
{
    dp1v2::CyclicFrameBuffer buffer;
    const cv::Size frame_size(3, 2);

    buffer.resetStorage(3, makeMetadata(frame_size, CV_8UC1));

    EXPECT_EQ(buffer.capacity, 3);
    ASSERT_EQ(buffer.slots.size(), 3U);
    EXPECT_EQ(buffer.next_slot, 0);
    EXPECT_EQ(buffer.filled_count, 0);
    EXPECT_FALSE(buffer.full());
    EXPECT_EQ(buffer.metadata.frame_size, frame_size);
    EXPECT_EQ(buffer.metadata.frame_type, CV_8UC1);

    for (const cv::Mat& slot : buffer.slots) {
        EXPECT_EQ(slot.size(), frame_size);
        EXPECT_EQ(slot.type(), CV_8UC1);
    }
}

TEST(CyclicFrameBuffer, ResetStorage_WhenInvalidCapacity_Throws)
{
    dp1v2::CyclicFrameBuffer buffer;

    EXPECT_THROW(buffer.resetStorage(0, makeMetadata(cv::Size(2, 2), CV_8UC1)), std::invalid_argument);
}

TEST(CyclicFrameBuffer, ResetStorage_WhenInvalidMetadata_Throws)
{
    dp1v2::CyclicFrameBuffer buffer;

    dp1v2::CyclicFrameBufferMetadata bad_size = makeMetadata(cv::Size(0, 2), CV_8UC1);
    EXPECT_THROW(buffer.resetStorage(1, bad_size), std::invalid_argument);

    dp1v2::CyclicFrameBufferMetadata bad_type = makeMetadata(cv::Size(2, 2), -1);
    EXPECT_THROW(buffer.resetStorage(1, bad_type), std::invalid_argument);

    dp1v2::CyclicFrameBufferMetadata bad_bit_depth = makeMetadata(cv::Size(2, 2), CV_8UC1);
    bad_bit_depth.input_bit_depth = -1;
    EXPECT_THROW(buffer.resetStorage(1, bad_bit_depth), std::invalid_argument);

    dp1v2::CyclicFrameBufferMetadata bad_range = makeMetadata(cv::Size(2, 2), CV_8UC1);
    bad_range.range_min = 10.0;
    bad_range.range_max = 5.0;
    EXPECT_THROW(buffer.resetStorage(1, bad_range), std::invalid_argument);
}

TEST(CyclicFrameBuffer, Store_WhenFrameMatchesMetadata_CopiesIntoNextSlot)
{
    dp1v2::CyclicFrameBuffer buffer;
    buffer.resetStorage(2, makeMetadata(cv::Size(2, 2), CV_8UC1));

    cv::Mat frame = makeU8Frame(2, 2, 17);
    buffer.store(frame);
    frame.setTo(cv::Scalar(99));

    EXPECT_EQ(buffer.next_slot, 1);
    EXPECT_EQ(buffer.filled_count, 1);
    EXPECT_FALSE(buffer.full());
    expectMatFilledWith(buffer.slots[0], 17);
}

TEST(CyclicFrameBuffer, Store_WhenCapacityWraps_OverwritesPhysicalSlotAndCapsFilledCount)
{
    dp1v2::CyclicFrameBuffer buffer;
    buffer.resetStorage(2, makeMetadata(cv::Size(2, 2), CV_8UC1));

    buffer.store(makeU8Frame(2, 2, 1));
    buffer.store(makeU8Frame(2, 2, 2));
    buffer.store(makeU8Frame(2, 2, 3));

    EXPECT_EQ(buffer.next_slot, 1);
    EXPECT_EQ(buffer.filled_count, 2);
    EXPECT_TRUE(buffer.full());
    expectMatFilledWith(buffer.slots[0], 3);
    expectMatFilledWith(buffer.slots[1], 2);
}

TEST(CyclicFrameBuffer, Store_WhenInputIsInvalid_Throws)
{
    dp1v2::CyclicFrameBuffer buffer;

    EXPECT_THROW(buffer.store(makeU8Frame(2, 2, 1)), std::logic_error);

    buffer.resetStorage(1, makeMetadata(cv::Size(2, 2), CV_8UC1));

    EXPECT_THROW(buffer.store(cv::Mat()), std::invalid_argument);
    EXPECT_THROW(buffer.store(makeU8Frame(3, 2, 1)), std::invalid_argument);
    EXPECT_THROW(buffer.store(cv::Mat(2, 2, CV_16UC1, cv::Scalar(1))), std::invalid_argument);
}

TEST(CyclicFrameBuffer, Clear_WhenCalled_ReleasesStorageAndResetsState)
{
    dp1v2::CyclicFrameBuffer buffer;
    buffer.resetStorage(2, makeMetadata(cv::Size(2, 2), CV_8UC1));
    buffer.store(makeU8Frame(2, 2, 1));

    buffer.clear();

    EXPECT_EQ(buffer.capacity, 0);
    EXPECT_TRUE(buffer.slots.empty());
    EXPECT_EQ(buffer.next_slot, 0);
    EXPECT_EQ(buffer.filled_count, 0);
    EXPECT_FALSE(buffer.full());
    EXPECT_THROW(buffer.store(makeU8Frame(2, 2, 1)), std::logic_error);
}
