---
id: dp1.stage.prep
title:
  uk: "Етап підготовки DP1"
  en: "DP1 Prep stage"
tags: [dp1, canonical, stage]
kind: stage-interface-card
source_role: canonical
source:
  file: "project-knowledge/02-dp1/canonical/stages/dp1.stage.prep.md"
  lines: "1-137"
status: "draft"
---

## Definition

Готує ROI, tiles, borders, нормалізацію типів і перетворення координат.

## Interface

`IPrepStage`.

## Contract

`process(input, context, config) -> output`.

## Algorithmic idea

Організувати обробку кадру без втрати геометрії: вибрати full-frame або ROI,
розбити дані на tiles за потреби, додати border/overlap і зберегти metadata
для переходів `local <-> global`.

## Inputs

Домен сирих даних `Raw`; фрагмент конфігурації для політики ROI, tile,
overlap і border.

## Internal computation domain

`Raw` або `Processing`, явно визначений майбутньою специфікацією етапу.
Допустимі формати: `CV_16UC1`; `CV_8UC1` лише для явно задекларованого
швидкого тракту.

## Outputs

Підготовлене представлення кадру/ROI/tiles і metadata для перетворення
координат.

## Domain bindings

```yaml
frame_level_binding:
  allowed_input: "dp1.domain.raw.frame_packet"
  runtime_context: "dp1.domain.runtime.frame_context"
  allowed_output:
    - "dp1.domain.raw.frame_packet"
    - "dp1.domain.processing.frame"
  notes: "Підготовка source data та ROI/tile route. Без candidates, masks або measurements."
route_specific_carriers:
  - route: "full_frame"
    input_carrier: "FramePacket"
    output_carrier: "FramePacket або ProcessingFrame"
  - route: "roi"
    input_carrier: "FramePacket + ROI metadata"
    output_carrier: "ROI-scoped raw/processing representation, якщо це визначено stage spec"
  - route: "tiles"
    input_carrier: "FramePacket"
    output_carrier: "TileDesc[]"
  - route: "adaptive_roi"
    input_carrier: "FramePacket + explicit ROI source/state"
    output_carrier: "ROI metadata або route-specific processing units, якщо це визначено stage spec"
```

`prep.variant = "tiles"` будує `TileDesc[]`, border/overlap і valid area.
`TileDesc` є execution metadata, а не image buffer і не stage output для
детекції. `Prep` не має створювати `BinaryMask`, `Candidate`,
`ValidatedObject` або `MeasurementRecord`.

## Complexity variants

- `L0`: full-frame або ROI без tiles, мінімум копій.
- `L1`: tiles з overlap і явним обліком координат.
- `L2`: multi-scale, адаптивні ROI, складна схема tiles.

## OpenCV mapping

- `cv::Rect` ROI: `native`.
- `resize`: `native`, якщо обрано multi-scale режим.
- Tiling і border policy: `custom`.

## Config fragment

Ключ DSL: `prep`.

Обов’язкові поля: `enabled`, `variant`, `level`, `parameters`.

Допустимі `variant`: `full_frame`, `roi`, `tiles`, `adaptive_roi`.

`variant` має бути зареєстрований у `dp1.config.stage_variant_registry`.

## Timing / profiling

Профілювати час підготовки, кількість tiles, витрати на копіювання, витрати на
border/overlap і перетворення форматів.

## Must not do

Виявлення, вимірювання або обчислення на основі візуалізації.

## Constraints

Не допускаються неявні перетворення типів. Динамічний діапазон raw має зберігатися відповідно до правил перетворення.

Критичні інваріанти:
- відсутність зайвих копій;
- коректна геометрія координат;
- traceability від локальних координат tiles до глобальних координат кадру.
- `prep.variant = "tiles"` має відповідати `dp1.pipeline.stage_io_matrix` і
  `dp1.domain.coordinates`.

## Failure cases

Некоректний ROI, неузгоджене перетворення координат, втрата даних на межах tile.

## Typical misuse

Трактувати політику tiling як детектор.

## Open questions

- Exact duplicate suppression policy після tile merge.

## Connections

- uses: dp1.domain.raw
- uses: dp1.domain.raw.frame_packet
- uses: dp1.domain.runtime.frame_context
- may_produce: dp1.domain.processing.frame
- may_produce: dp1.domain.runtime.tile_desc
- uses: dp1.domain.conversion_rules
- constrained_by: dp1.config.stage_variant_registry
- constrained_by: dp1.pipeline.stage_io_matrix
- feeds: dp1.stage.radiometric_correction
