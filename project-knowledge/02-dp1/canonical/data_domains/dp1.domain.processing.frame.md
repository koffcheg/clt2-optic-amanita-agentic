---
id: dp1.domain.processing.frame
title: "Кадр у Processing домені DP1"
tags: [dp1, canonical, data-domain, processing, structure]
kind: data-domain-card
source_role: canonical
source:
  file: "project-knowledge/02-dp1/canonical/data_domains/dp1.domain.processing.frame.md"
status: "draft"
---

## Definition

`ProcessingFrame` є canonical structure у Processing domain для кадру або похідного від кадру представлення після одного чи кількох processing stages.

Ця structure використовується тоді, коли semantics вже відрізняються від Raw/Input `FramePacket`.

## Assumptions

- Processing representation може використовувати `U8`, `U16` або `F32` залежно від configuration `C` і stage spec.
- OpenCV carrier не визначає semantics без explicit pixel format і processing domain.
- Code-level structure має бути підтверджена окремою implementation task.

## Theorem / Contract

`ProcessingFrame` має мінімально містити або посилатися на:

- `frame_id` — ідентичність кадру, узгоджена з `FramePacket`.
- `source_frame_ref` — optional reference на source `FramePacket`.
- `image` — processing storage carrier.
- `pixel_format` — `U8`, `U16` або `F32`, defined by `dp1.domain.pixel_format`.
- `processing_domain` — semantic label із `dp1.domain.pixel_format`, наприклад
  `RadiometricResidual`, `RadiometricCorrected`, `EnhancedFrame` або
  `DetectorResponse`.
- `geometry` — width, height, coordinate origin policy.
- `range_policy` — interpretation of scalar range, якщо це потрібно для route.

`ProcessingFrame` не має містити binary mask semantics, candidates, segments або measurements.

Для `prep.variant = "tiles"` full-frame `ProcessingFrame` не є runtime payload
цього route. Tile-specific processing payload описує `TileProcessingFrame`.

Рекомендована full-frame C++ форма, якщо stage spec явно дозволяє full-frame
route:

```cpp
struct ProcessingFrame {
    std::uint64_t frame_id = 0;
    std::optional<std::uint64_t> source_frame_id;
    cv::Mat image;
    PixelFormat pixel_format = PixelFormat::F32;
    PixelRange value_range;
    ProcessingDomain processing_domain = ProcessingDomain::RadiometricResidual;
    RangePolicy range_policy = RangePolicy::NormalizedFloat;
    FrameGeometry geometry;
    CoordinateSpace coordinate_space = CoordinateSpace::FrameGlobal;
};
```

## Поля

| Поле | Тип | Навіщо | Для яких обчислень | Пам'ять |
|---|---|---|---|---|
| `frame_id` | `std::uint64_t` | Ідентичність source frame. | Validation, tracing. | 8 B |
| `source_frame_id` | optional `std::uint64_t` | Relation до `FramePacket`. | Audit, reproducibility. | ~16 B |
| `image` | `cv::Mat` | Processing pixels. | Residual, enhancement, detector response. | header ~96 B + payload |
| `pixel_format` | `PixelFormat` | `F32`, `S16`, `S32` або explicit fast route. | Algorithm route validation. | 4 B |
| `value_range` | `PixelRange` | Numeric range після conversion. | Thresholds, clipping, photometry. | ~32 B |
| `processing_domain` | `ProcessingDomain` | Семантика image payload. | Забороняє змішування residual/enhanced/response. | 4 B |
| `range_policy` | `RangePolicy` | Як трактувати значення. | Signed residual, normalization. | 4 B |
| `geometry` | `FrameGeometry` | Розмір і origin representation. | Coordinate validation. | ~16 B |
| `coordinate_space` | `CoordinateSpace` | Frame-global або tile-local. | Merge/geometry correctness. | 4 B |

## Пам'ять

Full-frame `F32` для `1440x1080` коштує приблизно 6.2 MB. Для
`prep.variant = "tiles"` такий buffer не створюється цим route; processing
memory цього route живе у `TileContext.processing_buffer_a/b`. Для
`full_frame`, `roi` і `adaptive_roi` memory policy має бути задана окремою
stage spec.

## Етапи

- Full-frame `ProcessingFrame` може існувати тільки якщо stage spec/config
  явно вибирає full-frame route.
- Stages у route `prep.variant = "tiles"` використовують `TileProcessingFrame`.

## Interpretation

Ця structure дозволяє одному algorithm route працювати з різною бітністю або normalized representation, якщо stage spec явно задає conversion/threshold/range rules.

## Failure cases

- Detector response записується як raw frame без processing semantics.
- `U8` і `U16` змішуються без explicit range policy.
- Binary mask передається як `ProcessingFrame`.

## Typical misuse

- Трактувати кожен `cv::Mat` між stages як еквівалентний processing object.
- Ховати algorithm outputs у processing metadata.

## Open questions

- Standard vocabulary для `processing_domain`.
- Exact normalization/range policy для `F32` detector response.
- Чи мають processing frames бути immutable після stage emission.

## Connections

- belongs_to: dp1.domain.processing
- derived_from: dp1.domain.raw.frame_packet
- uses: dp1.domain.pixel_format
- constrained_by: dp1.domain.memory_ownership
- constrained_by: dp1.domain.threshold
- may_feed: dp1.stage.candidate_extraction
- may_produce: dp1.domain.mask.binary_mask
