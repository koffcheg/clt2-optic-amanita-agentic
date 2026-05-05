---
id: dp1.domain.runtime.tile_desc
title:
  uk: "Опис tile/ROI для DP1"
  en: "DP1 tile descriptor"
tags: [dp1, canonical, data-domain, runtime, tile, parallelism]
kind: data-domain-card
source_role: canonical
source:
  file: "project-knowledge/02-dp1/canonical/data_domains/dp1.domain.runtime.tile_desc.md"
status: "draft"
---

## Definition

`TileDesc` є lightweight runtime structure для опису tile/ROI, який може оброблятися локально або паралельно.

Це не image buffer і не копія кадру.

## Assumptions

- Tile processing є target execution model, а не claim про поточну реалізацію.
- ROI може бути представлений як OpenCV `cv::Rect` або equivalent geometry в implementation.
- Конкретна parallel scheduler / thread-pool implementation не входить у scope цієї картки.

## Theorem / Contract

`TileDesc` має мінімально містити або посилатися на:

- `tile_id` — stable tile identifier у межах кадру.
- `frame_id` — ідентифікатор source frame.
- `roi` — valid tile area у глобальних координатах кадру.
- `roi_with_border` — processing area з урахуванням overlap/border.
- `valid_area` — crop area, результати якої вважаються валідними після border-dependent operations.
- `origin_px` — tile origin у глобальних координатах кадру.
- optional `border_policy` — required overlap semantics для filters/morphology.

`TileDesc` має бути cheap to copy/pass між workers.

## Interpretation

Очікуваний tile execution pattern:

```text
build TileDesc list from FramePacket
parallel worker reads source frame ROI using TileDesc
worker produces TileResult
merge TileResult objects into global frame coordinates
```

## Failure cases

- Tile копіюється як image buffer замість опису через ROI metadata.
- Border/overlap не враховано для filters або morphology.
- Local coordinates видаються як global coordinates без transform.

## Typical misuse

- Трактувати `TileDesc` як owner image memory.
- Використовувати tile-local output без valid-area crop.

## Open questions

- Standard tile size і overlap policy.
- Чи tile grid є static або config-driven.
- Exact boundary behavior на краях кадру.

## Connections

- belongs_to: dp1.domain.runtime
- describes_view_of: dp1.domain.raw.frame_packet
- used_by: dp1.domain.runtime.tile_context
- produces_scope_for: dp1.domain.runtime.tile_result
