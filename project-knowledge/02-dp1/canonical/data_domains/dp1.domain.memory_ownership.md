---
id: dp1.domain.memory_ownership
title: "Canonical-політика володіння пам'яттю DP1"
tags: [dp1, canonical, data-domain, memory, ownership, parallelism]
kind: data-domain-card
source_role: canonical
source:
  file: "project-knowledge/02-dp1/canonical/data_domains/dp1.domain.memory_ownership.md"
status: "draft"
---

## Definition

Ця картка визначає canonical memory ownership і lifetime policy для DP1 frame,
tile і processing structures.

## Assumptions

- Canonical DP1 має підтримувати memory-efficient tile route.
- Tile workers не повинні створювати full-frame intermediate buffers у route
  `prep.variant = "tiles"`.
- Source raw frame read-only для tile workers.

## Theorem / Contract

Canonical ownership rules:

- `FramePacket` owns або утримує lifetime full-frame raw image payload.
- `TileRawView` є non-owning ROI view на `FramePacket.image`.
- `TileProcessingFrame` і `TileBinaryMask` посилаються на buffers, owned by
  `TileContext`.
- `TileContext` є per-worker owner reusable tile-local buffers.
- `ProcessingFrame` owns full-frame processing payload only у `full_frame`,
  `roi` або explicitly specified routes.
- `TileResult` переносить structural outputs, але не image buffers.
- `TileRawView.image` є read-only by contract, навіть якщо `cv::Mat` API
  технічно mutable.
- `FrameContext` не володіє primary image buffers, masks, candidates, segments,
  validated objects або measurements.
- Runtime strings (`source_id`, `source_ref`, `pipeline_run_id`) не мають бути
  hot-path identity mechanism без explicit interning або numeric-id policy.

Shared mutable image buffers між workers заборонені без explicit synchronization
contract в implementation task.

## Interpretation

Ця policy оптимізує memory footprint і паралельність: raw frame зберігається
один раз, tile workers читають ROI views і пишуть тільки у власні reusable
buffers.

## Failure cases

- `TileRawView` deep-copies raw ROI для кожного tile без stage spec.
- Worker пише у shared processing buffer.
- `TileResult` переносить `cv::Mat` payload.
- Full-frame `F32` buffers створюються в tile route для кожного stage.
- `FrameContext` використовується як прихований owner output vectors.

## Typical misuse

- Вважати `cv::Mat` ownership очевидним без domain contract.
- Зберігати non-owning tile view після завершення lifetime source frame.

## Open questions

- Exact ownership enum для future C++ DTO.
- Pooled allocation policy для `TileContext`.
- Interned-string або numeric-id policy для runtime metadata.

## Connections

- constrains: dp1.domain.raw.frame_packet
- constrains: dp1.domain.raw.tile_raw_view
- constrains: dp1.domain.processing.frame
- constrains: dp1.domain.processing.tile_processing_frame
- constrains: dp1.domain.mask.tile_binary_mask
- constrains: dp1.domain.runtime.tile_context
- constrains: dp1.domain.runtime.tile_result
- constrains: dp1.domain.opencv_invariants
