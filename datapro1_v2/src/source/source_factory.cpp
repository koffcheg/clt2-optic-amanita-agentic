#include "dp1v2/source/source_factory.hpp"

#include <chrono>
#include <utility>

#include "dp1v2/source/ipc_frame_source.hpp"
#include "dp1v2/source/uri_file_source.hpp"

namespace {

class UnsupportedFrameSource final : public dp1v2::IFrameSource {
public:
    explicit UnsupportedFrameSource(const char *reason)
        : reason_(reason) {}

    dp1v2::SourceReadResult read_next() override {
        return dp1v2::SourceReadResult{.status = dp1v2::SourceReadStatus::Failed, .reason = reason_};
    }

private:
    const char *reason_ = "unsupported_source";
};

bool is_link_based_uri_source(const std::string &source) {
    return source == "ipcam" || source == "videofile" || source == "imagefile";
}

} // namespace

namespace dp1v2 {

std::unique_ptr<IFrameSource> create_frame_source(const SourceConfig &config, const int cam_index) {
    if (config.kind == SourceKind::UriFile && is_link_based_uri_source(config.source)) {
        return std::make_unique<UriFileFrameSource>(config.link);
    }

    if (config.kind == SourceKind::UriFile && config.source == "webcam") {
        return std::make_unique<UnsupportedFrameSource>("webcam_source_not_implemented");
    }

    if (config.kind == SourceKind::CamproIpc) {
        return std::make_unique<IpcFrameSource>(cam_index, std::chrono::milliseconds(config.ipc_receive_timeout_ms));
    }

    return std::make_unique<UnsupportedFrameSource>("unsupported_source");
}

} // namespace dp1v2
