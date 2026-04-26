#include "dp1v2/source/ipc_frame_source.hpp"

#include <cerrno>
#include <cstring>
#include <ctime>
#include <stdexcept>
#include <vector>

#include <fcntl.h>
#include <mqueue.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include "camera_frame.hpp"
#include "m_cfg_if.h"
#include "m_ipc_def.h"

namespace {

constexpr int kQueueOpenFlags = O_NONBLOCK | O_RDONLY;
constexpr long long kNanosecondsPerSecond = 1'000'000'000LL;
constexpr long kMaxNanosecondsRemainder = static_cast<long>(kNanosecondsPerSecond - 1);
constexpr int kSupportedIpcBytesPerPixel = 2;
constexpr int kSupportedIpcBitDepth = 16;

timespec make_abs_timeout(const std::chrono::milliseconds timeout) {
    timespec time{};
    clock_gettime(CLOCK_REALTIME, &time);

    const auto timeout_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(timeout);
    time.tv_sec += static_cast<time_t>(timeout_ns.count() / kNanosecondsPerSecond);
    time.tv_nsec += static_cast<long>(timeout_ns.count() % kNanosecondsPerSecond);
    if (time.tv_nsec > kMaxNanosecondsRemainder) {
        time.tv_nsec -= static_cast<long>(kNanosecondsPerSecond);
        ++time.tv_sec;
    }
    return time;
}

dp1v2::SourceReadResult failed_result(const char *reason) {
    return dp1v2::SourceReadResult{.status = dp1v2::SourceReadStatus::Failed, .reason = reason};
}

} // namespace

namespace dp1v2 {

IpcFrameSource::IpcFrameSource(const int cam_index, const std::chrono::milliseconds receive_timeout)
    : cam_index_(cam_index),
      receive_timeout_(receive_timeout) {
    ipc_name_cfg ipc_names(cam_index_);
    shmem_name_ = ipc_names.get_shmem_obj_name();
    queue_name_ = ipc_names.get_queue_obj_name();
}

IpcFrameSource::~IpcFrameSource() {
    close_handles();
}

bool IpcFrameSource::open_shared_memory() {
    if (shmem_fd_ != -1) {
        return true;
    }

    shmem_fd_ = shm_open(shmem_name_.c_str(), O_RDONLY, S_IRUSR | S_IWUSR);
    return shmem_fd_ != -1;
}

bool IpcFrameSource::open_queue() {
    if (queue_fd_ != -1) {
        return true;
    }

    queue_fd_ = static_cast<int>(mq_open(queue_name_.c_str(), kQueueOpenFlags, S_IRUSR | S_IWUSR, nullptr));
    if (queue_fd_ == -1) {
        return false;
    }

    mq_attr attr{};
    if (mq_getattr(static_cast<mqd_t>(queue_fd_), &attr) == -1) {
        mq_close(static_cast<mqd_t>(queue_fd_));
        queue_fd_ = -1;
        return false;
    }

    queue_msg_size_ = static_cast<std::size_t>(attr.mq_msgsize);
    attr.mq_flags &= ~O_NONBLOCK;
    if (mq_setattr(static_cast<mqd_t>(queue_fd_), &attr, nullptr) == -1) {
        mq_close(static_cast<mqd_t>(queue_fd_));
        queue_fd_ = -1;
        queue_msg_size_ = 0;
        return false;
    }

    return true;
}

bool IpcFrameSource::map_shared_memory(const std::size_t size) {
    if (mapped_addr_ && mapped_size_ == size) {
        return true;
    }

    if (mapped_addr_) {
        munmap(mapped_addr_, mapped_size_);
        mapped_addr_ = nullptr;
        mapped_size_ = 0;
    }

    mapped_addr_ = mmap(nullptr, size, PROT_READ, MAP_SHARED, shmem_fd_, 0);
    if (mapped_addr_ == MAP_FAILED) {
        mapped_addr_ = nullptr;
        return false;
    }

    mapped_size_ = size;
    return true;
}

void IpcFrameSource::close_handles() {
    if (mapped_addr_) {
        munmap(mapped_addr_, mapped_size_);
        mapped_addr_ = nullptr;
        mapped_size_ = 0;
    }
    if (queue_fd_ != -1) {
        mq_close(static_cast<mqd_t>(queue_fd_));
        queue_fd_ = -1;
    }
    if (shmem_fd_ != -1) {
        close(shmem_fd_);
        shmem_fd_ = -1;
    }
}

SourceReadResult IpcFrameSource::read_next() {
    if (!open_shared_memory()) {
        return errno == ENOENT
            ? SourceReadResult{.status = SourceReadStatus::Timeout, .reason = "ipc_shared_memory_not_available"}
            : failed_result("ipc_shared_memory_open_failed");
    }
    if (!open_queue()) {
        return errno == ENOENT
            ? SourceReadResult{.status = SourceReadStatus::Timeout, .reason = "ipc_queue_not_available"}
            : failed_result("ipc_queue_open_failed");
    }

    std::vector<char> queue_message(queue_msg_size_);
    auto timeout = make_abs_timeout(receive_timeout_);
    const ssize_t read_size = mq_timedreceive(
        static_cast<mqd_t>(queue_fd_),
        queue_message.data(),
        queue_message.size(),
        nullptr,
        &timeout);

    if (read_size == -1) {
        return errno == ETIMEDOUT || errno == EAGAIN
            ? SourceReadResult{.status = SourceReadStatus::Timeout, .reason = "ipc_queue_receive_timeout"}
            : failed_result("ipc_queue_receive_failed");
    }
    if (static_cast<std::size_t>(read_size) != sizeof(ipc_next_frame_notify)) {
        return failed_result("ipc_queue_message_size_mismatch");
    }

    const auto *notify = reinterpret_cast<const ipc_next_frame_notify *>(queue_message.data());
    if (!map_shared_memory(notify->sh_mem_size)) {
        return failed_result("ipc_shared_memory_map_failed");
    }
    if (notify->frame_ptr_offset > mapped_size_ || notify->frame_size > mapped_size_ - notify->frame_ptr_offset) {
        return failed_result("ipc_frame_bounds_invalid");
    }
    if (notify->frame_size < sizeof(cam_pro::FrameHeader)) {
        return failed_result("ipc_frame_size_invalid");
    }

    const auto *frame_addr = static_cast<const unsigned char *>(mapped_addr_) + notify->frame_ptr_offset;
    const auto *campro_frame = reinterpret_cast<const cam_pro::Frame *>(frame_addr);
    if (campro_frame->header.width <= 0 || campro_frame->header.height <= 0) {
        return failed_result("ipc_frame_dimensions_invalid");
    }
    if (campro_frame->header.bytesPerPixel != kSupportedIpcBytesPerPixel ||
        campro_frame->header.bitDepth != kSupportedIpcBitDepth) {
        return failed_result("ipc_frame_format_unsupported");
    }
    const auto expected_frame_size = cam_pro::Frame::size(
        campro_frame->header.width,
        campro_frame->header.height,
        campro_frame->header.bytesPerPixel);
    if (expected_frame_size > notify->frame_size) {
        return failed_result("ipc_frame_size_mismatch");
    }
    const auto *frame_data = frame_addr + cam_pro::Frame::getDataOffset();
    cv::Mat frame_view(campro_frame->header.height, campro_frame->header.width, CV_16UC1, const_cast<unsigned char *>(frame_data));
    if (frame_view.empty()) {
        return failed_result("ipc_frame_empty");
    }

    RawFrameEnvelope envelope{};
    envelope.frame = frame_view.clone();
    envelope.header_hint.bit_depth = campro_frame->header.bitDepth;
    envelope.header_hint.bytes_per_pixel = campro_frame->header.bytesPerPixel;
    envelope.header_hint.frame_id = static_cast<std::uint64_t>(notify->frame_index);
    envelope.header_hint.camera_id = cam_index_;
    envelope.header_hint.exposure_start = campro_frame->header.exposureStart;
    envelope.header_hint.exposure_length_sec = campro_frame->header.exposureLength;
    envelope.received_steady_ts = notify->cam_pro_time;

    return SourceReadResult{
        .status = SourceReadStatus::FrameReady,
        .envelope = std::move(envelope),
        .reason = "",
    };
}

} // namespace dp1v2
