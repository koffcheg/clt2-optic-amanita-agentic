---
id: dp1.validation.canonical_conformance
title:
  uk: "Вимоги відповідності canonical DP1"
  en: "Canonical DP1 conformance requirements"
tags: [dp1, canonical, validation]
kind: validation-card
source_role: verification
source:
  file: "project-knowledge/02-dp1/canonical/validation/dp1.validation.canonical_conformance.md"
  lines: "1-150"
status: "draft"
---

## Definition

DP1-local вимоги відповідності для canonical DP1 knowledge і stage architecture.

Ця картка визначає, що canonical DP1 очікує перевірити перед тим, як реалізацію можна вважати узгодженою з цільовою DP1-архітектурою. Вона не є cross-project test execution route або runtime validation workflow.

## Assumptions

Це документаційна картка вимог відповідності. Automated tests у цій задачі не створюються.

Практичне виконання перевірок має бути описане в `05-validation`, де validation route прив'язується до code/config/test scenarios/reports/runtime evidence.

## Theorem / Contract

Перевірки відповідності включають:
- існування карток доменів даних;
- існування восьми canonical-карток інтерфейсів етапів;
- наявність специфікацій етапів перед генерацією коду;
- відповідність кожного етапу `process(input, context, config) -> output`;
- відсутність неявних перетворень;
- відсутність змішування обчислень і візуалізації;
- збереження динамічного діапазону raw до вимірювання;
- коректність домену вимірювань;
- використання canonical-межі протоколу для DP1 -> DP2 handoff;
- вибір реалізацій через конфігурацію `C`;
- перевірку `variant` через `dp1.config.stage_variant_registry`;
- відповідність stage input/output до `dp1.pipeline.stage_io_matrix`;
- дотримання `dp1.domain.opencv_invariants` для всіх `cv::Mat` carriers;
- відтворюваність результатів;
- доступність профілювання для кожного етапу;
- дотримання рівнів складності.

## Interpretation

Ця картка є DP1-local requirements source: вона описує, що саме треба перевіряти для canonical DP1 conformance.

Вона не визначає команди запуску тестів, datasets, runtime reports або CI workflow. Такі execution-level деталі мають жити в `05-validation`.

## Failure cases

- Код етапу існує без специфікації етапу.
- Legacy-протокол трактується як canonical handoff.
- DP1-local requirements використовуються як заміна execution validation route.
- Code generation починається без stage contract checks і без registered
  variant.

## Typical misuse

- Трактувати успішну компіляцію як відповідність специфікації.
- Трактувати цю картку як інструкцію запуску тестів.

## Open questions

- Конкретні метрики і пороги.
- Які datasets/runtime reports мають бути обов'язковими для execution validation.

## Connections

- uses: dp1.pipeline.stage_contract
- uses: dp1.pipeline.stage_io_matrix
- uses: dp1.config.stage_variant_registry
- uses: dp1.domain.opencv_invariants
- uses: protocols.dp1_dp2.measurement_handoff
- extended_by: dp1.validation.stage_contract_checks
- executed_by: validation.dp1.canonical_conformance
