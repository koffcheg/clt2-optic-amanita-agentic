---
id: dp1.domain.raw.frame_packet
title: "Пакет кадру в Raw/Input домені DP1"
tags: [dp1, canonical, data-domain, raw, structure, frame]
kind: data-domain-card
source_role: canonical
source:
  file: "project-knowledge/02-dp1/canonical/data_domains/structures/raw/dp1.domain.raw.frame_packet.md"
status: "draft"
---

## Definition

`FramePacket` є canonical structure у Raw/Input domain для представлення кадру, що входить у DP1 pipeline.

Він відповідає на питання: що саме обробляється як вхідний кадр.

## Assumptions

- `cv::Mat` може бути storage carrier для image payload.
- Semantic meaning кадру визначається не лише carrier type, а також `pixel_format`, geometry, source metadata і route context.
- Фактичні поля C++ structure мають бути підтверджені окремою implementation task.
- Один `FramePacket` належить рівно одному camera/source input поточного DP1
  instance.

## Theorem / Contract

`FramePacket` має мінімально містити або посилатися на:

- `frame_id` — stable identifier кадру в межах pipeline run.
- `camera_id` — camera/source identifier для одного DP1 instance.
- `source_id` — optional stable source string, якщо numeric `camera_id` недостатній.
- `image` — full-frame raw image payload/storage carrier.
- `pixel_format` — canonical pixel carrier, defined by `dp1.domain.pixel_format`.
- `bit_depth` — фактична input бітність: `Bit8|Bit10|Bit12|Bit14|Bit16`.
- `pixel_range` — numeric range і camera level metadata.
- `geometry` — width, height, coordinate origin policy.
- `ingest_time` — timestamp отримання кадру у DP1.
- `acquisition_time` — optional camera/exposure timestamp.

`FramePacket` не має містити algorithm-specific outputs: masks, candidates, segments, measurements або tile-local working buffers.

`FramePacket` не є контейнером для кількох камер. Multi-camera scenario має
бути представлений кількома DP1 instances або downstream aggregation boundary.
DP1 stages обробляють один `FramePacket` як один frame/source tuple.

Рекомендована C++ форма:

```cpp
struct FramePacket {
    std::uint64_t frame_id = 0;
    int camera_id = -1;
    std::string source_id;
    cv::Mat image;
    PixelFormat pixel_format = PixelFormat::U16;
    InputBitDepth bit_depth = InputBitDepth::Bit16;
    PixelRange pixel_range;
    FrameGeometry geometry;
    TimestampRef ingest_time;
    std::optional<TimestampRef> acquisition_time;
};
```

## Поля

```yaml
fields:
  - name: "`frame_id`"
    type: "`std::uint64_t`"
    purpose: "Стабільний id кадру в межах DP1 instance."
    used_for: "Зв'язує `TileDesc`, `TileResult`, `MeasurementRecord`, logs."
    memory: "8 B"
  - name: "`camera_id`"
    type: "`int`"
    purpose: "Ідентифікатор камери/DP1 instance."
    used_for: "DP2 handoff, multi-instance diagnostics."
    memory: "4 B"
  - name: "`source_id`"
    type: "`std::string`"
    purpose: "Optional текстовий id source."
    used_for: "Logs, source routing, reproducibility."
    memory: "~24 B + payload"
  - name: "`image`"
    type: "`cv::Mat`"
    purpose: "Full-frame raw carrier."
    used_for: "ROI views для `TileRawView`; raw photometry reference."
    memory: "header ~96 B + full-frame payload"
  - name: "`pixel_format`"
    type: "`PixelFormat`"
    purpose: "Storage carrier: `U8` або `U16`."
    used_for: "Route validation, algorithm implementation selection."
    memory: "4 B"
  - name: "`bit_depth`"
    type: "`InputBitDepth`"
    purpose: "Фактична бітність сенсора."
    used_for: "Threshold scaling, residual range, photometry."
    memory: "4 B"
  - name: "`pixel_range`"
    type: "`PixelRange`"
    purpose: "Min/max/black/saturation metadata."
    used_for: "Conversion, normalization, quality flags."
    memory: "~32 B"
  - name: "`geometry`"
    type: "`FrameGeometry`"
    purpose: "Розмір і origin кадру."
    used_for: "Tile grid, ROI validation, coordinate transforms."
    memory: "~16 B"
  - name: "`ingest_time`"
    type: "`TimestampRef`"
    purpose: "Час прийому кадру в DP1."
    used_for: "Latency/profiling."
    memory: "~16 B"
  - name: "`acquisition_time`"
    type: "optional `TimestampRef`"
    purpose: "Час експозиції з камери, якщо доступний."
    used_for: "DP2 timeline, synchronization."
    memory: "~24 B"
```

## Пам'ять

`FramePacket.image` є full-frame raw payload незалежно від `prep.variant`.
Для `1440x1080`:

- `U8`: приблизно 1.6 MB;
- `U16`: приблизно 3.1 MB.

Для `prep.variant = "tiles"` tile workers не копіюють `FramePacket.image`; вони
створюють `TileRawView` як ROI view. Для `full_frame`, `roi` і `adaptive_roi`
memory policy має бути задана відповідною stage spec. Якщо camera input queue
зберігає кілька кадрів, її depth має бути bounded у configuration `C`.

## Етапи

- Camera/source layer створює `FramePacket`.
- `prep` читає `FramePacket` і вибирає route: `full_frame`, `roi`, `tiles` або
  `adaptive_roi`.
- Для `prep.variant = "tiles"` runtime створює `TileRawView` для кожного tile.
- `measurement` може читати raw ROI для photometry, але не мутує `FramePacket`.

## Interpretation

`FramePacket` є boundary object для raw або near-raw frame payload. Пізніші processing representations мають використовувати `dp1.domain.processing.frame`, якщо їх semantics відрізняються від input/acquisition frame.

## Failure cases

- Stage записує algorithm output назад у `FramePacket` metadata.
- Один `FramePacket` змішує image payload або metadata кількох камер.
- `frame_id` змінюється між stages без явної remapping policy.
- Pixel format виводиться лише з `cv::Mat::type()` без canonical metadata.

## Typical misuse

- Використовувати `FramePacket` як універсальний контейнер для всіх результатів pipeline.
- Ховати runtime state або profiling у `FramePacket` замість `FrameContext`.

## Open questions

- Чи `frame_id` має бути глобальним або per-source/per-run.
- Exact C++ ownership model for image payload.

## Connections

- belongs_to: dp1.domain.raw
- uses: dp1.domain.pixel_format
- constrained_by: dp1.domain.identity
- constrained_by: dp1.domain.time
- constrained_by: dp1.domain.memory_ownership
- constrained_by: dp1.domain.coordinates
- accompanied_by: dp1.domain.runtime.frame_context
- may_produce: dp1.domain.processing.frame
