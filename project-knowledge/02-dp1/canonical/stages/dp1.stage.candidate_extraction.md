---
id: dp1.stage.candidate_extraction
title:
  uk: "Етап виділення кандидатів DP1"
  en: "DP1 Candidate extraction stage"
tags: [dp1, canonical, stage]
kind: stage-interface-card
source_role: canonical
source:
  file: "project-knowledge/02-dp1/canonical/stages/dp1.stage.candidate_extraction.md"
  lines: "1-146"
status: "draft"
---

## Definition

Формує гіпотези, будує binary mask і виконує первинне виділення кандидатів.

## Interface

`ICandidateExtractionStage`.

## Contract

`process(input, context, config) -> output`.

## Algorithmic idea

Перетворити detector response або різницеве представлення на гіпотези
кандидатів через thresholding, adaptive thresholding або stateful background
model.

## Inputs

Карта відгуку детектора в домені обробки.

## Internal computation domain

Домени обробки та масок. Допустимий input: `CV_32FC1` або явно
задекларований `CV_8UC1`.

## Outputs

Домен масок `CV_8UC1` і структуровані гіпотези кандидатів.

## Domain bindings

```yaml
frame_level_binding:
  allowed_input: "dp1.domain.processing.frame"
  runtime_context: "dp1.domain.runtime.frame_context"
  allowed_output:
    - "dp1.domain.mask.binary_mask"
    - "dp1.domain.struct.candidate"
  notes: "Candidate є provisional, а не validated object."
route_specific_carriers:
  - route: "full_frame"
    input_carrier: "ProcessingFrame"
    output_carrier: "BinaryMask + Candidate[]"
  - route: "roi"
    input_carrier: "ROI-scoped processing representation, якщо це визначено stage spec"
    output_carrier: "ROI-scoped BinaryMask + Candidate[] з explicit coordinate metadata"
  - route: "tiles"
    input_carrier: "TileProcessingFrame"
    runtime_context: "TileContext + FrameContext"
    output_carrier: "TileBinaryMask + Candidate[]"
```

`BinaryMask` не є photometry source. `Candidate` є гіпотезою, а не
`ValidatedObject`; етап не має видавати `MeasurementRecord` або виконувати
downstream validation/classification.

## Complexity variants

- `L0`: global threshold.
- `L1`: adaptive threshold.
- `L2`: stateful background model, наприклад MOG2/KNN.

## OpenCV mapping

- `threshold`: `native`.
- `adaptiveThreshold`: `native`.
- `absdiff`: `native`.
- MOG2/KNN: `native` із явним контролем stateful-моделі.

## Config fragment

Ключ DSL: `candidate_extraction`.

Обов’язкові поля: `enabled`, `variant`, `level`, `parameters`.

Допустимі `variant`: `global_threshold`, `adaptive_threshold`, `absdiff`,
`mog2`, `knn`.

## Timing / profiling

Профілювати час thresholding/background update, кількість кандидатів, витрати
на перетворення response map у mask і вартість stateful-моделі.

Profiling records мають використовувати
`StageKey = "candidate_extraction"` і структури з `dp1.domain.profiling`.
Candidate count, threshold mode, response-to-mask conversion, stateful update
cost і copy/conversion metrics мають бути recorded або aggregated.

## State ownership

MOG2/KNN або інші stateful background models мають мати явного власника стану,
правила reset/update і облік часу оновлення.

`variant` має бути зареєстрований у `dp1.config.stage_variant_registry`.

## Must not do

Фінальна валідація об’єктів або downstream-вимірювання.

## Constraints

Правила thresholding мають бути явними у майбутній специфікації етапу.

Критичні інваріанти:
- mask stability має контролюватися;
- гіпотези не є валідованими об’єктами;
- stateful-моделі не допускаються як прихований швидкий шлях.


## Stage specification

- Pilot small-TZ: `../stage_specs/dp1.stage_spec.candidate_extraction.pilot.md`.

## Failure cases

Нестабільність порогу, фрагментація кандидатів, пропуск слабких сигналів.

## Typical misuse

Трактувати гіпотези як валідовані об’єкти.

## Open questions

Схема гіпотези кандидата.

## Connections

- uses: dp1.domain.mask
- uses: dp1.domain.processing.frame
- uses: dp1.domain.processing.tile_processing_frame
- uses: dp1.domain.mask.binary_mask
- uses: dp1.domain.mask.tile_binary_mask
- produces: dp1.domain.struct.candidate
- uses: dp1.domain.runtime.frame_context
- uses: dp1.domain.runtime.tile_context
- uses: dp1.domain.profiling
- feeds: dp1.stage.segmentation_refinement
- constrained_by: dp1.domain.conversion_rules
