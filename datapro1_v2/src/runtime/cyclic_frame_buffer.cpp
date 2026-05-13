#include "dp1v2/runtime/cyclic_frame_buffer.hpp"

#include <algorithm>
#include <stdexcept>

namespace dp1v2 {
namespace {

void validateMetadata(const CyclicFrameBufferMetadata& metadata)
{
    if (metadata.frame_size.width <= 0 || metadata.frame_size.height <= 0) {
        throw std::invalid_argument("cyclic frame buffer frame size must be positive");
    }
    if (metadata.frame_type < 0) {
        throw std::invalid_argument("cyclic frame buffer frame type must be initialized");
    }
    if (metadata.input_bit_depth < 0) {
        throw std::invalid_argument("cyclic frame buffer input bit depth must be >= 0");
    }
    if (metadata.range_min != metadata.range_max && metadata.range_max <= metadata.range_min) {
        throw std::invalid_argument("cyclic frame buffer range metadata must be ordered");
    }
}

}  // namespace

void CyclicFrameBuffer::resetStorage(int new_capacity, const cv::Size& frame_size, int frame_type)
{
    CyclicFrameBufferMetadata new_metadata{};
    new_metadata.frame_size = frame_size;
    new_metadata.frame_type = frame_type;
    resetStorage(new_capacity, new_metadata);
}

void CyclicFrameBuffer::resetStorage(int new_capacity, const CyclicFrameBufferMetadata& new_metadata)
{
    if (new_capacity < 1) {
        throw std::invalid_argument("cyclic frame buffer capacity must be >= 1");
    }
    validateMetadata(new_metadata);

    capacity = new_capacity;
    metadata = new_metadata;
    slots.resize(static_cast<std::size_t>(capacity));
    for (cv::Mat& slot : slots) {
        slot.create(metadata.frame_size, metadata.frame_type);
    }
    resetState();
}

void CyclicFrameBuffer::resetState() noexcept
{
    next_slot = 0;
    filled_count = 0;
}

void CyclicFrameBuffer::clear() noexcept
{
    capacity = 0;
    slots.clear();
    metadata = {};
    resetState();
}

bool CyclicFrameBuffer::full() const noexcept
{
    return filled_count == capacity && capacity > 0;
}

void CyclicFrameBuffer::store(const cv::Mat& frame)
{
    if (capacity < 1 || slots.empty()) {
        throw std::logic_error("cyclic frame buffer storage is not initialized");
    }
    if (frame.empty()) {
        throw std::invalid_argument("cyclic frame buffer input frame must not be empty");
    }
    if (frame.size() != metadata.frame_size) {
        throw std::invalid_argument("cyclic frame buffer input frame size does not match storage metadata");
    }
    if (frame.type() != metadata.frame_type) {
        throw std::invalid_argument("cyclic frame buffer input frame type does not match storage metadata");
    }

    frame.copyTo(slots[static_cast<std::size_t>(next_slot)]);
    next_slot = (next_slot + 1) % capacity;
    filled_count = std::min(filled_count + 1, capacity);
}

}  // namespace dp1v2
