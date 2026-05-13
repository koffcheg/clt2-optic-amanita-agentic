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
- Поведінка cyclic frame buffer перевіряється як частина поведінки
  `inverse_median`, а не як окремий тест reusable component без додаткового
  погодження.
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
- reset стану історії при зміні geometry, pixel format, input bit depth, range
  policy або binning route;
- що residual geometry відповідає фактичній geometry канонічного input після
  Stage 0 / input normalization;
- що сховище історії не використовує IPC/shared-memory transport slots як
  довгоживучу пам'ять МКМФ;
- що кадри, відібрані у temporal window, копіюються в algorithm-owned
  `CyclicFrameBuffer`;
- що borrowed current-frame view не зберігається після завершення часу життя
  input boundary;
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
- коректне обмеження значень при `ClipToInputRange`: `R < 0 -> 0`,
  `0 <= R <= U -> R`, `R > U -> U`;
- межі `ClipToInputRange` беруться з типу вихідного зображення, а не з
  `min(R)` / `max(R)` по кадру;
- `RawSigned` не втрачає від'ємні значення;
- `ClipToInputRange` не виконує min-max normalization і не масштабує residual.
- після reset через зміну geometry, pixel format, input bit depth, range policy
  або binning route, перший валідний residual знову з'являється тільки після
  заповнення нового temporal window;
- зміна `binning.owner` або `binning.factor` не може повторно використовувати
  попередній стан історії;

## Interpretation

Ця картка є validation route для `inverse_median`. Вона не замінює
`специфікація МКМФ`, configuration `C` або stage-interface card.

Валідація має перевіряти саме canonical behavior:
- `radiometric` DSL key;
- `variant: "inverse_median"`;
- `FixedK3` / `FixedK5`;
- causal + hold-last-median;
- signed residual як внутрішній pipeline output.
- обмежену algorithm-owned history у `CyclicFrameBuffer`, відокремлену від
  borrowed transport/current-frame views.

## Failure cases

- Тест перевіряє просторовий median filter замість міжкадрового попіксельного
  медіанного фільтру.
- Reference algorithm використовує centered або майбутні кадри.
- Порівнюється clipped output замість internal `RawSigned` residual.
- `ClipToInputRange` перевіряється як min-max normalization по кадру замість
  saturating cast до діапазону типу.
- Warm-up стан трактується як валідний residual.
- `stride` помилково трактується як row stride або memory stride.
- Фізичний порядок slots циклічного буфера помилково трактується як хронологічний
  порядок у перевірках, хоча для `inverse_median` median має бути
  order-insensitive.
- Runtime allocation не перевіряється для hot path, хоча реалізація заявляє
  realtime behavior.
- Тест або review дозволяє реалізації використовувати IPC/shared-memory slots як
  temporal history МКМФ.
- Стан історії не скидається після зміни geometry, pixel format, input bit
  depth, range policy або binning route.
- Residual geometry помилково порівнюється з source-frame geometry, коли Stage 0
  вже змінив geometry через binning.

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
- references: dp1.domain.runtime.cyclic_frame_buffer
- references: dp1.domain.memory_ownership
- constrained_by: project-knowledge/00-governance/TESTING_POLICY.md
