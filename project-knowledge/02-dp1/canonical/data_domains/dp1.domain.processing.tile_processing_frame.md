---
id: dp1.domain.processing.tile_processing_frame
title: "Локальний processing frame tile у Processing домені DP1"
tags: [dp1, canonical, data-domain, processing, structure, tile, memory]
kind: data-domain-card
source_role: canonical
source:
  file: "project-knowledge/02-dp1/canonical/data_domains/dp1.domain.processing.tile_processing_frame.md"
status: "draft"
---

## Definition

`TileProcessingFrame` є tile-local Processing-domain structure для residual,
enhanced frame або detector response у межах одного tile.

Для `prep.variant = "tiles"` це processing payload route. Full-frame
`ProcessingFrame` належить іншим prep variants або explicit stage specs.

## Assumptions

- Payload `image` зазвичай посилається на `TileContext.processing_buffer_a` або
  `TileContext.processing_buffer_b`.
- Route може бути `F32`, `S16`, `S32` або explicit compatibility route,
  визначений stage spec і configuration `C`.
- `processing_domain` має явно розрізняти `RadiometricResidual`,
  `RadiometricCorrected`, `EnhancedFrame` і `DetectorResponse`.
- Один DP1 instance стартує з фіксованим `U8` або `U16` input route.

## Theorem / Contract

Рекомендована C++ форма:

```cpp
struct TileProcessingFrame {
    std::uint64_t frame_id = 0;
    int tile_id = -1;
    cv::Mat image;
    PixelFormat pixel_format = PixelFormat::F32;
    PixelRange value_range;
    ProcessingDomain processing_domain = ProcessingDomain::RadiometricResidual;
    RangePolicy range_policy = RangePolicy::NormalizedFloat;
    cv::Rect valid_area;
    cv::Point origin_px{0, 0};
    CoordinateSpace coordinate_space = CoordinateSpace::TileLocal;
};
```

## Поля

| Поле | Тип | Навіщо | Для яких обчислень | Пам'ять |
|---|---|---|---|---|
| `frame_id` | `std::uint64_t` | Зв'язок із source frame. | Validation, profiling, merge trace. | 8 B |
| `tile_id` | `int` | Зв'язок із source tile. | Tile profiling, duplicate handling. | 4 B |
| `image` | `cv::Mat` | Processing pixels для одного tile. | Radiometric output, enhancement, matched filtering, thresholding. | header ~96 B; payload у `TileContext` |
| `pixel_format` | `PixelFormat` | Carrier processing values. | Allowed conversions і algorithm route validation. | 4 B |
| `value_range` | `PixelRange` | Numeric range після conversion/residual. | Thresholds, clipping/scaling, photometry if selected. | ~32 B |
| `processing_domain` | `ProcessingDomain` | Семантика payload. | Перевірка, що stage читає residual/enhanced/response. | 4 B |
| `range_policy` | `RangePolicy` | Правила інтерпретації значень. | Signed residual, normalized float, clipped output. | 4 B |
| `valid_area` | `cv::Rect` | Border-safe output area. | Crop перед candidate/segment/measurement acceptance. | 16 B |
| `origin_px` | `cv::Point` | Local-to-global offset. | Geometry transform. | 8 B |
| `coordinate_space` | `CoordinateSpace` | Tile-local або frame-global coordinates. | Merge correctness. | 4 B |

## Пам'ять

`TileProcessingFrame` не повинен володіти full-frame payload. Для tile
`256x256`:

- `F32`: ~256 KB;
- два `F32` ping-pong buffers у `TileContext`: ~512 KB;
- `S16`: ~128 KB;
- `S32`: ~256 KB.

Пам'ять виділяється per worker у `TileContext`, а не per tile. Stage має
використовувати `cv::Mat::create()` і reuse.

## Етапи

- `radiometric_correction` створює `TileProcessingFrame` з `TileRawView`.
- `enhancement` читає і видає `TileProcessingFrame`.
- `matched_filtering` читає і видає detector-response `TileProcessingFrame`.
- `candidate_extraction` читає `TileProcessingFrame` і видає `TileBinaryMask`
  та `Candidate[]`.
- `measurement` може читати `TileProcessingFrame` як photometry source, якщо
  route це дозволяє.

## Обмеження

- Не є `BinaryMask`.
- Не містить candidates, segments або measurements.
- Full-frame allocation не належить route `prep.variant = "tiles"`, якщо окрема stage spec не дозволяє її явно.
- `pixel_format` і `range_policy` мають відповідати stage spec.

## Failure cases

- Detector response записується як raw frame.
- Signed residual silently clipped без `RangePolicy`.
- Tile-local processing output використовується без `valid_area` crop.

## Typical misuse

- Використовувати один shared mutable processing buffer між workers.
- Створювати full-frame `F32` buffer у route `prep.variant = "tiles"` без explicit stage spec.

## Open questions

- Які stages можуть працювати in-place, а які потребують ping-pong buffer.

## Connections

- belongs_to: dp1.domain.processing
- derived_from: dp1.domain.raw.tile_raw_view
- uses: dp1.domain.pixel_format
- constrained_by: dp1.domain.memory_ownership
- constrained_by: dp1.domain.coordinates
- constrained_by: dp1.domain.threshold
- owned_by: dp1.domain.runtime.tile_context
- may_produce: dp1.domain.mask.tile_binary_mask
