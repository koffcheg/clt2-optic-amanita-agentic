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
  lines: "1-130"
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

## Failure cases

Некоректний ROI, неузгоджене перетворення координат, втрата даних на межах tile.

## Typical misuse

Трактувати політику tiling як детектор.

## Open questions

Canonical-схема перетворення координат.

## Connections

- uses: dp1.domain.raw
- uses: dp1.domain.conversion_rules
- feeds: dp1.stage.radiometric_correction
