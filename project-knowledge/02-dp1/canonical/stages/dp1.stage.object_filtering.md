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
фотометричними правилами та видати `ValidatedObject[]`, не виконуючи downstream
tracking.

## Inputs

`Candidate[]` або `Segment[]` плюс optional photometric references із Raw або
Processing domain, якщо variant цього потребує.

## Internal computation domain

Struct domain і, де потрібно, Raw або Processing domain для photometric
criteria.

## Outputs

`ValidatedObject[]` у Struct domain. Accepted і rejected records мають бути
позначені bounded status/flags, якщо route зберігає rejected records.

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

Формувати `MeasurementRecord` або DP1 -> DP2 payload.

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
- produces: dp1.domain.struct.validated_object
- uses: dp1.domain.processing
- constrained_by: dp1.domain.conversion_rules
