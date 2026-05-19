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

Готує processing geometry для `CanonicalFrame`: full-frame, ROI, tiles, border/overlap, valid area і metadata для local/global coordinate mapping.

## Interface

`IPrepStage`.

## Contract

`process(input, context, config) -> output`.

## Algorithmic idea

Організувати просторову схему обробки `CanonicalFrame` без втрати геометрії: вибрати full-frame або ROI,
розбити дані на tiles за потреби, додати border/overlap і зберегти metadata
для переходів `local <-> global`.

## Inputs

`CanonicalFrame`; фрагмент конфігурації для політики ROI, tile,
overlap і border.

## Internal computation domain

`Raw` або route-specific metadata domain, явно визначений майбутньою специфікацією етапу.
Prep не виконує pixel-format normalization, bit-depth conversion або binning.

## Outputs

Processing layout: full-frame/ROI/tile descriptors і metadata для перетворення
координат.

## Domain bindings

```yaml
frame_level_binding:
  allowed_input: "dp1.domain.raw.canonical_frame"
  runtime_context: "dp1.domain.runtime.frame_context"
  allowed_output:
    - "dp1.domain.raw.canonical_frame"
    - "dp1.domain.runtime.tile_desc"
  notes: "Підготовка processing geometry для CanonicalFrame. Без binning, conversion, candidates, masks або measurements."
route_specific_carriers:
  - route: "full_frame"
    input_carrier: "CanonicalFrame"
    output_carrier: "CanonicalFrame view або route-specific ProcessingLayout"
  - route: "roi"
    input_carrier: "CanonicalFrame + ROI metadata"
    output_carrier: "ROI-scoped raw/processing representation, якщо це визначено stage spec"
  - route: "tiles"
    input_carrier: "CanonicalFrame"
    output_carrier: "TileDesc[]"
  - route: "adaptive_roi"
    input_carrier: "CanonicalFrame + explicit ROI source/state"
    output_carrier: "ROI metadata або route-specific processing units, якщо це визначено stage spec"
```

`prep.variant = "tiles"` будує `TileDesc[]`, border/overlap і valid area.
`TileDesc` є execution metadata, а не image buffer і не stage output для
детекції. `Prep` не має створювати `BinaryMask`, `Candidate`,
`ValidatedObject` або `MeasurementRecord`.

## Complexity variants

- `L0`: full-frame або ROI без tiles, мінімум копій.
- `L1`: tiles з overlap і явним обліком координат.
- `L2`: адаптивні ROI або складна схема tiles за окремою stage spec.

## OpenCV mapping

- `cv::Rect` ROI: `native`.
- `resize`: заборонено для Prep, якщо майбутня погоджена stage spec явно не винесе spatial resampling в окремий non-normalization route.
- Tiling і border policy: `custom`.

## Config fragment

Ключ DSL: `prep`.

Обов’язкові поля: `enabled`, `variant`, `level`, `parameters`.

Допустимі `variant`: `full_frame`, `roi`, `tiles`, `adaptive_roi`.

`variant` має бути зареєстрований у `dp1.config.stage_variant_registry`.

## Timing / profiling

Профілювати час підготовки, кількість tiles, витрати на копіювання і витрати на
border/overlap. Перетворення форматів не належить Prep.

Profiling records мають використовувати `StageKey = "prep"` і структури з
`dp1.domain.profiling`: `StageTiming`, `OperationTiming`, `CardinalityMetrics`
і `MemoryMetrics`. Tile/ROI metrics мають бути frame-scoped або явно
tile-scoped.

## Must not do

Виявлення, вимірювання, pixel normalization, bit-depth conversion, binning або обчислення на основі візуалізації.

## Constraints

Не допускаються неявні перетворення типів. Динамічний діапазон `CanonicalFrame` має зберігатися; будь-яка normalization/conversion належить Stage0 або окремій stage spec.

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
- uses: dp1.domain.raw.canonical_frame
- uses: dp1.domain.runtime.frame_context
- uses: dp1.domain.profiling
- may_produce: dp1.domain.runtime.tile_desc
- constrained_by: dp1.config.stage_variant_registry
- constrained_by: dp1.pipeline.stage_io_matrix
- feeds: dp1.stage.radiometric_correction
