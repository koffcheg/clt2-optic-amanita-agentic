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

- Tile-local execution є target model для майбутньої реалізації, а не claim про поточний runtime.
- Tile outputs можуть бути сформовані в tile-local coordinates і мають бути перетворені у global frame coordinates під час merge.
- Конкретний merge algorithm не входить у scope цієї картки.

## Theorem / Contract

`TileResult` має мінімально містити або посилатися на:

- `tile_id` і `frame_id`.
- `valid_area` — tile area, результати якої приймаються після border crop.
- `candidate_results` — tile-local або globalized `Candidate` structures.
- `segment_results` — tile-local або globalized `Segment` structures.
- `measurement_results` — tile-local або globalized `MeasurementRecord` structures.
- `diagnostics` — bounded warnings/errors, згенеровані під час tile processing.
- `profiling_summary` — timing/profile summary для tile.

`TileResult` має явно визначати, чи coordinates є tile-local або frame-global до merge.

## Interpretation

`TileResult` є explicit boundary між tile-local work і frame-level aggregation. Він не дозволяє tile workers приховано змінювати global output.

Очікуваний merge step:

```text
TileResult[] -> crop/filter by valid_area -> transform to global coordinates -> remove border duplicates -> frame-level MeasurementRecord[]
```

## Failure cases

- Tile-local outputs додаються глобально без coordinate transform.
- Border duplicates не видаляються.
- Detections із invalid border-area приймаються як final measurements.
- Tile worker пише напряму в global measurement output без `TileResult`/merge contract.

## Typical misuse

- Трактувати `TileResult` як final DP1 output без merge.
- Ховати `TileResult` всередині `FrameContext` або `TileContext`.

## Open questions

- Exact duplicate suppression policy на tile borders.
- Чи в MVP глобально merge-яться candidates/segments, чи тільки measurements.
- Merge ordering і determinism requirements.

## Connections

- belongs_to: dp1.domain.runtime
- produced_by: dp1.domain.runtime.tile_context
- may_contain: dp1.domain.struct.candidate
- may_contain: dp1.domain.struct.segment
- may_contain: dp1.domain.measurement.record
- merged_into: dp1.domain.measurement
