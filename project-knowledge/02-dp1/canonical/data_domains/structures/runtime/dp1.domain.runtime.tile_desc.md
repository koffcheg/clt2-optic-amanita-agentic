---
id: dp1.domain.runtime.tile_desc
title: "Опис tile/ROI для DP1"
tags: [dp1, canonical, data-domain, runtime, tile, parallelism]
kind: data-domain-card
source_role: canonical
source:
  file: "project-knowledge/02-dp1/canonical/data_domains/structures/runtime/dp1.domain.runtime.tile_desc.md"
status: "draft"
---

## Definition

`TileDesc` є lightweight runtime structure для опису tile/ROI, який може оброблятися локально або паралельно.

Це не image buffer і не копія кадру.

## Assumptions

- Tile processing є target execution model, а не claim про поточну реалізацію.
- ROI може бути представлений як OpenCV `cv::Rect` або equivalent geometry в implementation.
- Конкретна parallel scheduler / thread-pool implementation не входить у scope цієї картки.

## Theorem / Contract

`TileDesc` має мінімально містити або посилатися на:

- `tile_id` — stable tile identifier у межах кадру.
- `frame_id` — ідентифікатор source frame.
- `roi` — valid tile area у глобальних координатах кадру.
- `roi_with_border` — processing area з урахуванням overlap/border.
- `valid_area` — crop area, результати якої вважаються валідними після border-dependent operations.
- `origin_px` — tile origin у глобальних координатах кадру.
- optional `border_policy` — required overlap semantics для filters/morphology.

Coordinate rules визначає `dp1.domain.coordinates`. Ця structure лише несе
поля `roi`, `roi_with_border`, `valid_area` і `origin_px`, потрібні для
застосування canonical formulas.

Семантика координат навмисно змішана в одному `TileDesc`:

- `roi_with_border` задається у глобальних координатах кадру.
- `origin_px`/`origin_in_frame` задається у глобальних координатах кадру і
  зазвичай дорівнює `roi_with_border.tl()`.
- `valid_area` задається у локальних координатах тайла відносно
  `roi_with_border.tl()`.
- `coordinate_space = TileLocal` означає, що локальні вихідні дані тайла можна
  перевести у глобальні координати через дескриптор; це не означає, що кожен
  прямокутник, збережений у `TileDesc`, є локальним для тайла.

Ця змішана семантика прямокутників є навмисною. Реалізація виконання тайла
Stage2 має читати вхідні пікселі через глобальний для кадру
`roi_with_border` і обрізати або приймати локальні вихідні дані тайла через
`valid_area` перед переведенням у глобальні координати.


`TileDesc` має бути cheap to copy/pass між workers.

Рекомендована C++ форма:

```cpp
struct TileDesc {
    int tile_id = -1;
    std::uint64_t frame_id = 0;
    cv::Rect roi;
    cv::Rect roi_with_border;
    cv::Rect valid_area;
    cv::Point origin_px{0, 0};
    BorderPolicy border_policy;
};
```

## Поля

```yaml
fields:
  - name: "`tile_id`"
    type: "`int`"
    purpose: "Стабільний id tile."
    used_for: "Work scheduling, result ordering."
    memory: "4 B"
  - name: "`frame_id`"
    type: "`std::uint64_t`"
    purpose: "Source frame identity."
    used_for: "Validation, merge trace."
    memory: "8 B"
  - name: "`roi`"
    type: "`cv::Rect`"
    purpose: "Основна область tile у frame-global coordinates."
    used_for: "Ownership of valid output area."
    memory: "16 B"
  - name: "`roi_with_border`"
    type: "`cv::Rect`"
    purpose: "Область читання з overlap/border."
    used_for: "Filters, morphology, neighborhood operations."
    memory: "16 B"
  - name: "`valid_area`"
    type: "`cv::Rect`"
    purpose: "Border-safe область прийняття result."
    used_for: "Crop candidates/segments/measurements."
    memory: "16 B"
  - name: "`origin_px`"
    type: "`cv::Point`"
    purpose: "Local-to-global offset."
    used_for: "Coordinate transform."
    memory: "8 B"
  - name: "`border_policy`"
    type: "`BorderPolicy`"
    purpose: "Правила overlap і edge behavior."
    used_for: "Stage correctness near tile borders."
    memory: "implementation-specific"
```

## Пам'ять

`TileDesc` не містить image payload. Орієнтовний розмір — 80-100 B. Навіть
сотні tiles коштують лише десятки KB.

## Етапи

- `prep` створює `TileDesc[]`.
- Scheduler роздає `TileDesc` workers.
- Worker створює `TileRawView` через `roi_with_border`.
- `merge` використовує `valid_area` і `origin_px`.

## Interpretation

Очікуваний tile execution pattern:

```text
build TileDesc list from FramePacket
parallel worker reads source frame ROI using TileDesc
worker produces TileResult
merge TileResult objects into global frame coordinates
```

## Failure cases

- Tile копіюється як image buffer замість опису через ROI metadata.
- Border/overlap не враховано для filters або morphology.
- Local coordinates видаються як global coordinates без transform.
- `valid_area` записаний у frame-global coordinates і потім повторно
  globalization зміщує output.

## Typical misuse

- Трактувати `TileDesc` як owner image memory.
- Використовувати tile-local output без valid-area crop.

## Open questions

- Standard tile size і overlap policy.
- Чи tile grid є static або config-driven.
- Exact boundary behavior на краях кадру.
- Exact duplicate suppression policy після merge.

## Connections

- belongs_to: dp1.domain.runtime
- describes_view_of: dp1.domain.raw.frame_packet
- constrained_by: dp1.domain.common_types
- constrained_by: dp1.domain.opencv_invariants
- constrained_by: dp1.domain.coordinates
- used_by: dp1.domain.runtime.tile_context
- produces_scope_for: dp1.domain.runtime.tile_result
