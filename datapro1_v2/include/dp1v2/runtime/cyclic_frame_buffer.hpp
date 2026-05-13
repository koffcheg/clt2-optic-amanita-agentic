#pragma once

#include <string>
#include <vector>

#include <opencv2/core.hpp>

namespace dp1v2 {

struct CyclicFrameBufferMetadata {
    cv::Size frame_size;
    int frame_type = -1;
    int input_bit_depth = 0;
    double range_min = 0.0;
    double range_max = 0.0;
    std::string pixel_format;
    std::string range_policy;
};

struct CyclicFrameBuffer {
    int capacity = 0;
    std::vector<cv::Mat> slots;
    int next_slot = 0;
    int filled_count = 0;
    CyclicFrameBufferMetadata metadata;

    void resetStorage(int new_capacity, const cv::Size& frame_size, int frame_type);
    void resetStorage(int new_capacity, const CyclicFrameBufferMetadata& new_metadata);
    void resetState() noexcept;
    void clear() noexcept;
    bool full() const noexcept;
    void store(const cv::Mat& frame);
};

}  // namespace dp1v2
