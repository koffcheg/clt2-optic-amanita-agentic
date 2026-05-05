---
id: dp1.domain.runtime.tile_context
title:
  uk: "Runtime-контекст tile для DP1"
  en: "DP1 tile runtime context"
tags: [dp1, canonical, data-domain, runtime, tile, parallelism]
kind: data-domain-card
source_role: canonical
source:
  file: "project-knowledge/02-dp1/canonical/data_domains/dp1.domain.runtime.tile_context.md"
status: "draft"
---

## Definition

`TileContext` є runtime structure для per-tile або per-worker working state під час tile-local DP1 execution.

Він не замінює `FrameContext` і не є global state.

## Assumptions

- Tile-local execution є target model для майбутньої реалізації, а не claim про поточний runtime.
- Кожен worker/thread має отримувати або володіти власними tile-local working buffers.
- Доступ до raw/source frame має бути read-only, якщо майбутня task card явно не визначить іншу модель.

## Theorem / Contract

`TileContext` має мінімально містити або посилатися на:

- `tile_id` і `frame_id`.
- `tile_desc_ref` — source tile descriptor.
- `frame_context_ref` — parent frame runtime context.
- `processing_buffer` — tile-local processing payload, часто route `F32`.
- `mask_buffer` — tile-local binary mask buffer, зазвичай `MaskU8`.
- `candidate_buffer` — tile-local candidates.
- `segment_buffer` — tile-local segments.
- `measurement_buffer` — tile-local measurements.
- `profiling_trace` — tile-local timing/profile events.
- `warnings` / `errors` — tile-local diagnostics.

Buffers у `TileContext` не мають shared mutable access між workers, якщо implementation task явно не визначає synchronization.

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
- emits: dp1.domain.runtime.tile_result
