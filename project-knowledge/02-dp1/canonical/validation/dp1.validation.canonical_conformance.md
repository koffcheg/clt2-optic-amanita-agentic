---
id: dp1.validation.canonical_conformance
title:
  uk: "Валідація відповідності canonical DP1"
  en: "Canonical DP1 conformance validation"
tags: [dp1, canonical, validation]
kind: validation-card
source_role: verification
source:
  file: "project-knowledge/02-dp1/canonical/validation/dp1.validation.canonical_conformance.md"
  lines: "1-150"
status: "draft"
---

## Definition

Маршрут валідації для перевірки, чи реалізація DP1 відповідає знанням canonical DP1.

## Assumptions

Це документаційна картка валідації. Automated tests у цій задачі не створюються.

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
- відтворюваність результатів;
- доступність профілювання для кожного етапу;
- дотримання рівнів складності.

## Interpretation

Ця картка визначає, що потрібно перевірити перед тим, як код canonical DP1 можна вважати таким, що відповідає специфікації.

## Failure cases

- Код етапу існує без специфікації етапу.
- Legacy-протокол трактується як canonical handoff.

## Typical misuse

- Трактувати успішну компіляцію як відповідність специфікації.

## Open questions

- Конкретні метрики і пороги.

## Connections

- uses: dp1.pipeline.stage_contract
- uses: protocols.dp1_dp2.measurement_handoff
