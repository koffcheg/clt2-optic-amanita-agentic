---
id: dp1.domain.runtime.tile_result
title: "Результат tile-обробки DP1"
tags: [dp1, canonical, data-domain, runtime, tile, parallelism]
kind: data-domain-card
source_role: canonical
source:
  file: "project-knowledge/02-dp1/canonical/data_domains/structures/runtime/dp1.domain.runtime.tile_result.md"
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
- `profiling` — timing/profile result для tile з `dp1.domain.profiling`.

`TileResult` має явно визначати, чи coordinates є tile-local або frame-global до merge.

Production route має переносити мінімальний structural payload, потрібний для
merge і final output. `candidate_results`, `segment_results` і
`validated_object_results` є optional/debug payload, якщо merge потребує тільки
`measurement_results`.

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
    TileProfilingResult profiling;
};
```

## Поля

```yaml
fields:
  - name: "`tile_id`"
    type: "`int`"
    purpose: "Source tile id."
    used_for: "Merge ordering, diagnostics."
    memory: "4 B"
  - name: "`frame_id`"
    type: "`std::uint64_t`"
    purpose: "Source frame id."
    used_for: "Validation."
    memory: "8 B"
  - name: "`valid_area`"
    type: "`cv::Rect`"
    purpose: "Border-safe area."
    used_for: "Crop/filter tile outputs."
    memory: "16 B"
  - name: "`origin_px`"
    type: "`cv::Point`"
    purpose: "Local-to-global offset."
    used_for: "Coordinate transform."
    memory: "8 B"
  - name: "`candidate_results`"
    type: "`std::vector<Candidate>`"
    purpose: "Optional intermediate tile candidates."
    used_for: "Diagnostics/merge support."
    memory: "~24 B + payload"
  - name: "`segment_results`"
    type: "`std::vector<Segment>`"
    purpose: "Optional intermediate tile segments."
    used_for: "Diagnostics/merge support."
    memory: "~24 B + payload"
  - name: "`validated_object_results`"
    type: "`std::vector<ValidatedObject>`"
    purpose: "Optional tile validated objects."
    used_for: "Merge/diagnostics before measurement."
    memory: "~24 B + payload"
  - name: "`measurement_results`"
    type: "`std::vector<MeasurementRecord>`"
    purpose: "Main tile output."
    used_for: "Frame-level merge and DP2 handoff."
    memory: "~24 B + payload"
  - name: "`diagnostics`"
    type: "`std::vector<DiagnosticMessage>`"
    purpose: "Tile warnings/errors."
    used_for: "Report/debug."
    memory: "bounded"
  - name: "`profiling`"
    type: "`TileProfilingResult` із `dp1.domain.profiling`"
    purpose: "Tile timing, cardinality and memory summary."
    used_for: "Performance analysis, frame-level aggregation."
    memory: "bounded summary; raw trace only when explicitly configured"
```

## Пам'ять

`TileResult` не містить image buffers. Він має переносити тільки structural
outputs. `candidate_results`, `segment_results` і `validated_object_results`
можуть бути optional у production, якщо final merge потребує тільки
`measurement_results`.
Vectors мають передаватися move/transfer semantics у future implementation, а
не копіюватися без потреби між `TileContext` і `TileResult`.

`profiling` не має містити image buffers або raw stage outputs. Raw
tile traces не мають копіюватися у `TileResult`, якщо це не ввімкнено через
`dp1.config.application.profiling`.

## Interpretation

`TileResult` є explicit boundary між tile-local work і frame-level aggregation. Він не дозволяє tile workers приховано змінювати global output.

Очікуваний merge step:

```text
TileResult[] -> crop/filter by valid_area -> transform to global coordinates -> remove border duplicates -> frame-level MeasurementRecord[]
```

Формули globalization і coordinate-space правила визначає
`dp1.domain.coordinates`; ця картка не дублює їх.

## Failure cases

- Tile-local outputs додаються глобально без coordinate transform.
- Border duplicates не видаляються.
- Detections із invalid border-area приймаються як final measurements.
- Tile worker пише напряму в global measurement output без `TileResult`/merge contract.
- TileResult копіює всі intermediate vectors у production route без
  debug/merge потреби.
- TileResult переносить unbounded raw profiling trace замість bounded summary.

## Typical misuse

- Трактувати `TileResult` як final DP1 output без merge.
- Ховати `TileResult` всередині `FrameContext` або `TileContext`.

## Open questions

- Exact duplicate suppression policy на tile borders.
- Чи globally merge-яться candidates/segments/validated objects, чи тільки
  measurements.
- Merge ordering і determinism requirements.
- Exact move-only або transfer ownership policy для future C++ DTO.

## Connections

- belongs_to: dp1.domain.runtime
- produced_by: dp1.domain.runtime.tile_context
- may_contain: dp1.domain.struct.candidate
- may_contain: dp1.domain.struct.segment
- may_contain: dp1.domain.struct.validated_object
- may_contain: dp1.domain.measurement.record
- constrained_by: dp1.domain.memory_ownership
- constrained_by: dp1.domain.coordinates
- uses: dp1.domain.profiling
- configured_by: dp1.config.application.profiling
- merged_into: dp1.domain.measurement
