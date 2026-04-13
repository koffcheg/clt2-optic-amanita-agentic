#include <iostream>
#include <string_view>

#include <opencv2/core.hpp>

#include "dp1v2/frame_packet.hpp"

int main()
{
    const cv::Mat mono16_frame(4, 4, CV_16UC1, cv::Scalar(1024));

    dp1v2::FrameHeaderHint success_hint{};
    success_hint.bit_depth = 12;
    success_hint.bytes_per_pixel = 2;
    success_hint.frame_id = 1;
    success_hint.camera_id = 0;

    const auto success = dp1v2::make_frame_packet(mono16_frame, success_hint, std::chrono::steady_clock::now());
    if (!success.ok()) {
        std::cerr << "datapro1_v2 ingest contract smoke failed, error="
                  << dp1v2::frame_packet_error_to_cstr(success.error) << std::endl;
        return 1;
    }

    dp1v2::FrameHeaderHint mismatch_hint{};
    mismatch_hint.bit_depth = 8;
    const auto mismatch = dp1v2::make_frame_packet(mono16_frame, mismatch_hint, std::chrono::steady_clock::now());
    if (mismatch.error != dp1v2::FramePacketBuildError::HeaderMatConflict) {
        std::cerr << "datapro1_v2 ingest contract smoke failed, error="
                  << dp1v2::frame_packet_error_to_cstr(mismatch.error) << std::endl;
        return 1;
    }

    const cv::Mat empty_frame;
    const auto empty = dp1v2::make_frame_packet(empty_frame, {}, std::chrono::steady_clock::now());
    if (empty.error != dp1v2::FramePacketBuildError::EmptyFrame) {
        std::cerr << "datapro1_v2 ingest contract smoke failed, error="
                  << dp1v2::frame_packet_error_to_cstr(empty.error) << std::endl;
        return 1;
    }

    if (std::string_view(dp1v2::frame_packet_error_to_cstr(dp1v2::FramePacketBuildError::HeaderMatConflict)) != "header_mat_conflict") {
        std::cerr << "datapro1_v2 ingest contract smoke failed, error="
                  << dp1v2::frame_packet_error_to_cstr(dp1v2::FramePacketBuildError::HeaderMatConflict) << std::endl;
        return 1;
    }

    if (std::string_view(dp1v2::frame_packet_error_to_cstr(dp1v2::FramePacketBuildError::None)) != "none") {
        std::cerr << "datapro1_v2 ingest contract smoke failed, error="
                  << dp1v2::frame_packet_error_to_cstr(dp1v2::FramePacketBuildError::None) << std::endl;
        return 1;
    }

    std::cout << "datapro1_v2 ingest contract ready, explicit status API enabled" << std::endl;
    return 0;
}
