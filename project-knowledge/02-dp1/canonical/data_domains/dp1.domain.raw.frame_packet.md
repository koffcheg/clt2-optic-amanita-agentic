---
id: dp1.domain.raw.frame_packet
title:
  uk: "Пакет кадру в Raw/Input домені DP1"
  en: "DP1 Raw/Input frame packet"
tags: [dp1, canonical, data-domain, raw, structure, frame]
kind: data-domain-card
source_role: canonical
source:
  file: "project-knowledge/02-dp1/canonical/data_domains/dp1.domain.raw.frame_packet.md"
  lines: "1-N"
status: "draft"
---

## Definition

`FramePacket` є canonical structure у Raw/Input domain для представлення кадру, що входить у DP1 pipeline.

Він відповідає на питання: що саме обробляється як вхідний кадр.

## Assumptions

- `cv::Mat` може бути storage carrier для image payload.
- Semantic meaning кадру визначається не лише carrier type, а також `pixel_format`, geometry, source metadata і route context.
- Фактичні поля C++ structure мають бути підтверджені окремою implementation task.

## Theorem / Contract

`FramePacket` має мінімально містити або посилатися на:

- `frame_id` — stable identifier кадру в межах pipeline run.
- `source_id` або `camera_id` — джерело кадру.
- `acquisition_time` або timestamp reference.
- `image` — image payload/storage carrier.
- `pixel_format` — canonical pixel format, defined by `dp1.domain.pixel_format`.
- `geometry` — width, height, coordinate origin policy.
- `metadata` — bounded source/frame metadata, а не arbitrary stage output.

`FramePacket` не має містити algorithm-specific outputs: masks, candidates, segments, measurements або tile-local working buffers.

## Interpretation

`FramePacket` є boundary object для raw або near-raw frame payload. Пізніші processing representations мають використовувати `dp1.domain.processing.frame`, якщо їх semantics відрізняються від input/acquisition frame.

## Failure cases

- Stage записує algorithm output назад у `FramePacket` metadata.
- `frame_id` змінюється між stages без явної remapping policy.
- Pixel format виводиться лише з `cv::Mat::type()` без canonical metadata.

## Typical misuse

- Використовувати `FramePacket` як універсальний контейнер для всіх результатів pipeline.
- Ховати runtime state або profiling у `FramePacket` замість `FrameContext`.

## Open questions

- Остаточна timestamp policy.
- Чи `frame_id` має бути глобальним або per-source/per-run.
- Exact C++ ownership model for image payload.

## Connections

- belongs_to: dp1.domain.raw
- uses: dp1.domain.pixel_format
- accompanied_by: dp1.domain.runtime.frame_context
- may_produce: dp1.domain.processing.frame
