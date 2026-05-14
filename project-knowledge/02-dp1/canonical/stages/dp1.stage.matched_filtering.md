---
id: dp1.stage.matched_filtering
title:
  uk: "Етап узгодженої фільтрації як детектора DP1"
  en: "DP1 Matched filtering detector stage"
tags: [dp1, canonical, stage]
kind: stage-interface-card
source_role: canonical
source:
  file: "project-knowledge/02-dp1/canonical/stages/dp1.stage.matched_filtering.md"
  lines: "1-129"
status: "draft"
---

## Definition

Виявляє сигнал через підсилення структур, узгоджених із PSF/template, і формує карту відгуку.

## Interface

`IMatchedFilterStage`.

## Contract

`process(input, context, config) -> output`.

## Algorithmic idea

Побудувати detector response для сигналу, узгодженого з PSF або шаблоном.
Це detector stage, а не загальний етап фільтрації зображення.

## Inputs

Домен обробки після підсилення і denoising.

## Internal computation domain

Домен обробки. Рекомендований формат: `CV_32FC1`.

## Outputs

Карта відгуку детектора у домені обробки: `CV_32FC1`.

## Domain bindings

```yaml
frame_level_binding:
  allowed_input: "dp1.domain.processing.frame"
  runtime_context: "dp1.domain.runtime.frame_context"
  allowed_output: "dp1.domain.processing.frame"
  notes: "Detector/response representation лишається в Processing domain."
route_specific_carriers:
  - route: "full_frame"
    input_carrier: "ProcessingFrame"
    output_carrier: "ProcessingFrame"
  - route: "roi"
    input_carrier: "ROI-scoped processing representation, якщо це визначено stage spec"
    output_carrier: "ROI-scoped processing representation, якщо це визначено stage spec"
  - route: "tiles"
    input_carrier: "TileProcessingFrame"
    runtime_context: "TileContext + FrameContext"
    output_carrier: "TileProcessingFrame"
```

Вихід має зберігати detector-response semantics, зокрема
`processing_domain = DetectorResponse`. Карта відгуку не є `BinaryMask`; етап не
має виконувати thresholding або candidate extraction.

## Complexity variants

- `L0`: Gaussian quasi-matched response.
- `L1`: `filter2D` із фіксованим ядром.
- `L2`: templates, `matchTemplate` або adaptive kernels.
- `L3`: PSF-fit або інші research/custom варіанти.

## OpenCV mapping

- `GaussianBlur`: `native`.
- `filter2D`: `native`.
- `matchTemplate`: `native`.
- PSF-fit: `custom`.

## Config fragment

Ключ DSL: `matched_filter`.

Обов’язкові поля: `enabled`, `variant`, `level`, `parameters`.

Допустимі `variant`: `gaussian`, `kernel`, `template`, `adaptive_kernel`,
`psf_fit`.

`variant` має бути зареєстрований у `dp1.config.stage_variant_registry`.

## Timing / profiling

Профілювати час побудови response map, розмір ядра/шаблону, кількість проходів
і витрати на перетворення форматів.

Profiling records мають використовувати `StageKey = "matched_filtering"` і
структури з `dp1.domain.profiling`. Kernel/template size, response-map
dimensions, pass count, conversions і copies мають бути explicit metrics.

## Must not do

Цей етап не можна трактувати як загальний фільтр зображення. Це етап детектора, а не просто фільтр.

## Constraints

Припущення щодо template/PSF мають бути явними у майбутній специфікації етапу.

Критичні інваріанти:
- detector semantics не можна втрачати;
- matched filtering не змішується з denoising;
- response map не є binary mask.

## Failure cases

Підсилення хибного відгуку, невідповідність template, насичення відгуку.

## Typical misuse

Використовувати вихід фільтра без семантики детектора.

## Open questions

Canonical-модель параметрів PSF/template.

## Connections

- feeds: dp1.stage.candidate_extraction
- uses: dp1.domain.processing
- uses: dp1.domain.processing.frame
- uses: dp1.domain.processing.tile_processing_frame
- uses: dp1.domain.runtime.frame_context
- uses: dp1.domain.runtime.tile_context
- uses: dp1.domain.profiling
