---
id: dp1.domain.runtime.tile_context
title: "Runtime-контекст tile для DP1"
tags: [dp1, canonical, data-domain, runtime, tile, parallelism]
kind: data-domain-card
source_role: canonical
source:
  file: "project-knowledge/02-dp1/canonical/data_domains/structures/runtime/dp1.domain.runtime.tile_context.md"
status: "draft"
---

## Definition

`TileContext` є runtime structure для per-worker working state під час tile-local DP1 execution.

Він не замінює `FrameContext` і не є global state.

## Assumptions

- Tile-local execution є target model для майбутньої реалізації, а не claim про поточний runtime.
- Кожен worker/thread має отримувати або володіти власними tile-local working buffers.
- Доступ до raw/source frame має бути read-only, якщо майбутня task card явно не визначить іншу модель.

## Theorem / Contract

`TileContext` має мінімально містити або посилатися на:

- `worker_id` — owner worker/thread.
- `processing_buffer_a` — основний tile-local processing payload.
- `processing_buffer_b` — другий tile-local processing payload для ping-pong/non-inplace stages.
- `mask_buffer` — tile-local binary mask buffer, зазвичай `MaskU8`.
- `candidate_buffer` — tile-local candidates.
- `segment_buffer` — tile-local segments.
- `validated_object_buffer` — tile-local validated-object records.
- `measurement_buffer` — tile-local measurements.
- `profiling_trace` — tile-local timing/profile events.
- `warnings` / `errors` — tile-local diagnostics.

Buffers у `TileContext` не мають shared mutable access між workers, якщо implementation task явно не визначає synchronization.

Рекомендована C++ форма:

```cpp
struct TileContext {
    int worker_id = -1;
    cv::Mat processing_buffer_a;
    cv::Mat processing_buffer_b;
    cv::Mat mask_buffer;
    std::vector<Candidate> candidate_buffer;
    std::vector<Segment> segment_buffer;
    std::vector<ValidatedObject> validated_object_buffer;
    std::vector<MeasurementRecord> measurement_buffer;
    std::vector<DiagnosticMessage> diagnostics;
    std::vector<ProfileEvent> profiling_trace;
};
```

## Поля

```yaml
fields:
  - name: "`worker_id`"
    type: "`int`"
    purpose: "Ідентифікує owner worker."
    used_for: "Profiling, diagnostics."
    memory: "4 B"
  - name: "`processing_buffer_a`"
    type: "`cv::Mat`"
    purpose: "Основний reusable processing buffer."
    used_for: "Radiometric, enhancement, matched filtering."
    memory: "header ~96 B + tile payload"
  - name: "`processing_buffer_b`"
    type: "`cv::Mat`"
    purpose: "Другий reusable processing buffer."
    used_for: "Non-inplace або ping-pong stage execution."
    memory: "header ~96 B + tile payload"
  - name: "`mask_buffer`"
    type: "`cv::Mat`"
    purpose: "Reusable binary mask buffer."
    used_for: "Candidate extraction, segmentation."
    memory: "header ~96 B + tile payload"
  - name: "`candidate_buffer`"
    type: "`std::vector<Candidate>`"
    purpose: "Reusable candidate storage."
    used_for: "Connected components output."
    memory: "~24 B + capacity"
  - name: "`segment_buffer`"
    type: "`std::vector<Segment>`"
    purpose: "Reusable segment storage."
    used_for: "Segmentation/contours/filtering."
    memory: "~24 B + capacity"
  - name: "`validated_object_buffer`"
    type: "`std::vector<ValidatedObject>`"
    purpose: "Reusable validated-object storage."
    used_for: "Object filtering output."
    memory: "~24 B + capacity"
  - name: "`measurement_buffer`"
    type: "`std::vector<MeasurementRecord>`"
    purpose: "Reusable measurement storage."
    used_for: "Tile measurement output."
    memory: "~24 B + capacity"
  - name: "`diagnostics`"
    type: "`std::vector<DiagnosticMessage>`"
    purpose: "Tile-local warnings/errors."
    used_for: "Debug without image payload."
    memory: "bounded"
  - name: "`profiling_trace`"
    type: "`std::vector<ProfileEvent>`"
    purpose: "Tile-local timings."
    used_for: "Performance analysis."
    memory: "bounded"
```

## Пам'ять

`TileContext` є головним owner tile-local image memory. Він створюється per
worker, не per tile.

Для tile `256x256` без border:

- один `F32` processing buffer: ~256 KB;
- два `F32` processing buffers: ~512 KB;
- один `MaskU8` buffer: ~64 KB;
- разом image buffers per worker: ~576 KB.

Для 8 workers це приблизно 4.5 MB image buffers, без урахування vector
capacities. Це значно менше, ніж full-frame processing/mask buffers для кожного
stage.

## Етапи

Worker використовує один `TileContext` для послідовного tile-local проходу:

```text
TileRawView
  -> radiometric_correction
  -> enhancement
  -> matched_filtering
  -> candidate_extraction
  -> segmentation_refinement
  -> object_filtering
  -> measurement
  -> TileResult
```

Vectors у `TileContext` очищаються через `clear()`, але не `shrink_to_fit()` у
hot path.

## Interpretation

`TileContext` є memory-local companion до `TileDesc`. Він дозволяє worker виконати один або кілька DP1 stages над tile без створення full-frame buffers для кожного проміжного представлення.

## Failure cases

- Tile workers випадково використовують shared mutable buffers.
- `TileContext` зберігає global frame outputs без merge semantics.
- Tile-local coordinates не перетворюються перед фінальним merge.

## Typical misuse

- Використовувати `TileContext` як global mutable pipeline state.
- Ховати final stage outputs у context замість emission через `TileResult`.

## Open questions

- Exact buffer reuse policy.
- Чи `TileContext` має бути per tile, per worker або pooled.
- Required synchronization model для profiling і diagnostics.

## Connections

- belongs_to: dp1.domain.runtime
- parent_context: dp1.domain.runtime.frame_context
- scoped_by: dp1.domain.runtime.tile_desc
- constrained_by: dp1.domain.memory_ownership
- emits: dp1.domain.runtime.tile_result
