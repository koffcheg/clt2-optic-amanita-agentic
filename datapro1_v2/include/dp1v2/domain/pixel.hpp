#pragma once

namespace dp1v2 {

enum class PixelFormat {
    U8,
    U16,
    F32,
    MaskU8,
    S16,
    S32,
};

enum class InputBitDepth {
    Bit8 = 8,
    Bit10 = 10,
    Bit12 = 12,
    Bit14 = 14,
    Bit16 = 16,
};

struct PixelRange {
    double min_value = 0.0;
    double max_value = 0.0;
    double black_level = 0.0;
    double saturation_level = 0.0;
};

enum class ProcessingDomain {
    RawIntensity,
    RadiometricResidual,
    RadiometricCorrected,
    EnhancedFrame,
    DetectorResponse,
};

enum class RangePolicy {
    Unknown,
    RawSensorRange,
    SignedResidual,
    ClippedToInputRange,
    NormalizedFloat,
    DetectorResponse,
};

} // namespace dp1v2
