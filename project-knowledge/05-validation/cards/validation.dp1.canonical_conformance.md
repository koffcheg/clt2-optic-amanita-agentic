---
id: validation.dp1.canonical_conformance
title:
  uk: "Маршрут валідації відповідності canonical DP1"
  en: "Validation route for canonical DP1 conformance"
tags: [validation, dp1, canonical]
kind: validation-card
source_role: verification
source:
  file: "project-knowledge/05-validation/cards/validation.dp1.canonical_conformance.md"
  lines: "1-160"
status: "draft"
---

## Definition

Глобальна картка валідації відповідності canonical DP1.

## Assumptions

Ця картка перевіряє повноту специфікації та відповідність реалізації. Вона сама по собі не створює automated tests.

## Theorem / Contract

Перед генерацією або прийняттям canonical DP1 code потрібно перевірити:
- існують картки Raw, Processing, Mask, Measurement, Visualization і conversion rules;
- існують усі вісім stage-interface cards;
- для етапів, що реалізуються, існують майбутні stage specifications;
- кожен етап дотримується `process(input, context, config) -> output`;
- неявні перетворення типів заборонені та підлягають аудиту;
- computation і visualization розділені;
- raw dynamic range зберігається до Measurement domain;
- Measurement domain є structured data, а не `cv::Mat`;
- canonical DP1 -> DP2 handoff визначений у `04-protocols`;
- вибір реалізацій задається конфігурацією `C`;
- відтворюваність результатів визначена;
- profiling доступний для кожного етапу;
- complexity levels задекларовані та перевіряються.

## Interpretation

Це validation entry у `05-validation/`. DP1-local validation card може містити DP1-specific context, але ця картка є cross-section route.

## Failure cases

- Код існує до формальної stage specification.
- Legacy transport проходить валідацію як canonical handoff.
- Debug/visualization output трактується як computation data.

## Typical misuse

- Трактувати цей checklist як заміну stage specs.

## Open questions

- Конкретні test datasets.
- Числові tolerances.
- Формат profiling.
- Acceptance thresholds для complexity levels.

## Connections
- validates: dp1.validation.canonical_conformance
- uses: protocols.dp1_dp2.measurement_handoff
