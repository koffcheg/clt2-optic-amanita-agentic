---
id: dp1.stage.enhancement
title:
  uk: "Етап підсилення DP1"
  en: "DP1 Enhancement stage"
tags: [dp1, canonical, stage]
kind: stage-interface-card
source_role: canonical
source:
  file: "project-knowledge/02-dp1/canonical/stages/dp1.stage.enhancement.md"
  lines: "1-128"
status: "draft"
---

## Definition

Підвищує SNR, згладжує шум, підсилює локальні структури і стабілізує сигнал перед виявленням.

## Interface

`IEnhancementStage`.

## Contract

`process(input, context, config) -> output`.

## Algorithmic idea

Покращити співвідношення сигнал/шум перед детекцією без прийняття рішення про
об’єкт і без змішування з matched filtering.

## Inputs

Домен обробки.

## Internal computation domain

Домен обробки. Рекомендований внутрішній формат: `CV_32FC1`.

## Outputs

Підсилений домен обробки: `CV_32FC1` або явно задекларований `CV_8UC1`.

## Domain bindings

```yaml
frame_level_binding:
  allowed_input: "dp1.domain.processing.frame"
  runtime_context: "dp1.domain.runtime.frame_context"
  allowed_output: "dp1.domain.processing.frame"
  notes: "Покращує processing representation."
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

Етап працює з processing payload, а не напряму з raw frame. Він не має
створювати `BinaryMask`, `Candidate`, `ValidatedObject` або
`MeasurementRecord`; raw input може з'являтися тільки через explicit stage spec.

## Complexity variants

- `L0`: Gaussian 3x3.
- `L1`: Gaussian 5x5 або DoG.
- `L2`: bilateral, guided або multi-scale.

## OpenCV mapping

- `GaussianBlur`: `native`.
- DoG: `composed`.
- `bilateralFilter`: `native`.
- guided або multi-scale: `wrapped` або `custom`, залежно від майбутньої
  специфікації.

## Config fragment

Ключ DSL: `enhancement`.

Обов’язкові поля: `enabled`, `variant`, `level`, `parameters`.

Допустимі `variant`: `gaussian`, `dog`, `bilateral`, `guided`, `multi_scale`.

## Timing / profiling

Профілювати час фільтрації, розмір ядра, кількість проходів, витрати на
перетворення форматів і, якщо повертаються статистики, вартість їх обчислення.

## Must not do

Фінальне рішення про виявлення або вихід вимірювань.

## Constraints

Має зберігати інформацію, потрібну наступним етапам детектора і вимірювання.

Критичні інваріанти:
- не змішувати з matched filtering;
- не створювати binary mask;
- зберігати сигнал, потрібний для detector response і photometry.

## Failure cases

Згладжування сигналу, створення штучних структур, нестабільний відгук між кадрами.

## Typical misuse

Трактувати результат denoising як binary mask.

## Open questions

Метрики якості для покращення SNR.

## Connections

- feeds: dp1.stage.matched_filtering
- uses: dp1.domain.processing
- uses: dp1.domain.processing.frame
- uses: dp1.domain.processing.tile_processing_frame
- uses: dp1.domain.runtime.frame_context
- uses: dp1.domain.runtime.tile_context
