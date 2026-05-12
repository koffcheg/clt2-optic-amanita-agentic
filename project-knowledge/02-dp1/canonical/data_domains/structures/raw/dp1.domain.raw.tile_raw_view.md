---
id: dp1.domain.raw.tile_raw_view
title: "Локальний raw view tile у Raw/Input домені DP1"
tags: [dp1, canonical, data-domain, raw, structure, tile, bit-depth]
kind: data-domain-card
source_role: canonical
source:
  file: "project-knowledge/02-dp1/canonical/data_domains/structures/raw/dp1.domain.raw.tile_raw_view.md"
status: "draft"
---

## Definition

`TileRawView` є tile-local view на `FramePacket.image`. Він описує raw pixels
одного tile з border/overlap і route metadata для `U8` або `U16` input.

Це не копія кадру і не owner image memory.

## Assumptions

- DP1 instance підключений до однієї камери і отримує повний raw frame у
  `FramePacket`.
- Ця structure використовується, коли `prep.variant = "tiles"` або інший stage spec явно вибирає tile execution.
- `TileRawView.image` створюється як ROI view через `TileDesc.roi_with_border`.
- Raw pixels read-only для tile workers.

## Theorem / Contract

Рекомендована C++ форма:

```cpp
struct TileRawView {
    std::uint64_t frame_id = 0;
    int tile_id = -1;
    cv::Mat image;
    PixelFormat pixel_format = PixelFormat::U16;
    InputBitDepth bit_depth = InputBitDepth::Bit16;
    PixelRange pixel_range;
    cv::Rect valid_area;
    cv::Point origin_px{0, 0};
};
```

## Поля

```yaml
fields:
  - name: "`frame_id`"
    type: "`std::uint64_t`"
    purpose: "Зв'язує tile з source frame."
    used_for: "Перевірка, merge, diagnostics."
    memory: "8 B"
  - name: "`tile_id`"
    type: "`int`"
    purpose: "Ідентифікує tile у межах кадру."
    used_for: "Профілювання, duplicate handling, debug trace."
    memory: "4 B"
  - name: "`image`"
    type: "`cv::Mat`"
    purpose: "ROI view на raw pixels у `FramePacket.image`."
    used_for: "Input для radiometric route і photometry reference."
    memory: "header ~96 B; payload не належить structure"
  - name: "`pixel_format`"
    type: "`PixelFormat`"
    purpose: "Вказує carrier: `U8` або `U16`."
    used_for: "Вибір allowed algorithm implementation і validation."
    memory: "4 B"
  - name: "`bit_depth`"
    type: "`InputBitDepth`"
    purpose: "Вказує фактичну сенсорну розрядність."
    used_for: "Threshold normalization, range validation, photometry."
    memory: "4 B"
  - name: "`pixel_range`"
    type: "`PixelRange`"
    purpose: "Вказує min/max/black/saturation levels."
    used_for: "Conversion, signed residual, photometry stats."
    memory: "~32 B"
  - name: "`valid_area`"
    type: "`cv::Rect`"
    purpose: "Позначає border-safe область tile."
    used_for: "Crop output, segmentation, measurement acceptance."
    memory: "16 B"
  - name: "`origin_px`"
    type: "`cv::Point`"
    purpose: "Local-to-global offset."
    used_for: "Перетворення координат candidates/segments/measurements."
    memory: "8 B"
```

## Пам'ять

`TileRawView` не виділяє payload. Його `image` є shallow OpenCV ROI header на
пам'ять `FramePacket.image`.

Приклад:

```cpp
cv::Mat raw_tile = frame_packet.image(tile_desc.roi_with_border);
```

Це має залишатися view. `.clone()` або `copyTo()` у hot path заборонені без
explicit stage spec.

## Етапи

- `prep` створює `TileDesc[]`.
- Runtime/scheduler створює `TileRawView` для кожного `TileDesc`.
- `radiometric_correction` читає `TileRawView`.
- `measurement` може читати `TileRawView` як photometry source, якщо stage spec
  вибирає raw photometry.

## Обмеження

- `TileRawView.image` read-only.
- `pixel_format`, `bit_depth` і `pixel_range` мають бути успадковані з
  `FramePacket`.
- `valid_area` виражений у tile-local coordinates відносно
  `TileDesc.roi_with_border` згідно з `dp1.domain.coordinates` і не має
  виходити за межі `image`.
- Algorithm implementation не має виводити повну semantics тільки з
  `image.type()`.
- `image` може бути OpenCV submatrix і не має гарантувати `isContinuous()`.

## Failure cases

- Tile worker змінює raw pixels.
- ROI створюється як deep copy для кожного tile.
- `CV_16UC1` input трактується як `Bit16`, хоча route задає `Bit12`.
- Tile-local coordinates видаються як frame-global без `origin_px`.
- Stage code обробляє ROI view як continuous buffer без перевірки.

## Typical misuse

- Зберігати `TileRawView` після завершення lifetime source `FramePacket.image`.
- Використовувати `TileRawView` як output processing stage.

## Open questions

- Чи потрібна окрема lifetime policy для queued frames, якщо camera input
  випереджає tile processing.

## Connections

- belongs_to: dp1.domain.raw
- derived_from: dp1.domain.raw.frame_packet
- scoped_by: dp1.domain.runtime.tile_desc
- uses: dp1.domain.pixel_format
- constrained_by: dp1.domain.opencv_invariants
- constrained_by: dp1.domain.memory_ownership
- constrained_by: dp1.domain.coordinates
- feeds: dp1.stage.radiometric_correction
- may_feed: dp1.stage.measurement
