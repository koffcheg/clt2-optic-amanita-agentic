---
id: validation.dp1.radiometric_correction.inverse_median
title:
  uk: "Валідація inverse_median для етапу radiometric_correction"
  en: "Validation for radiometric_correction inverse_median"
tags: [validation, dp1, canonical, radiometric-correction, inverse-median]
kind: validation-card
source_role: verification
source:
  file: "project-knowledge/05-validation/cards/validation.dp1.radiometric_correction.inverse_median.md"
  lines: "1-180"
status: "draft"
---

## Definition

Картка валідації canonical-варіанта `inverse_median` етапу
`radiometric_correction`.

Ця картка описує вимоги до перевірки МКМФ. Вона не створює automated tests,
fixtures, mocks, snapshots або golden files без окремого погодження.

## Assumptions

- Валідація спирається на `dp1.stage_spec.radiometric_correction.inverse_median`.
- Тести можуть бути створені тільки після явного approval відповідно до
  `project-knowledge/00-governance/TESTING_POLICY.md`.
- До approval на тести допустимі тільки review by code reading, build або ручні
  sanity checks без додавання test files чи fixtures.

## Theorem / Contract

Майбутня реалізація `inverse_median` вважається валідованою тільки якщо
перевірено:
- підтримку `FixedK3`;
- підтримку `FixedK5`;
- `stride = 1`;
- `stride = 2`;
- `stride = 3`;
- каузальне формування часового вікна;
- відсутність використання майбутніх кадрів;
- hold-last-median поведінку для кадрів, що не потрапили у буфер;
- невалідний residual до заповнення циклічного буфера;
- `Med_t` у форматі фактичного вхідного кадру;
- residual `uint8 -> int16`;
- residual `uint16 -> int32`;
- точну відповідність формулі `Residual_t = I_t - Med_t`;
- `RawSigned`;
- `ClipToInputRange`;
- `ShiftToPositive`;
- `ScaleToInputRange`;
- відсутність динамічних алокацій у `processFrame`, якщо це можна
  інструментувати;
- стабільну роботу на довгій послідовності кадрів;
- окреме профілювання оновлення медіани, формування residual і приведення
  формату.

Еталонна перевірка медіани має використовувати повільний reference algorithm:
для кожного пікселя зібрати значення з відповідного часового вікна і обчислити
медіану через повне сортування.

Вимоги до коректності:
- побітова рівність `Med_t` з еталонною реалізацією;
- точна відповідність residual формулі `I_t - Med_t`;
- коректне обмеження значень при `ClipToInputRange`;
- `RawSigned` не втрачає від'ємні значення;
- `ShiftToPositive` і `ScaleToInputRange` виконуються тільки як явні режими
  приведення.

## Interpretation

Ця картка є validation route для `inverse_median`. Вона не замінює
`специфікація МКМФ`, configuration `C` або stage-interface card.

Валідація має перевіряти саме canonical behavior:
- `radiometric` DSL key;
- `variant: "inverse_median"`;
- `FixedK3` / `FixedK5`;
- causal + hold-last-median;
- signed residual як внутрішній pipeline output.

## Failure cases

- Тест перевіряє просторовий median filter замість міжкадрового попіксельного
  медіанного фільтру.
- Reference algorithm використовує centered або майбутні кадри.
- Порівнюється clipped output замість internal `RawSigned` residual.
- Warm-up стан трактується як валідний residual.
- `stride` помилково трактується як row stride або memory stride.
- Runtime allocation не перевіряється для hot path, хоча реалізація заявляє
  realtime behavior.

## Typical misuse

- Створювати тести або fixtures автоматично без approval.
- Вважати цю validation card достатньою для code generation.
- Використовувати user-facing converted frame як доказ коректності internal
  residual.

## Open questions

- Конкретне місце майбутніх automated tests у репозиторії.
- Чи потрібен окремий allocation instrumentation test.
- Які performance thresholds прийнятні для `RT-5`, `RT-20` або custom profiles.
- Які datasets або synthetic sequences будуть canonical для acceptance.

## Connections

- validates: dp1.stage_spec.radiometric_correction.inverse_median
- validates: dp1.stage.radiometric_correction
- constrained_by: project-knowledge/00-governance/TESTING_POLICY.md
