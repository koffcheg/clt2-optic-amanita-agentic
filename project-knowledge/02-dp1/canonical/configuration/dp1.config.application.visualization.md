---
id: dp1.config.application.visualization
title:
  uk: "Canonical-конфігурація visualization застосунку DP1"
  en: "Canonical DP1 application visualization configuration"
tags: [dp1, canonical, config, application, runtime, visualization]
kind: config-card
source_role: canonical
source:
  file: "project-knowledge/02-dp1/canonical/configuration/dp1.config.application.visualization.md"
status: "draft"
---

## Definition

`ApplicationConfig.visualization` є canonical runtime-конфігурацією
необов’язкового створення артефактів візуалізації для перегляду людиною та
налагодження застосунком DP1.

Ця картка визначає типізовану поверхню конфігурації для секції `visualization`.
Кореневий `ApplicationConfig` визначає `dp1.config.application`.

## Assumptions

- Visualization керує тільки runtime-створенням діагностичних артефактів.
- Налаштування visualization не мають впливати на алгоритмічні результати.
- Артефакти візуалізації не є canonical measurement output.
- Артефакти візуалізації не є DP1 -> DP2 payload.
- Поточна реалізація підтримує тільки синхронний файловий режим `sync_file`.
- Семантика зображення та домену обмежена `dp1.domain.visualization`.

## Theorem / Contract

Конфігурація visualization не має вибирати порядок етапів, `stage.variant`,
`stage.level`, алгоритмічні параметри, input route, processing route,
семантику вимірювань або семантику DP1 -> DP2 payload.

Базова форма JSON authoring:

```json
{
  "visualization": {
    "enabled": false,
    "output_dir": "datapro1_v2_output/visualization",
    "mode": "sync_file",
    "every_n_frames": 1,
    "max_frames": 0,
    "stages": [
      "radiometric"
    ]
  }
}
```

Типізована canonical config model для code generation має існувати незалежно від
JSON representation. JSON є serialization/authoring form, але C++ генерація
має спиратися на typed fields:

```cpp
struct VisualizationConfig {
    bool enabled = false;
    std::string output_dir = "datapro1_v2_output/visualization";
    std::string mode = "sync_file";
    int every_n_frames = 1;
    int max_frames = 0;
    std::vector<std::string> stages;
};
```

## Fields / Interface

```yaml
fields:
  - name: "`visualization`"
    type: "`VisualizationConfig`"
    required: false
    default: "якщо секція відсутня, runtime parser застосовує defaults `VisualizationConfig`"
    purpose: "Містить усі canonical runtime settings для необов’язкового створення артефактів visualization."
    affects: "Чи runtime створює visualization sink, куди пише артефакти, з якою частотою і для яких підтриманих stage aliases."
    does_not_affect: "Не змінює порядок pipeline, stage variants, stage levels, algorithm parameters, measurement semantics або DP1 -> DP2 payload."
    validation: "Якщо секція присутня, має бути JSON object без unknown keys."

  - name: "`visualization.enabled`"
    type: "`bool`"
    required: false
    default: "`false`"
    purpose: "Вмикає або вимикає створення артефактів візуалізації на рівні застосунку."
    affects: "Чи `VisualizationSink` фактично записує артефакти для дозволених stages."
    does_not_affect: "Не вимикає stage execution, не змінює stage outputs і не змінює result sink behavior."
    validation: "Тільки boolean value."

  - name: "`visualization.output_dir`"
    type: "`std::string`"
    required: false
    default: "`datapro1_v2_output/visualization`"
    purpose: "Задає каталог, у який runtime записує артефакти візуалізації."
    affects: "Розташування у filesystem для створених visualization files і пов’язаних per-frame artifacts."
    does_not_affect: "Не визначає schema артефактів, не змінює measurement output path і не задає DP2 destination."
    validation: "Має бути непорожнім string, якщо `visualization.enabled=true`; path має бути portable relative path або approved runtime path."

  - name: "`visualization.mode`"
    type: "`std::string`"
    required: false
    default: "`sync_file`"
    purpose: "Оголошує runtime backend/policy для створення артефактів візуалізації."
    affects: "Який режим створення артефактів візуалізації може використати application runtime."
    does_not_affect: "Не створює renderer API, async worker, network stream або GUI contract."
    validation: "Поточна реалізація підтримує тільки `sync_file`; будь-яке інше значення invalid."

  - name: "`visualization.every_n_frames`"
    type: "`int`"
    required: false
    default: "`1`"
    purpose: "Задає інтервал відбору frames для створення артефактів visualization."
    affects: "Як часто runtime пробує записати артефакти для enabled visualization stages."
    does_not_affect: "Не змінює кількість processed frames, runtime loop limits або frame ids."
    validation: "Має бути integer `>= 1`."

  - name: "`visualization.max_frames`"
    type: "`int`"
    required: false
    default: "`0`"
    purpose: "Задає верхню межу кількості frames, для яких runtime може створити артефакти візуалізації."
    affects: "Обмежений обсяг артефактів візуалізації для одного application run."
    does_not_affect: "Не зупиняє runtime loop, не обмежує source ingestion і не змінює stage processing budget."
    validation: "Має бути integer `>= 0`; `0` означає без обмеження кількості visualization frames."

  - name: "`visualization.stages`"
    type: "`std::vector<std::string>`"
    required: false
    default: "`[]`"
    purpose: "Білий список stage aliases, для яких runtime може записувати артефакти візуалізації."
    affects: "Які підтримані stage outputs передаються у visualization sink."
    does_not_affect: "Не додає нові pipeline stages, не змінює canonical stage registry і не дозволяє stage output contract changes."
    validation: "Має бути обмежений string array без duplicate values; кожен item має бути непорожнім;"
```

## Interpretation

`dp1.config.application.visualization` є конфігурацією application-runtime
навколо pipeline, а не частиною конфігурації pipeline `C`.

Артефакти візуалізації призначені для перегляду людиною, діагностики та
обмеженого runtime evidence. Вони не є джерелом обчислень, не замінюють
measurement domain і не визначають protocol payload.

## Failure cases

- Зображення visualization використовується як input для computation.
- `visualization.mode` містить backend, який runtime не підтримує.
- `visualization.stages` містить stage alias поза підтриманим registry.
- `visualization.output_dir` порожній при `visualization.enabled=true`.
- Артефакти візуалізації трактуються як measurement handoff або protocol
  payload.
- Конфігурація visualization використовується для algorithm parameters або stage
  variant selection.

## Typical misuse

- Додавати algorithm thresholds або stage parameters у visualization config.
- Вважати PNG/debug artifacts canonical output DP1.
- Вмикати visualization у performance-sensitive runtime без bounded limits.
- Використовувати runtime alias `radiometric` як доказ повної canonical stage
  registry.
- Трактувати `visualization.mode` як довільний plugin/backend selector.

## Open questions

- Чи має artifact manifest schema бути окремою canonical card.
- Чи мають stage names у `visualization.stages` перейти з runtime aliases на
  canonical stage IDs.
- Чи потрібні окремі cards для renderer policy, async mode і artifact naming.

## Connections

- belongs_to: dp1.config.application
- constrained_by: dp1.domain.visualization
- related_runtime: datapro1_v2/include/dp1v2/config/config.hpp
- related_runtime: datapro1_v2/src/config/config.cpp
- follows_up: AMNT-0029
