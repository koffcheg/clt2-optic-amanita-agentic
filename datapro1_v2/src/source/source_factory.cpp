#include "dp1v2/source/source_factory.hpp"

#include <utility>

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

} // namespace

namespace dp1v2 {

std::unique_ptr<IFrameSource> create_frame_source(const SourceConfig &config, const int cam_index) {
    if (config.mode == FrameSourceMode::File) {
        return std::make_unique<UriFileFrameSource>(config.file.path, cam_index);
    }

    return std::make_unique<UnsupportedFrameSource>("unsupported_canonical_source_mode");
}

} // namespace dp1v2
