---
id: dp1.domain.pixel_format
title: "Формати пікселів, бітність і route-метадані DP1"
tags: [dp1, canonical, data-domain, pixel-format, bit-depth, route]
kind: data-domain-card
source_role: canonical
source:
  file: "project-knowledge/02-dp1/canonical/data_domains/dp1.domain.pixel_format.md"
status: "draft"
---

## Definition

Картка визначає canonical vocabulary форматів пікселів для DP1 data domains.

Формат пікселів є властивістю route/config, а не властивістю алгоритму. Stage spec не має зашивати 8-bit або 16-bit поведінку без явного посилання на цей домен і configuration `C`.

## Assumptions

- OpenCV type може бути storage carrier, але semantic format визначається цією карткою і відповідним data-domain contract.
- Фактична підтримка конкретного формату в коді має перевірятися через implementation artifacts.

## Theorem / Contract

Canonical DP1 має мінімально розрізняти такі формати pixel carrier:

- `U8` — unsigned 8-bit grayscale carrier, зазвичай `CV_8UC1`.
- `U16` — unsigned 16-bit grayscale carrier, зазвичай `CV_16UC1`.
- `F32` — single-channel floating processing/response carrier, зазвичай `CV_32FC1`.
- `MaskU8` — carrier для бінарної маски, зазвичай `CV_8UC1`.
- `S16` — signed 16-bit residual carrier для route, де `uint8` input формує signed residual.
- `S32` — signed 32-bit residual carrier для route, де `uint16` input формує signed residual.

Формат має бути явно заданий у data object або route-level metadata. Неявне виведення semantics тільки з `cv::Mat::type()` недостатнє для canonical contract.

Canonical DP1 має розрізняти фактичну бітність input окремо від OpenCV carrier:

- `Bit8` — 8-bit camera/input data.
- `Bit10` — 10-bit camera data, зазвичай у `U16` carrier.
- `Bit12` — 12-bit camera data, зазвичай у `U16` carrier.
- `Bit14` — 14-bit camera data, зазвичай у `U16` carrier.
- `Bit16` — 16-bit camera/input data.

Мінімальні route metadata для structures:

| Concept | Canonical fields | Навіщо |
|---|---|---|
| Pixel carrier | `pixel_format`, `input_format`, `processing_format` | Визначає storage carrier і allowed conversions. |
| Input bit depth | `bit_depth`, `input_bit_depth`, `source_bit_depth` | Визначає фактичну розрядність сенсора. |
| Pixel range | `pixel_range`, `value_range` | Визначає min/max/black/saturation values для thresholds і photometry. |
| Range policy | `range_policy` | Визначає numeric semantics після residual/conversion. |
| Processing domain | `processing_domain` | Відрізняє residual, enhanced frame і detector response. |
| Coordinate space | `coordinate_space` | Відрізняє tile-local і frame-global coordinates. |

Рекомендовані helper structures для майбутнього C++ DTO:

```cpp
enum class PixelFormat {
    U8,
    U16,
    F32,
    MaskU8,
    S16,
    S32
};

enum class InputBitDepth {
    Bit8,
    Bit10,
    Bit12,
    Bit14,
    Bit16
};

enum class CoordinateSpace {
    FrameGlobal,
    TileLocal
};

enum class ProcessingDomain {
    RawView,
    RadiometricResidual,
    RadiometricCorrected,
    EnhancedFrame,
    DetectorResponse
};

enum class RangePolicy {
    RawSensorRange,
    NormalizedFloat,
    SignedResidual,
    ClippedToInputRange,
    ScaledToInputRange
};

struct PixelRange {
    double min_value = 0.0;
    double max_value = 0.0;
    double black_level = 0.0;
    double saturation_level = 0.0;
};

struct FrameGeometry {
    int width = 0;
    int height = 0;
    cv::Point origin_px{0, 0};
};
```

## Поля службових структур

`PixelRange.min_value` задає мінімальне допустиме значення route. Для raw route
це зазвичай `0`.

`PixelRange.max_value` задає верхню межу route: `255`, `1023`, `4095`,
`16383`, `65535` або іншу межу, якщо camera/config задає calibrated range.

`PixelRange.black_level` задає рівень чорного, якщо камера або calibration
його надає. Якщо значення невідоме, route має явно задати default policy.

`PixelRange.saturation_level` задає рівень насичення для photometry, quality
flags і reject logic.

`FrameGeometry.width` і `FrameGeometry.height` задають геометрію кадру або
tile-scoped representation.

`FrameGeometry.origin_px` задає origin representation у frame-global
coordinates. Для full-frame `FramePacket` це зазвичай `(0, 0)`. Для tile-local
structures це offset `TileDesc.origin_px`.

## Розділення структур і алгоритмів

Structures є domain carriers. Algorithms є route-specific implementations.
Один DP1 instance може стартувати з фіксованим `U8` або `U16` route:

```text
FramePacket { pixel_format=U8, bit_depth=Bit8 }
  -> RadiometricU8 або інша U8-specific implementation

FramePacket { pixel_format=U16, bit_depth=Bit10|Bit12|Bit14|Bit16 }
  -> RadiometricU16 або інша U16-specific implementation
```

Жоден stage не повинен виводити повну semantics тільки з `cv::Mat::type()`.
`cv::Mat::type()` є storage fact, а не domain contract.

## Interpretation

`U8` і `U16` дозволяють запускати той самий stage route на різній бітності, якщо stage spec і config `C` явно визначають threshold/scale/range policy.

`F32` використовується для normalized або detector-response представлень, коли алгоритму потрібен scalar domain, що не є raw pixel domain.

`MaskU8` не є intensity image. Це domain-specific carrier для mask semantics.

`S16` і `S32` потрібні для signed residual routes, зокрема для stage specs, де
residual не має втрачати від'ємні значення до downstream processing.

Для `prep.variant = "tiles"` full-frame processing або mask buffers не є
частиною цього route. Вони можуть існувати в інших `prep` variants або як
окрема вимога stage spec.

## Failure cases

- Алгоритм трактує `CV_8UC1` і `CV_16UC1` однаково без scale/range policy.
- Stage spec описує threshold як абсолютне число без прив'язки до pixel format.
- Бінарна маска трактується як grayscale processing frame.
- `CV_16UC1` трактується як `Bit16`, хоча camera route задає `Bit12`.
- Signed residual route примусово приводиться до unsigned carrier без explicit
  `RangePolicy`.

## Typical misuse

- Вважати OpenCV type повним domain contract.
- Зашивати 8-bit або 16-bit поведінку в алгоритм замість route/config selection.
- Називати canonical fields `raw16`, `proc32`, `mask8`, якщо structure має
  підтримувати route metadata.

## Open questions

- Єдина canonical threshold range policy для `U8`, `U16` і `F32`.
- Чи потрібні окремі packed/color formats поза grayscale canonical route.
- Canonical default для `black_level`, якщо camera metadata його не надає.

## Connections

- constrains: dp1.domain.raw.frame_packet
- constrains: dp1.domain.raw.tile_raw_view
- constrains: dp1.domain.processing.frame
- constrains: dp1.domain.processing.tile_processing_frame
- constrains: dp1.domain.mask.binary_mask
- constrains: dp1.domain.mask.tile_binary_mask
- constrained_by: dp1.config.pipeline_configuration_c
