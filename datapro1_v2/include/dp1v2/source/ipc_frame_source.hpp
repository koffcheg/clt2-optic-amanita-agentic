#pragma once

#include <chrono>
#include <cstddef>
#include <string>

#include "dp1v2/config/defaults.hpp"
#include "dp1v2/source/source.hpp"

namespace dp1v2 {

class IpcFrameSource final : public IFrameSource {
public:
    IpcFrameSource(int cam_index, std::chrono::milliseconds receive_timeout);
    ~IpcFrameSource() override;

    IpcFrameSource(const IpcFrameSource &) = delete;
    IpcFrameSource &operator=(const IpcFrameSource &) = delete;

    SourceReadResult read_next() override;

private:
    bool open_shared_memory();
    bool open_queue();
    bool map_shared_memory(std::size_t size);
    void close_handles();

    int cam_index_ = -1;
    std::chrono::milliseconds receive_timeout_{kDefaultIpcReceiveTimeoutMs};
    std::string shmem_name_;
    std::string queue_name_;
    int shmem_fd_ = -1;
    int queue_fd_ = -1;
    std::size_t queue_msg_size_ = 0;
    void *mapped_addr_ = nullptr;
    std::size_t mapped_size_ = 0;
};

} // namespace dp1v2
