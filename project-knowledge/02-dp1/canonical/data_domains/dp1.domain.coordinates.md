---
id: dp1.domain.coordinates
title: "Canonical-політика координат DP1"
tags: [dp1, canonical, data-domain, coordinates, tile, merge]
kind: data-domain-card
source_role: canonical
source:
  file: "project-knowledge/02-dp1/canonical/data_domains/dp1.domain.coordinates.md"
status: "draft"
---

## Definition

Ця картка визначає canonical coordinate policy для DP1 frame, tile, struct і
measurement outputs.

## Assumptions

- Tile route може створювати tile-local objects паралельно.
- Final DP1 product для DP1 -> DP2 має бути frame-global.
- Coordinate conversion має бути explicit і traceable.

## Theorem / Contract

Canonical coordinate spaces:

- `FrameGlobal` — coordinates in full source frame.
- `TileLocal` — coordinates local to tile ROI/view.

Rules:

- `FramePacket` і final `MeasurementRecord` використовують `FrameGlobal`.
- `TileRawView`, `TileProcessingFrame`, `TileBinaryMask`, `Candidate`,
  `Segment` і `ValidatedObject` можуть бути `TileLocal` у tile route.
- Tile-local structures мають нести `origin_px`, `valid_area` або source
  relation, достатні для globalization.
- Merge/globalization step є єдиним місцем, де tile-local outputs стають
  frame-global final outputs, якщо stage spec не визначає інший explicit route.
- Border/overlap outputs мають проходити valid-area crop і duplicate
  suppression перед final measurement.
- `cv::Rect` використовує OpenCV half-open convention:
  `x <= px < x + width`, `y <= py < y + height`; right/bottom boundary не
  включається.
- У tile route `valid_area` виражений у tile-local coordinates відносно
  `TileDesc.roi_with_border`.
- Формула tile-local to frame-global для point:
  `global.x = tile_desc.roi_with_border.x + local.x`;
  `global.y = tile_desc.roi_with_border.y + local.y`.
- Формула tile-local to frame-global для `cv::Rect`:
  `global.x = tile_desc.roi_with_border.x + local.x`;
  `global.y = tile_desc.roi_with_border.y + local.y`;
  `width` і `height` не змінюються.
- `centroid_px` з floating coordinates має позначати координату у pixel
  coordinate system. Якщо centroid обчислюється з integer pixel region, stage
  spec має визначити, чи використовується pixel-center convention `x + 0.5`,
  `y + 0.5`, або OpenCV moments convention.

## Interpretation

Ця policy дозволяє workers працювати незалежно в local coordinates і зменшує
shared synchronization. Frame-global result формується після deterministic merge.

## Failure cases

- Tile-local bbox видається як frame-global.
- Border duplicate стає final measurement.
- `origin_px` або `valid_area` втрачені між stages.
- Inclusive rectangle convention використано замість OpenCV half-open
  convention.
- Tile-local `valid_area` помилково трактується як frame-global.

## Typical misuse

- Конвертувати coordinates у кожному stage без єдиної merge policy.
- Змішувати frame-global і tile-local objects в одному vector без
  `coordinate_space`.

## Open questions

- Exact duplicate suppression policy.
- Чи потрібен окремий `CoordinateTransformRef`.
- Єдина centroid convention для всіх measurement variants.

## Connections

- constrains: dp1.domain.raw.tile_raw_view
- constrains: dp1.domain.processing.tile_processing_frame
- constrains: dp1.domain.mask.tile_binary_mask
- constrains: dp1.domain.struct.candidate
- constrains: dp1.domain.struct.segment
- constrains: dp1.domain.struct.validated_object
- constrains: dp1.domain.measurement.record
- constrains: dp1.pipeline.stage_domain_bindings
- constrains: dp1.pipeline.stage_io_matrix
