---
id: dp1.domain.processing.frame
title: "Кадр у Processing домені DP1"
tags: [dp1, canonical, data-domain, processing, structure]
kind: data-domain-card
source_role: canonical
source:
  file: "project-knowledge/02-dp1/canonical/data_domains/structures/processing/dp1.domain.processing.frame.md"
status: "draft"
---

## Definition

`ProcessingFrame` є canonical structure у Processing domain для кадру або похідного від кадру представлення після одного чи кількох processing stages.

Ця structure використовується тоді, коли semantics вже відрізняються від Raw/Input `FramePacket`.

## Assumptions

- Canonical processing representation за замовчуванням використовує `F32`.
- `S16` і `S32` дозволені тільки як explicit signed residual route.
- `U8` і `U16` у Processing domain дозволені тільки як explicit fast або
  compatibility route, якщо це дозволено configuration `C` і stage spec.
- OpenCV carrier не визначає semantics без explicit pixel format і processing domain.
- Code-level structure має бути підтверджена окремою implementation task.

## Theorem / Contract

`ProcessingFrame` має мінімально містити або посилатися на:

- `frame_id` — ідентичність кадру, узгоджена з `FramePacket`.
- `source_frame_ref` — optional reference на source `FramePacket`.
- `image` — processing storage carrier.
- `pixel_format` — за замовчуванням `F32`; `S16` або `S32` тільки для explicit
  signed residual route; `U8` або `U16` тільки для explicit fast/compatibility
  route, defined by `dp1.domain.pixel_format`.
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
    RangePolicy range_policy = RangePolicy::SignedResidual;
    FrameGeometry geometry;
    CoordinateSpace coordinate_space = CoordinateSpace::FrameGlobal;
};
```

Default `range_policy` не є глобальним для всіх Processing domains:

```yaml
range_policy_defaults:
  - processing_domain: "RadiometricResidual"
    default_range_policy: "SignedResidual"
  - processing_domain: "RadiometricCorrected"
    default_range_policy: "ClippedToInputRange або explicit route policy"
  - processing_domain: "EnhancedFrame"
    default_range_policy: "NormalizedFloat або explicit route policy"
  - processing_domain: "DetectorResponse"
    default_range_policy: "DetectorResponse або explicit NormalizedFloat"
```

## Поля

```yaml
fields:
  - name: "`frame_id`"
    type: "`std::uint64_t`"
    purpose: "Ідентичність source frame."
    used_for: "Validation, tracing."
    memory: "8 B"
  - name: "`source_frame_id`"
    type: "optional `std::uint64_t`"
    purpose: "Relation до `FramePacket`."
    used_for: "Audit, reproducibility."
    memory: "~16 B"
  - name: "`image`"
    type: "`cv::Mat`"
    purpose: "Processing pixels."
    used_for: "Residual, enhancement, detector response."
    memory: "header ~96 B + payload"
  - name: "`pixel_format`"
    type: "`PixelFormat`"
    purpose: "`F32`, `S16`, `S32` або explicit fast route."
    used_for: "Algorithm route validation."
    memory: "4 B"
  - name: "`value_range`"
    type: "`PixelRange`"
    purpose: "Numeric range після conversion."
    used_for: "Thresholds, clipping, photometry."
    memory: "~32 B"
  - name: "`processing_domain`"
    type: "`ProcessingDomain`"
    purpose: "Семантика image payload."
    used_for: "Забороняє змішування residual/enhanced/response."
    memory: "4 B"
  - name: "`range_policy`"
    type: "`RangePolicy`"
    purpose: "Як трактувати значення."
    used_for: "Signed residual, normalization."
    memory: "4 B"
  - name: "`geometry`"
    type: "`FrameGeometry`"
    purpose: "Розмір і origin representation."
    used_for: "Coordinate validation."
    memory: "~16 B"
  - name: "`coordinate_space`"
    type: "`CoordinateSpace`"
    purpose: "Frame-global або tile-local."
    used_for: "Merge/geometry correctness."
    memory: "4 B"
```

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
- `image` має відповідати `dp1.domain.opencv_invariants`.
- `MaskU8` не є допустимим `ProcessingFrame.pixel_format`.

## Interpretation

Ця structure дозволяє одному algorithm route працювати з різною бітністю або normalized representation, якщо stage spec явно задає conversion/threshold/range rules.

## Failure cases

- Detector response записується як raw frame без processing semantics.
- `U8` і `U16` змішуються без explicit range policy.
- Binary mask передається як `ProcessingFrame`.
- `RadiometricResidual` отримує default `NormalizedFloat` без explicit route
  policy.

## Typical misuse

- Трактувати кожен `cv::Mat` між stages як еквівалентний processing object.
- Ховати algorithm outputs у processing metadata.

## Open questions

- Standard vocabulary для `processing_domain`.
- Exact normalization/range policy для `F32` detector response.
- Чи мають processing frames бути immutable після stage emission.
- Exact behavior для explicit `U8` / `U16` compatibility route.

## Connections

- belongs_to: dp1.domain.processing
- derived_from: dp1.domain.raw.frame_packet
- uses: dp1.domain.pixel_format
- constrained_by: dp1.domain.opencv_invariants
- constrained_by: dp1.domain.memory_ownership
- constrained_by: dp1.domain.threshold
- may_feed: dp1.stage.candidate_extraction
- may_produce: dp1.domain.mask.binary_mask
