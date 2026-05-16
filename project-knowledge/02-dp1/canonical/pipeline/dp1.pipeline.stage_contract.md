---
id: dp1.pipeline.stage_contract
title:
  uk: "Контракт етапу canonical DP1"
  en: "Canonical DP1 stage contract"
tags: [dp1, canonical, stage-contract]
kind: stage-interface-card
source_role: canonical
source:
  file: "project-knowledge/02-dp1/canonical/pipeline/dp1.pipeline.stage_contract.md"
  lines: "1-120"
status: "draft"
---

## Definition

Кожен основний етап canonical DP1 є інтерфейсною одиницею обробки з
уніфікованою формою виконання:

```text
process(input, context, config) -> output
```

In C++ implementations, route-specific stage execution must be represented by overloads named `process(...)`. Route-specific method names such as `processFullFrame(...)`, `processTile(...)`, `prepareFullFrame(...)`, and `prepareTiles(...)` are not canonical stage execution methods. Exact C++ parameter lists for full-frame, ROI, and tile routes remain subject to route-specific stage specifications.

## Assumptions

- Картка інтерфейсу етапу описує межу етапу, а не повну алгоритмічну
  реалізацію.
- Повні деталі реалізації належать до майбутнього малого ТЗ етапу.
- `input`, внутрішній домен обчислень і `output` явно декларуються в картці
  кожного етапу.

## Theorem / Contract

Етап має декларувати:
- ідентифікатор етапу;
- canonical-назву інтерфейсу;
- призначення;
- алгоритмічну ідею;
- вхідні домени даних;
- конкретні вхідні формати;
- внутрішній домен обчислень;
- конкретні внутрішні формати;
- вихідні домени даних;
- конкретні вихідні формати;
- підтримувані рівні складності (`L0`, `L1`, `L2`; за потреби `L3`/`Lx`);
- відповідність OpenCV (`native`, `wrapped`, `composed`, `custom`);
- фрагмент конфігурації (`enabled`, `variant`, `level`, `parameters`);
- вимоги до часу виконання та профілювання;
- володіння станом, якщо етап допускає stateful-модель;
- критичні інваріанти;
- що етап не повинен робити;
- обмеження;
- типові відмови;
- зв’язки.

Фрагмент конфігурації кожного етапу є частиною canonical-конфігурації
конвеєра `C`:

```json
{
  "enabled": true,
  "variant": "<variant-id>",
  "level": "L0|L1|L2|L3|Lx",
  "parameters": {}
}
```

`variant` має бути зареєстрований у `dp1.config.stage_variant_registry`.
`level` має бути рівнем складності з `dp1.config.complexity_levels`, а не
назвою algorithm variant.

Stage output має бути explicit domain structure. Stage не має записувати
primary result тільки у `FrameContext`.

Кожен executed canonical stage має створювати або оновлювати один
`StageTiming` record згідно з `dp1.domain.profiling`. Disabled stage не має
отримувати fake duration; disabled/skipped state може бути записаний окремо як
status або diagnostic.

## Interpretation

Це контракт інтерфейсу, а не повна специфікація етапу. Сам по собі він не є
достатнім джерелом для генерації коду.

## Failure cases

- Неявне перетворення типів.
- Змішування обчислень і візуалізації.
- Прихована залежність від legacy-буферів.
- Прихований стан етапу, не задекларований у картці.
- Витрати часу етапу не враховані у кадровій часовій моделі.
- Stage не створює `StageTiming` record згідно з `dp1.domain.profiling`.
- Варіант алгоритму вибрано поза конфігурацією `C`.
- Primary output stage записаний у `FrameContext` замість explicit output.
- `variant` і `level` змішані в configuration або stage card.

## Typical misuse

- Генерувати код із цього інтерфейсу без майбутньої специфікації етапу.
- Трактувати OpenCV-примітив як архітектуру етапу.

## Open questions

- Стандартні поля контексту.
- Пороги валідації для кожного рівня складності.
- Exact C++ parameter lists for full-frame, ROI, and tile route overloads.

## Connections

- used_by: dp1.stage.prep
- used_by: dp1.stage.measurement
- constrains: dp1.config.pipeline_configuration_c
- constrained_by: dp1.config.stage_variant_registry
- constrained_by: dp1.pipeline.stage_io_matrix
- constrained_by: dp1.domain.conversion_rules
- constrained_by: dp1.domain.profiling
