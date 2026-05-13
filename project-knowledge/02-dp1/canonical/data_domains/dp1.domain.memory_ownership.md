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

Ця картка визначає канонічну політику володіння пам'яттю та часу життя для
кадрів DP1, тайлів і структур обробки.

## Assumptions

- Канонічний DP1 має підтримувати тайловий маршрут, ощадливий до пам'яті.
- Тайлові виконавці не повинні створювати повнокадрові проміжні буфери у маршруті
  `prep.variant = "tiles"`.
- Початковий raw frame доступний тайловим виконавцям тільки для читання.

## Theorem / Contract

Канонічні правила володіння:

- `FramePacket` володіє або утримує час життя повнокадрового raw image payload.
- `TileRawView` є неволодіючим ROI view на `FramePacket.image`.
- `TileProcessingFrame` і `TileBinaryMask` посилаються на буфери, якими володіє
  `TileContext`.
- `TileContext` є власником reusable tile-local buffers для одного виконавця.
- `ProcessingFrame` володіє повнокадровим processing payload тільки у `full_frame`,
  `roi` або явно визначених маршрутах.
- `TileResult` переносить структурні outputs, але не буфери зображень.
- `TileRawView.image` є read-only за контрактом, навіть якщо `cv::Mat` API
  технічно mutable.
- `FrameContext` не володіє primary image buffers, masks, candidates, segments,
  validated objects або measurements.
- Runtime-рядки (`source_id`, `source_ref`, `pipeline_run_id`) не мають бути
  hot-path identity mechanism без explicit interning або numeric-id policy.

Спільні mutable image buffers між виконавцями заборонені без явного
synchronization contract в implementation task.

Транспортна пам'ять і алгоритмічна пам'ять історії є різними шарами володіння:

- Позичені транспортні view можуть посилатися на producer-owned або runtime-owned
  input memory тільки протягом часу життя, визначеного input boundary.
- Довгоживучі stateful algorithms не мають зберігати позичені транспортні view
  як історію.
- Якщо stateful algorithm потребує попередні кадри, він володіє або явно
  позичає обмежений буфер історії, визначений його stage spec.

Правила history для МКМФ / `inverse_median`:

- Часова історія МКМФ належить реалізації `inverse_median` і обмежена
  контрактом `dp1.domain.runtime.cyclic_frame_buffer`.
- IPC/shared-memory slots, якщо вони присутні в input route, є transport memory
  і не є слотами історії МКМФ.
- Поточний кадр може читатися через позичений read-only view, поки input boundary
  гарантує час життя цього view.
- Кадр, відібраний у temporal window МКМФ, копіюється в algorithm-owned
  `CyclicFrameBuffer`.
- Ця копія в історію є необхідним algorithm state і не порушує zero-copy transport
  для current-frame ingestion.
- Слоти історії МКМФ мають зберігати одну geometry, pixel format, OpenCV type і
  route metadata до явного reset/reallocation.

## Interpretation

Ця policy оптимізує memory footprint і паралельність: raw frame зберігається
один раз, тайлові виконавці читають ROI views і пишуть тільки у власні reusable
buffers.

Для stateful temporal stages ощадлива до пам'яті доставка input не усуває
потребу в обмеженій algorithm-owned history. Ownership boundary має явно
розрізняти позичені current-frame views і збережені копії історії.

## Failure cases

- `TileRawView` виконує deep copy raw ROI для кожного tile без stage spec.
- Виконавець пише у shared processing buffer.
- `TileResult` переносить `cv::Mat` payload.
- Full-frame `F32` buffers створюються в tile route для кожного stage.
- `FrameContext` використовується як прихований owner output vectors.
- Stateful stage зберігає borrowed input/transport view після завершення
  часу життя source frame.
- IPC/shared-memory ring buffer використовується як temporal history buffer для
  МКМФ.
- History МКМФ продовжує використовувати slots після зміни geometry, pixel
  format або route metadata без explicit reset/reallocation.

## Typical misuse

- Вважати `cv::Mat` ownership очевидним без domain contract.
- Зберігати non-owning tile view після завершення часу життя source frame.

## Open questions

- Точний ownership enum для future C++ DTO.
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
- constrains: dp1.domain.runtime.cyclic_frame_buffer
- constrains: dp1.stage_spec.radiometric_correction.inverse_median
- constrains: dp1.domain.opencv_invariants
