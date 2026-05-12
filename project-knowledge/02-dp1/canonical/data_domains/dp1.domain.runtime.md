---
id: dp1.domain.runtime
title:
  uk: "Runtime домен DP1"
  en: "DP1 Runtime domain"
tags: [dp1, canonical, data-domain, runtime, tile, profiling]
kind: data-domain-card
source_role: canonical
source:
  file: "project-knowledge/02-dp1/canonical/data_domains/dp1.domain.runtime.md"
status: "draft"
---

## Definition

Runtime domain описує службові canonical structures, які супроводжують
виконання DP1 pipeline, але не є Raw, Processing, Mask, Struct або Measurement
payload.

## Assumptions

- Runtime structures потрібні для context, profiling, diagnostics, tile
  execution, buffer reuse і state ownership.
- Runtime domain не є hidden output container.
- Runtime structures можуть посилатися на data-domain objects, але не
  підміняють explicit stage input/output contracts.

## Theorem / Contract

До Runtime domain належать:

```yaml
runtime_structures:
  - id: "dp1.domain.runtime.frame_context"
    role: "Per-frame runtime/config/profiling/diagnostics context."
  - id: "dp1.domain.runtime.cyclic_frame_buffer"
    role: "Bounded reusable frame-history state for stateful stages."
  - id: "dp1.domain.runtime.tile_desc"
    role: "Tile/ROI metadata, border and valid-area description."
  - id: "dp1.domain.runtime.tile_context"
    role: "Per-worker reusable tile-local buffers and diagnostics."
  - id: "dp1.domain.runtime.tile_result"
    role: "Explicit tile-local structural results before merge."
```

Runtime domain забороняє:

- приховано зберігати primary image buffers у `FrameContext`;
- приховано зберігати masks, candidates, segments, validated objects або
  measurements у `FrameContext`;
- використовувати tile runtime structures як final DP1 output без merge;
- змішувати runtime state кількох camera/source inputs в одному active DP1
  instance.

## Fields / Interface

Runtime structures мають явно декларувати:

```yaml
required_runtime_metadata:
  - "frame_id"
  - "camera_id або source relation, якщо structure frame-scoped"
  - "route або config reference, якщо structure залежить від PipelineConfig"
  - "coordinate_space / origin / valid_area, якщо structure tile-scoped"
  - "profiling або diagnostics bounds, якщо structure накопичує runtime events"
```

## Input / Output

Input:

- `PipelineConfig`;
- `FramePacket`;
- stage-interface contracts;
- tile route metadata.

Output:

- explicit runtime/context structures for stage execution;
- profiling/diagnostic support data;
- tile-local merge boundary through `TileResult`.

## Constraints

- Runtime domain не є computation domain.
- Runtime domain не є DP1 -> DP2 handoff payload.
- Runtime structures не мають ставати source of truth для domain semantics,
  які вже визначені у Raw, Processing, Mask, Struct або Measurement cards.

## Failure cases

- `FrameContext` використовується як глобальний mutable output container.
- `TileResult` переносить `cv::Mat` image buffers.
- `TileDesc` трактується як image payload.
- Runtime state змішує frames або sources без explicit boundary.

## Typical misuse

- Виносити algorithm result у Runtime domain, щоб обійти stage output contract.
- Використовувати runtime diagnostics як machine-readable protocol payload.

## Open questions

- Exact lifetime/ownership enum для runtime DTO.
- Чи потрібна окрема policy для pooled allocation.
- Exact runtime error taxonomy.

## Connections

- contains: dp1.domain.runtime.frame_context
- contains: dp1.domain.runtime.cyclic_frame_buffer
- contains: dp1.domain.runtime.tile_desc
- contains: dp1.domain.runtime.tile_context
- contains: dp1.domain.runtime.tile_result
- constrained_by: dp1.domain.memory_ownership
- constrained_by: dp1.domain.coordinates
- constrained_by: dp1.domain.common_types
- constrained_by: dp1.pipeline.stage_contract
