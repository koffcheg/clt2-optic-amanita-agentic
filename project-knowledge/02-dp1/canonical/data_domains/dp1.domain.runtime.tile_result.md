---
id: dp1.domain.runtime.tile_result
title: "Результат tile-обробки DP1"
tags: [dp1, canonical, data-domain, runtime, tile, parallelism]
kind: data-domain-card
source_role: canonical
source:
  file: "project-knowledge/02-dp1/canonical/data_domains/dp1.domain.runtime.tile_result.md"
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
- `validated_object_results` — tile-local або globalized `ValidatedObject`
  structures, якщо merge або diagnostics потребують object-level records.
- `measurement_results` — tile-local або globalized `MeasurementRecord` structures.
- `diagnostics` — bounded warnings/errors, згенеровані під час tile processing.
- `profiling_summary` — timing/profile summary для tile.

`TileResult` має явно визначати, чи coordinates є tile-local або frame-global до merge.

Рекомендована C++ форма:

```cpp
struct TileResult {
    int tile_id = -1;
    std::uint64_t frame_id = 0;
    cv::Rect valid_area;
    cv::Point origin_px{0, 0};
    std::vector<Candidate> candidate_results;
    std::vector<Segment> segment_results;
    std::vector<ValidatedObject> validated_object_results;
    std::vector<MeasurementRecord> measurement_results;
    std::vector<DiagnosticMessage> diagnostics;
    TileProfilingSummary profiling_summary;
};
```

## Поля

| Поле | Тип | Навіщо | Для яких обчислень | Пам'ять |
|---|---|---|---|---|
| `tile_id` | `int` | Source tile id. | Merge ordering, diagnostics. | 4 B |
| `frame_id` | `std::uint64_t` | Source frame id. | Validation. | 8 B |
| `valid_area` | `cv::Rect` | Border-safe area. | Crop/filter tile outputs. | 16 B |
| `origin_px` | `cv::Point` | Local-to-global offset. | Coordinate transform. | 8 B |
| `candidate_results` | `std::vector<Candidate>` | Optional intermediate tile candidates. | Diagnostics/merge support. | ~24 B + payload |
| `segment_results` | `std::vector<Segment>` | Optional intermediate tile segments. | Diagnostics/merge support. | ~24 B + payload |
| `validated_object_results` | `std::vector<ValidatedObject>` | Optional tile validated objects. | Merge/diagnostics before measurement. | ~24 B + payload |
| `measurement_results` | `std::vector<MeasurementRecord>` | Main tile output. | Frame-level merge and DP2 handoff. | ~24 B + payload |
| `diagnostics` | `std::vector<DiagnosticMessage>` | Tile warnings/errors. | Report/debug. | bounded |
| `profiling_summary` | `TileProfilingSummary` | Tile timing summary. | Performance analysis. | implementation-specific |

## Пам'ять

`TileResult` не містить image buffers. Він має переносити тільки structural
outputs. `candidate_results`, `segment_results` і `validated_object_results`
можуть бути optional у production, якщо final merge потребує тільки
`measurement_results`.

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
- Чи globally merge-яться candidates/segments/validated objects, чи тільки
  measurements.
- Merge ordering і determinism requirements.

## Connections

- belongs_to: dp1.domain.runtime
- produced_by: dp1.domain.runtime.tile_context
- may_contain: dp1.domain.struct.candidate
- may_contain: dp1.domain.struct.segment
- may_contain: dp1.domain.struct.validated_object
- may_contain: dp1.domain.measurement.record
- constrained_by: dp1.domain.memory_ownership
- constrained_by: dp1.domain.coordinates
- merged_into: dp1.domain.measurement
