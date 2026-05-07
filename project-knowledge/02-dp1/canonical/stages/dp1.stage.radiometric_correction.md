---
id: dp1.stage.radiometric_correction
title:
  uk: "Етап радіометричної корекції DP1"
  en: "DP1 Radiometric correction stage"
tags: [dp1, canonical, stage]
kind: stage-interface-card
source_role: canonical
source:
  file: "project-knowledge/02-dp1/canonical/stages/dp1.stage.radiometric_correction.md"
  lines: "1-170"
status: "draft"
---

## Definition

Формує background/residual, компенсує освітлення, приглушує низькочастотну складову і будує модель фону.

## Interface

`IRadiometricStage`.

## Contract

`process(input, context, config) -> output`.

## Algorithmic idea

Оцінити background, відокремити його від корисного сигналу і сформувати
residual у processing domain.

## Inputs

Semantic input належить до raw-like або processing representation після
`prep`. Concrete carrier залежить від `prep.variant` і має бути явно
задекларований route/config:

| Route | Дозволений input carrier |
|---|---|
| `full_frame` | `FramePacket` або `ProcessingFrame` |
| `roi` | ROI/view над `FramePacket` або `ProcessingFrame` |
| `tiles` | `TileRawView` або `TileProcessingFrame` |

## Internal computation domain

Домен обробки. Рекомендований формат: `CV_32FC1`; `CV_8UC1` дозволений лише
для явно задекларованого швидкого режиму.

## Outputs

Вихід завжди належить Processing domain. Concrete carrier залежить від route:

| Route | Output carrier |
|---|---|
| `full_frame` | `ProcessingFrame` |
| `roi` | `ProcessingFrame` або ROI-scoped processing representation, якщо це визначено stage spec |
| `tiles` | `TileProcessingFrame` |

Output має явно задавати:

- `pixel_format` (`S16`, `S32`, `F32`, `U8` або `U16` відповідно до route);
- `processing_domain` (`RadiometricResidual` або `RadiometricCorrected`);
- `range_policy`;
- `value_range`;
- `coordinate_space`.

## Domain bindings

```yaml
frame_level_binding:
  allowed_input:
    - "dp1.domain.raw.frame_packet"
    - "dp1.domain.processing.frame"
  runtime_context: "dp1.domain.runtime.frame_context"
  allowed_output: "dp1.domain.processing.frame"
  notes: "Формує corrected/residual processing representation; concrete carrier залежить від route."
route_specific_carriers:
  - route: "full_frame"
    input_carrier: "FramePacket або ProcessingFrame"
    output_carrier: "ProcessingFrame"
  - route: "roi"
    input_carrier: "ROI/view над FramePacket або ProcessingFrame"
    output_carrier: "ProcessingFrame або ROI-scoped processing representation, якщо це визначено stage spec"
  - route: "tiles"
    input_carrier: "TileRawView або TileProcessingFrame"
    runtime_context: "TileContext + FrameContext"
    output_carrier: "TileProcessingFrame"
```

Вихід має бути explicit Processing-domain carrier з
`processing_domain = RadiometricResidual` або `RadiometricCorrected`.
Етап не має мутувати `FramePacket.image`, видавати candidates/masks/measurements
або перетворювати internal background buffers на canonical output.

## Complexity variants

- `L0`: mean або Gaussian subtraction.
- `L1`: inverse median як robust-варіант.
- `L2`: adaptive background, band-pass або per-tile background.

`inverse_median` конкретизується у
`dp1.stage_spec.radiometric_correction.inverse_median`.

## OpenCV mapping

- `blur`: `native`.
- `GaussianBlur`: `native`.
- `medianBlur`: `native`.
- inverse median: `wrapped` або `custom`.

## Config fragment

Ключ DSL: `radiometric`.

Обов’язкові поля: `enabled`, `variant`, `level`, `parameters`.

Допустимі `variant`: `mean_subtraction`, `gaussian_subtraction`,
`inverse_median`, `adaptive_background`, `band_pass`, `per_tile_background`.

## Timing / profiling

Профілювати час оцінки background, час формування residual, витрати на
перетворення форматів і вартість per-tile/stateful моделей.

## State ownership

Якщо використовується adaptive або stateful background, власник стану має бути
задекларований у специфікації етапу і конфігурації `C`.

Для `inverse_median` state ownership, lifecycle циклічного буфера, розрядність
residual, режими приведення виходу та вимоги валідації визначені у
`dp1.stage_spec.radiometric_correction.inverse_median`.

## Must not do

Прийняття кандидатів, фільтрація об’єктів або фінальне вимірювання.

## Constraints

Фотометричні операції потребують задекларованого домену обчислень без втрати даних, якщо `C` явно не вибирає швидкий режим.

Критичні інваріанти:
- background model і residual є різними сутностями;
- етап видає explicit Processing-domain carrier, а не мутує raw input;
- фотометрично значущі операції не виконуються непомітно на `CV_8U`.

## Failure cases

Дрейф моделі фону, втрата динамічного діапазону, надмірне приглушення низьких частот.

## Typical misuse

Кодувати корекцію фону як налаштування візуалізації.

## Open questions

Володіння станом моделі фону.

## Connections

- uses: dp1.domain.processing
- uses: dp1.domain.raw.frame_packet
- uses: dp1.domain.raw.tile_raw_view
- uses: dp1.domain.processing.frame
- uses: dp1.domain.processing.tile_processing_frame
- uses: dp1.domain.runtime.frame_context
- uses: dp1.domain.runtime.tile_context
- constrained_by: dp1.domain.conversion_rules
- specified_by: dp1.stage_spec.radiometric_correction.inverse_median
- feeds: dp1.stage.enhancement
