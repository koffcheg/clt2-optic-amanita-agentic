---
id: dp1.domain.runtime.tile_result
title:
  uk: "Результат tile-обробки DP1"
  en: "DP1 tile result structure"
tags: [dp1, canonical, data-domain, runtime, tile, parallelism]
kind: data-domain-card
source_role: canonical
source:
  file: "project-knowledge/02-dp1/canonical/data_domains/dp1.domain.runtime.tile_result.md"
  lines: "1-N"
status: "draft"
---

## Definition

`TileResult` є runtime/output structure для результатів tile-local DP1 execution, які мають бути merged у frame-global результат.

## Assumptions

- Tile-local execution is a target model for future implementation, not a current-runtime claim.
- Tile outputs may be produced in tile-local coordinates and must be converted to global frame coordinates during merge.
- Exact merge algorithm is out of scope for this card.

## Theorem / Contract

`TileResult` має мінімально містити або посилатися на:

- `tile_id` and `frame_id`.
- `valid_area` — tile area whose results should be accepted after border crop.
- `candidate_results` — tile-local or globalized `Candidate` structures.
- `segment_results` — tile-local or globalized `Segment` structures.
- `measurement_results` — tile-local or globalized `MeasurementRecord` structures.
- `diagnostics` — bounded warnings/errors emitted during tile processing.
- `profiling_summary` — timing/profile summary for the tile.

TileResult must declare whether coordinates are tile-local or frame-global before merge.

## Interpretation

TileResult is the explicit boundary between tile-local work and frame-level aggregation. It prevents hidden global mutation from tile workers.

The intended merge step:

```text
TileResult[] -> crop/filter by valid_area -> transform to global coordinates -> remove border duplicates -> frame-level MeasurementRecord[]
```

## Failure cases

- Tile-local outputs are appended globally without coordinate transform.
- Border duplicates are not removed.
- Invalid border-area detections are accepted as final measurements.
- Tile worker writes directly to global measurement output without merge contract.

## Typical misuse

- Treating TileResult as final DP1 output without merge.
- Hiding TileResult inside FrameContext or TileContext.

## Open questions

- Exact duplicate suppression policy at tile borders.
- Whether candidates/segments or only measurements are merged globally in MVP.
- Merge ordering and determinism requirements.

## Connections

- belongs_to: dp1.domain.runtime
- produced_by: dp1.domain.runtime.tile_context
- may_contain: dp1.domain.struct.candidate
- may_contain: dp1.domain.struct.segment
- may_contain: dp1.domain.measurement.record
- merged_into: dp1.domain.measurement
