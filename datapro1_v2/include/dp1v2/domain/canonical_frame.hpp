#pragma once

#include <cstdint>
#include <optional>
#include <string>

#include <opencv2/core.hpp>

#include "dp1v2/domain/geometry.hpp"
#include "dp1v2/domain/pixel.hpp"
#include "dp1v2/domain/time.hpp"

namespace dp1v2 {

enum class BinningMode {
    None,
    Sum,
    Average,
};

enum class NormalizationSource {
    None,
    Camera,
    CameraProSim,
    Stage0,
    External,
};

enum class CanonicalPayloadOwnership {
    BorrowedReadOnly,
    OwnedCopy,
    OwnedConverted,
    OwnedBinned,
};

struct NormalizationProvenance {
    NormalizationSource source = NormalizationSource::None;
    bool copied = false;
    bool converted = false;
    bool binned = false;
    int bin_factor_x = 1;
    int bin_factor_y = 1;
    BinningMode binning_mode = BinningMode::None;
    PixelFormat source_pixel_format = PixelFormat::U16;
    InputBitDepth source_bit_depth = InputBitDepth::Bit16;
    PixelRange source_pixel_range;
};

struct CanonicalFrame {
    std::uint64_t frame_id = 0;
    int camera_id = -1;
    std::string source_id;

    cv::Mat image;
    CanonicalPayloadOwnership image_ownership = CanonicalPayloadOwnership::BorrowedReadOnly;

    PixelFormat pixel_format = PixelFormat::U16;
    InputBitDepth bit_depth = InputBitDepth::Bit16;
    PixelRange pixel_range;
    FrameGeometry geometry;
    CoordinateSpace coordinate_space = CoordinateSpace::FrameGlobal;

    TimestampRef ingest_time;
    std::optional<TimestampRef> acquisition_time;

    std::string parent_artifact_id;
    NormalizationProvenance normalization;
};

} // namespace dp1v2
