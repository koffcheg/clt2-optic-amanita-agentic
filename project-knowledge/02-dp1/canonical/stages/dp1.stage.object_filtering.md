---
id: dp1.stage.object_filtering
title:
  uk: "Етап фільтрації об’єктів DP1"
  en: "DP1 Object filtering stage"
tags: [dp1, canonical, stage]
kind: stage-interface-card
source_role: canonical
source:
  file: "project-knowledge/02-dp1/canonical/stages/dp1.stage.object_filtering.md"
  lines: "1-130"
status: "draft"
---

## Definition

Відсіює псевдооб’єкти за геометричними та фотометричними критеріями і вибирає валідні об’єкти.

## Interface

`IObjectFilterStage`.

## Contract

`process(input, context, config) -> output`.

## Algorithmic idea

Відфільтрувати псевдооб’єкти за площею, геометрією, shape descriptors і
фотометричними правилами, не виконуючи downstream tracking.

## Inputs

Уточнені сегментовані кандидати плюс photometric references із домену обробки.

## Internal computation domain

Структурований домен кандидатів/об’єктів і, де потрібно, домен обробки.

## Outputs

Набір валідованих об’єктів.

## Complexity variants

- `L0`: фільтрація за площею.
- `L1`: площа плюс базова геометрія.
- `L2`: shape descriptors плюс фотометричні правила.

## OpenCV mapping

- Геометричні обчислення: `native`.
- Hu moments: `native`.
- Правила валідності: `custom`.

## Config fragment

Ключ DSL: `object_filtering`.

Обов’язкові поля: `enabled`, `variant`, `level`, `parameters`.

Допустимі `variant`: `area`, `geometry`, `shape_photometry`.

## Timing / profiling

Профілювати час фільтрації, кількість вхідних кандидатів, кількість відсіяних
об’єктів, кількість фінальних об’єктів і причини відсіву.

## Must not do

Змінювати raw data або визначати політику tracking у DP2.

## Constraints

Критерії мають бути задекларовані у майбутній специфікації етапу та конфігурації `C`.

Критичні інваріанти:
- причини відсіву мають трасуватися;
- приховані пороги поза `C` заборонені;
- фотометричні критерії мають посилатися на задекларований photometry domain.

## Failure cases

Надмірна фільтрація, недостатня фільтрація, неузгоджені критерії між рівнями складності.

## Typical misuse

Вбудовувати приховані пороги поза конфігурацією.

## Open questions

Canonical-словник валідності об’єкта.

## Connections

- feeds: dp1.stage.measurement
- uses: dp1.domain.processing
- constrained_by: dp1.domain.conversion_rules
