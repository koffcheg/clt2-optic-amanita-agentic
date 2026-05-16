#pragma once

#include <cstdint>
#include <string>

#include <opencv2/videoio.hpp>

#include "dp1v2/source/source.hpp"

namespace dp1v2 {

class UriFileFrameSource final : public IFrameSource {
public:
    explicit UriFileFrameSource(std::string link);

    [[nodiscard]] bool is_open() const;
    SourceReadResult read_next() override;

private:
    std::string link_;
    cv::VideoCapture capture_;
    std::uint64_t next_frame_id_ = 0;
};

} // namespace dp1v2
