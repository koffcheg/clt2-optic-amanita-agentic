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
  lines: "1-130"
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

Домен обробки після підготовки.

## Internal computation domain

Домен обробки. Рекомендований формат: `CV_32FC1`; `CV_8UC1` дозволений лише
для явно задекларованого швидкого режиму.

## Outputs

Residual або скоригований домен обробки: `CV_32FC1` або явно задекларований
`CV_8UC1`.

## Complexity variants

- `L0`: mean або Gaussian subtraction.
- `L1`: inverse median як robust-варіант.
- `L2`: adaptive background, band-pass або per-tile background.

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

## Must not do

Прийняття кандидатів, фільтрація об’єктів або фінальне вимірювання.

## Constraints

Фотометричні операції потребують задекларованого домену обчислень без втрати даних, якщо `C` явно не вибирає швидкий режим.

Критичні інваріанти:
- background model і residual є різними сутностями;
- етап працює у processing domain;
- фотометрично значущі операції не виконуються непомітно на `CV_8U`.

## Failure cases

Дрейф моделі фону, втрата динамічного діапазону, надмірне приглушення низьких частот.

## Typical misuse

Кодувати корекцію фону як налаштування візуалізації.

## Open questions

Володіння станом моделі фону.

## Connections

- uses: dp1.domain.processing
- constrained_by: dp1.domain.conversion_rules
- feeds: dp1.stage.enhancement_denoising
