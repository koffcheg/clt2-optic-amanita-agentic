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
  lines: "1-N"
status: "draft"
---

## Definition

`TileDesc` є lightweight runtime structure для опису tile/ROI, який може бути оброблений локально або паралельно.

Це не image buffer і не копія кадру.

## Assumptions

- Tile processing is a target execution model, not a current-runtime claim.
- ROI may be represented as OpenCV `cv::Rect` or equivalent geometry in implementation.
- Exact parallel scheduler/thread-pool implementation is out of scope for this card.

## Theorem / Contract

`TileDesc` має мінімально містити або посилатися на:

- `tile_id` — stable tile identifier within a frame.
- `frame_id` — source frame identifier.
- `roi` — valid tile area in global frame coordinates.
- `roi_with_border` — processing area including overlap/border.
- `valid_area` — crop area whose results are valid after border-dependent operations.
- `origin_px` — tile origin in global frame coordinates.
- optional `border_policy` — required overlap semantics for filters/morphology.

`TileDesc` має бути cheap to copy/pass between workers.

## Interpretation

The intended tile execution pattern is:

```text
build TileDesc list from FramePacket
parallel worker reads source frame ROI using TileDesc
worker produces TileResult
merge TileResult objects into global frame coordinates
```

## Failure cases

- Tile is copied eagerly instead of described by ROI metadata.
- Border/overlap is omitted for filters or morphology.
- Local coordinates are emitted as global coordinates without transform.

## Typical misuse

- Treating `TileDesc` as owning image memory.
- Using tile-local output without valid-area crop.

## Open questions

- Standard tile size and overlap policy.
- Whether tile grid is static or config-driven.
- Exact boundary behavior at frame edges.

## Connections

- belongs_to: dp1.domain.runtime
- describes_view_of: dp1.domain.raw.frame_packet
- used_by: dp1.domain.runtime.tile_context
- produces_scope_for: dp1.domain.runtime.tile_result
