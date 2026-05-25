#pragma once

#include <cstdint>
#include <string>

#include <opencv2/videoio.hpp>

#include "dp1v2/source/source.hpp"

namespace dp1v2 {

class UriFileFrameSource final : public IFrameSource {
public:
    explicit UriFileFrameSource(std::string link, int camera_id = -1);

    [[nodiscard]] bool is_open() const;
    SourceReadResult read_next() override;

private:
    enum class SourceKind {
        StillImage,
        ImageSequence,
        Video,
    };

    std::string link_;
    int camera_id_ = -1;
    SourceKind source_kind_ = SourceKind::Video;
    cv::VideoCapture capture_;
    std::uint64_t next_frame_id_ = 0;
};

} // namespace dp1v2
