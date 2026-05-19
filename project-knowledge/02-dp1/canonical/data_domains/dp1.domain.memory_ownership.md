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


Правила ownership для `CanonicalFrame`:

- `CanonicalFrame` є explicit output Stage0 і authoritative input для `Prep`.
- У Stage0.1 `CanonicalFrame.image` може бути `BorrowedReadOnly` view на payload
  `FramePacket` без full-frame copy.
- Borrowed payload не має переживати declared `InputBoundary` або `FrameBoundary`,
  якщо implementation явно не гарантує довший lifetime.
- `FrameContext.artifacts` запис `canonical_frame` відображає provenance і
  payload semantics, але не переносить ownership heavy image buffer у context.
- Якщо payload reference більше не валідний, artifact має бути `MetadataOnly`
  або `ExpiredReference`.
- Майбутні conversion або binning routes мають позначати payload як
  `OwnedConverted` або `OwnedBinned` і потребують окремої StageSpec.
- Downstream stages мають читати `CanonicalFrame.image` як read-only, якщо
  окрема stage spec явно не передає owned mutable buffer.


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

### Володіння записами реєстру artifacts у `FrameContext`

`FrameContext.artifacts` є реєстром metadata/provenance для авторитетних
artifacts кадру. Запис у реєстрі не означає deep copy, transfer ownership або
подовження часу життя payload.

Payload, описаний записом реєстру, може належати:

- `FramePacket`, коли artifact описує raw input frame або view, валідний у
  межах input boundary.
- Явному stage output object, коли artifact описує результат, повернений через
  explicit output stage, наприклад `RadiometricFullFrameOutput`.
- Майбутньому external transport boundary, якщо окрема task/card явно визначить
  такий транспорт і його lifetime.
- Майбутньому tile-local context, якщо окрема task/card явно визначить
  tile-local ownership і merge boundary.

Канонічний vocabulary для `FrameContext` artifact ownership:

- `OwnedByFramePacket` - payload належить `FramePacket` або його input boundary;
  запис реєстру може тільки посилатися на цей payload або описувати його.
- `OwnedByStageOutput` - payload належить явному stage output object; запис
  реєстру фіксує identity, domain, producer і provenance цього output.
- `BorrowedReadOnly` - запис посилається на read-only borrowed view; consumers
  не мають права змінювати payload або використовувати його після завершення
  declared lifetime.
- `ExternalTransport` - payload належить зовнішньому transport/runtime boundary;
  запис реєстру має містити lifetime, дозволений цим boundary.
- `MetadataOnly` - запис не містить typed payload reference і лише описує факт
  існування artifact, producer, semantic name, domain, status або provenance.
- `ExpiredReference` - запис зберігається для audit/provenance, але referenced
  payload більше не є валідним для читання.

`FrameContext.artifacts` може володіти bounded metadata записів реєстру, але не
стає власником heavy payload за замовчуванням. Будь-яке context-owned володіння
small payload або bounded metadata має бути явно дозволене окремою
implementation task/card.

Запис реєстру не має подовжувати borrowed memory lifetime. Якщо lifetime
payload завершився, implementation має або видалити payload reference, або
позначити запис як `ExpiredReference`, залишивши тільки audit/provenance
metadata. Приховані mutable shared buffers через artifact registry заборонені.

## Interpretation

Ця policy оптимізує memory footprint і паралельність: raw frame зберігається
один раз, тайлові виконавці читають ROI views і пишуть тільки у власні reusable
buffers.

Для stateful temporal stages ощадлива до пам'яті доставка input не усуває
потребу в обмеженій algorithm-owned history. Ownership boundary має явно
розрізняти позичені current-frame views і збережені копії історії.

Для `FrameContext` це означає, що реєстр artifacts є audit/provenance шаром, а
не альтернативним data transport. Авторитетний artifact має бути відображений у
context, але payload залишається у фактичного власника, доки окрема task/card не
визначить інший bounded ownership contract.

## Failure cases

- `TileRawView` виконує deep copy raw ROI для кожного tile без stage spec.
- `CanonicalFrame` має borrowed payload, але artifact або structure позначає його як owned.
- Виконавець пише у shared processing buffer.
- `TileResult` переносить `cv::Mat` payload.
- Full-frame `F32` buffers створюються в tile route для кожного stage.
- `FrameContext` використовується як прихований owner output vectors.
- Stateful stage зберігає borrowed input/transport view після завершення
  часу життя source frame.
- Запис `FrameContext.artifacts` трактується як дозвіл на implicit deep copy
  heavy buffer у context.
- Запис `FrameContext.artifacts` подовжує borrowed lifetime або дозволяє читання
  payload після завершення input/stage/transport boundary.
- `BorrowedReadOnly` artifact використовується як mutable shared buffer між
  stages або tile workers.
- IPC/shared-memory ring buffer використовується як temporal history buffer для
  МКМФ.
- History МКМФ продовжує використовувати slots після зміни geometry, pixel
  format або route metadata без explicit reset/reallocation.

## Typical misuse

- Вважати `cv::Mat` ownership очевидним без domain contract.
- Зберігати non-owning tile view після завершення часу життя source frame.

## Open questions

- Точні C++ enum names для future DTO мають бути узгоджені з канонічним
  vocabulary цієї policy.
- Pooled allocation policy для `TileContext`.
- Interned-string або numeric-id policy для runtime metadata.

## Connections

- constrains: dp1.domain.raw.frame_packet
- constrains: dp1.domain.raw.canonical_frame
- constrains: dp1.domain.raw.tile_raw_view
- constrains: dp1.domain.processing.frame
- constrains: dp1.domain.processing.tile_processing_frame
- constrains: dp1.domain.mask.tile_binary_mask
- constrains: dp1.domain.runtime.tile_context
- constrains: dp1.domain.runtime.tile_result
- constrains: dp1.domain.runtime.cyclic_frame_buffer
- constrains: dp1.domain.runtime.frame_context
- constrains: dp1.stage_spec.radiometric_correction.inverse_median
- constrains: dp1.domain.opencv_invariants
