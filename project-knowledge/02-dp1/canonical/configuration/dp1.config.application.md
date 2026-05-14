---
id: dp1.config.application
title:
  uk: "Canonical-конфігурація застосунку DP1"
  en: "Canonical DP1 application configuration"
tags: [dp1, canonical, config, application, runtime]
kind: config-card
source_role: canonical
source:
  file: "project-knowledge/02-dp1/canonical/configuration/dp1.config.application.md"
status: "draft"
---

## Definition

`ApplicationConfig` є root canonical-конфігурацією runtime-рівня застосунку DP1.

Вона визначає верхньорівневу форму application runtime configuration і посилає
детальні секції до окремих child config cards.

## Assumptions

- Повна модель `ApplicationConfig` ще не визначена.
- Поточна root-картка визначає `ApplicationConfig.schema_version` і підключає
  child sections `ApplicationConfig.logging` та `ApplicationConfig.profiling`.
- Деталі logging визначає `dp1.config.application.logging`.
- Деталі profiling визначає `dp1.config.application.profiling`.
- `PipelineConfig C` вже існує як окрема canonical-конфігурація конвеєра
  обробки.
- Application runtime configuration не має впливати на алгоритмічні результати.

## Theorem / Contract

DP1 має дві окремі конфігураційні площини:

1. `PipelineConfig C` конфігурує конвеєр обробки.
2. `ApplicationConfig` конфігурує runtime-поведінку застосунку навколо конвеєра.

У межах цієї root-картки визначено:

- `ApplicationConfig.schema_version`;
- `ApplicationConfig.logging`, визначений у `dp1.config.application.logging`;
- `ApplicationConfig.profiling`, визначений у `dp1.config.application.profiling`.

Application runtime configuration не має вибирати порядок етапів,
`stage.variant`, `stage.level`, алгоритмічні параметри, input route, processing
route, семантику вимірювань або семантику DP1 -> DP2 payload.

Базова форма JSON authoring:

```json
{
  "schema_version": "1.0",
  "logging": {},
  "profiling": {}
}
```

Typed canonical config model для code generation має існувати незалежно від
JSON representation. JSON є serialization/authoring form, але C++ генерація
має спиратися на typed fields:

```cpp
struct ApplicationConfig {
    std::string schema_version;
    LoggingConfig logging;
    ProfilingConfig profiling;
};
```

`LoggingConfig` визначає `dp1.config.application.logging`.
`ProfilingConfig` визначає `dp1.config.application.profiling`.

## Fields / Interface

```yaml
fields:
  - name: "`schema_version`"
    type: "`std::string`"
    required: true
    default: "немає implicit default; authoring form має явно містити `\"1.0\"`"
    purpose: "Версія schema конфігурації застосунку."
    affects: "Вибір parser/validator для `ApplicationConfig` і правила перевірки вкладених секцій."
    does_not_affect: "Не змінює поведінку конвеєра обробки і не впливає на алгоритмічні результати."
    validation: "Має бути підтриманою версією; на першому етапі підтримується тільки `\"1.0\"`."

  - name: "`logging`"
    type: "`LoggingConfig`"
    required: true
    default: "якщо секція відсутня, configuration invalid"
    purpose: "Application runtime logging settings."
    affects: "Startup/bootstrap logging setup і bounded runtime logging behavior."
    does_not_affect: "Не змінює pipeline processing behavior або DP1 output."
    validation: "Має відповідати `dp1.config.application.logging`."

  - name: "`profiling`"
    type: "`ProfilingConfig`"
    required: true
    default: "якщо секція відсутня, configuration invalid"
    purpose: "Application runtime profiling settings."
    affects: "Runtime profiling setup, bounded trace retention, aggregation і report policy."
    does_not_affect: "Не змінює pipeline processing behavior або DP1 output."
    validation: "Має відповідати `dp1.config.application.profiling`."
```

## Input / Output

Input:

- application configuration authoring file;
- `dp1.config.application.logging`;
- `dp1.config.application.profiling`;
- `PROJECT_ECOSYSTEM.md` як джерело environment facts.

Output:

- typed `ApplicationConfig`;
- validated runtime logging configuration;
- validated runtime profiling configuration.

## Constraints

- `ApplicationConfig` не має дублювати `PipelineConfig C`.
- Runtime sections не мають містити stage parameters.
- Runtime sections не мають містити algorithm thresholds.
- Runtime sections не мають містити input/processing route selection.
- Runtime sections не мають створювати DP1 -> DP2 payload fields.
- Нові runtime configuration sections мають додаватися як child cards
  `dp1.config.application.<section>`, а не розростатися всередині root-картки.

## Interpretation

`dp1.config.application` є navigation/root contract для runtime application
configuration. Детальні правила секцій мають жити в child config cards, щоб
root-картка залишалася стабільною при додаванні нових runtime capabilities.

## Failure cases

- Application runtime config додано в `PipelineConfig C`.
- Stage implementation читає runtime config section для вибору algorithm behavior.
- Нова runtime section додана тільки в root-картку без окремої child card.
- Root-картка дублює всі поля child section і стає source-of-truth conflict.

## Typical misuse

- Змішувати application runtime config і pipeline config в одному object.
- Додавати `logging_enabled` або `profiling_enabled` у stage parameters.
- Використовувати runtime config як validation/output configuration.
- Вважати child config card окремою configuration plane поза `ApplicationConfig`.

## Open questions

- Final default path for DP1 application config file.
- Чи потрібен registry для майбутніх application runtime sections.

## Connections

- separates_from: dp1.config.pipeline_configuration_c
- contains: dp1.config.application.logging
- contains: dp1.config.application.profiling
- references: project-knowledge/01-project/PROJECT_ECOSYSTEM.md
- related_protocol_boundary: protocols.dp1_dp2.measurement_handoff
