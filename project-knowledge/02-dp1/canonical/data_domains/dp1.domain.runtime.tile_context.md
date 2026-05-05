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
  lines: "1-N"
status: "draft"
---

## Definition

`TileContext` є runtime structure для per-tile або per-worker working state під час tile-local DP1 execution.

Він не замінює `FrameContext` і не є глобальним state.

## Assumptions

- Tile-local execution is a target model for future implementation, not a current-runtime claim.
- Each worker/thread should own or receive its own tile-local working buffers.
- Raw/source frame access should be read-only unless a future task explicitly defines another model.

## Theorem / Contract

`TileContext` має мінімально містити або посилатися на:

- `tile_id` and `frame_id`.
- `tile_desc_ref` — source tile descriptor.
- `frame_context_ref` — parent frame runtime context.
- `processing_buffer` — tile-local processing payload, often `F32` route.
- `mask_buffer` — tile-local binary mask buffer, typically `MaskU8`.
- `candidate_buffer` — tile-local candidates.
- `segment_buffer` — tile-local segments.
- `measurement_buffer` — tile-local measurements.
- `profiling_trace` — tile-local timing/profile events.
- `warnings` / `errors` — tile-local diagnostics.

`TileContext` buffers must not be shared mutably across workers unless an implementation task defines explicit synchronization.

## Interpretation

`TileContext` is the memory-local companion to `TileDesc`. It allows a worker to run one or more DP1 stages over a tile without allocating full-frame processing and mask buffers for every intermediate representation.

## Failure cases

- Tile workers share mutable buffers accidentally.
- TileContext stores global frame outputs without merge semantics.
- Tile-local coordinates are not transformed before final merge.

## Typical misuse

- Using `TileContext` as global mutable pipeline state.
- Hiding final stage outputs in context instead of emitting `TileResult`.

## Open questions

- Exact buffer reuse policy.
- Whether TileContext is per tile, per worker, or pooled.
- Required synchronization model for profiling and diagnostics.

## Connections

- belongs_to: dp1.domain.runtime
- parent_context: dp1.domain.runtime.frame_context
- scoped_by: dp1.domain.runtime.tile_desc
- emits: dp1.domain.runtime.tile_result
