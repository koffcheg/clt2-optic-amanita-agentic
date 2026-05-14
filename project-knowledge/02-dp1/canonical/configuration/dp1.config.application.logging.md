---
id: dp1.config.application.logging
title:
  uk: "Canonical-конфігурація logging застосунку DP1"
  en: "Canonical DP1 application logging configuration"
tags: [dp1, canonical, config, application, runtime, logging]
kind: config-card
source_role: canonical
source:
  file: "project-knowledge/02-dp1/canonical/configuration/dp1.config.application.logging.md"
status: "draft"
---

## Definition

`ApplicationConfig.logging` є canonical runtime-конфігурацією logging для
застосунку DP1.

Ця картка визначає typed configuration surface для logging section. Root
`ApplicationConfig` визначає `dp1.config.application`.

## Assumptions

- Logging керує тільки виведенням діагностичних повідомлень.
- Налаштування logging не мають впливати на алгоритмічні результати.
- log4cxx XML може бути конфігурацією рівня реалізації або інфраструктури, але
  canonical typed surface описується цією карткою.
- Profiling configuration визначає окрема картка
  `dp1.config.application.profiling`.

## Theorem / Contract

Logging configuration не має вибирати порядок етапів, `stage.variant`,
`stage.level`, алгоритмічні параметри, input route, processing route, семантику
вимірювань або семантику DP1 -> DP2 payload.

Базова форма JSON authoring:

```json
{
  "logging": {
    "enabled": true,
    "config_file": "config/datapro1_v2-log.xml",
    "default_level": "INFO",
    "realtime_profile": "rt_safe",
    "structured_messages": true,
    "sanitize_external_strings": true,
    "max_field_length": 256,
    "max_messages_per_frame": 64,
    "max_messages_per_tile": 16,
    "mdc": {
      "enabled": true,
      "fields": [
        "pipeline_run_id",
        "camera_id",
        "source_id",
        "frame_id",
        "stage"
      ]
    },
    "sampling": {
      "frame_summary_every_n": 100,
      "rate_limit_per_event_per_sec": 1,
      "duplicate_suppression": true
    },
    "async": {
      "enabled": true,
      "buffer_size": 1024,
      "blocking": false,
      "discard_policy": "drop_debug_and_summarize"
    },
    "logger_overrides": [
      {
        "logger": "amanita.dp1",
        "level": "INFO"
      },
      {
        "logger": "amanita.protocol.dp1_dp2",
        "level": "WARN"
      }
    ]
  }
}
```

Typed canonical config model для code generation має існувати незалежно від
JSON representation. JSON є serialization/authoring form, але C++ генерація
має спиратися на typed fields:

```cpp
struct LoggingMdcConfig {
    bool enabled = true;
    std::vector<std::string> fields;
};

struct LoggingSamplingConfig {
    int frame_summary_every_n = 0;
    int rate_limit_per_event_per_sec = 0;
    bool duplicate_suppression = true;
};

struct LoggingAsyncConfig {
    bool enabled = true;
    int buffer_size = 1024;
    bool blocking = false;
    std::string discard_policy;
};

struct LoggerLevelOverride {
    std::string logger;
    std::string level;
};

struct LoggingConfig {
    bool enabled = true;
    std::string config_file;
    std::string default_level;
    std::string realtime_profile;
    bool structured_messages = true;
    bool sanitize_external_strings = true;
    int max_field_length = 256;
    int max_messages_per_frame = 64;
    int max_messages_per_tile = 16;
    LoggingMdcConfig mdc;
    LoggingSamplingConfig sampling;
    LoggingAsyncConfig async;
    std::vector<LoggerLevelOverride> logger_overrides;
};
```

## Fields / Interface

```yaml
fields:
  - name: "`logging`"
    type: "`LoggingConfig`"
    required: true
    default: "якщо секція відсутня, configuration invalid"
    purpose: "Містить усі canonical logging settings runtime-рівня застосунку."
    affects: "Startup/bootstrap logging setup, emission, routing, threshold, sampling і bounded runtime safeguards для logs."
    does_not_affect: "Не змінює порядок етапів, stage variants, stage levels, алгоритмічні параметри або семантику DP1 output."
    validation: "Має пройти validation усіх вкладених полів `LoggingConfig`."

  - name: "`logging.enabled`"
    type: "`bool`"
    required: true
    default: "`true`"
    purpose: "Вмикає або вимикає emission логів на рівні застосунку."
    affects: "Чи application bootstrap налаштовує active logging emission і чи runtime надсилає log events у logging backend."
    does_not_affect: "Не вимикає внутрішні `StageStatus`, `DiagnosticMessage`, error handling або validation logic."
    validation: "Тільки boolean value."

  - name: "`logging.config_file`"
    type: "`std::string`"
    required: true
    default: "немає implicit default; рекомендований приклад `config/datapro1_v2-log.xml`"
    purpose: "Посилання на log4cxx configuration file, який читає startup/bootstrap код."
    affects: "Завантаження appenders, layouts і logger thresholds під час startup застосунку."
    does_not_affect: "Не є canonical schema для `ApplicationConfig` і не визначає алгоритмічну поведінку."
    validation: "Має бути непорожнім portable relative path або approved absolute path; не має містити credentials."

  - name: "`logging.default_level`"
    type: "`std::string`"
    required: true
    default: "`INFO`"
    purpose: "Базовий logging level для policy запуску застосунку."
    affects: "Threshold для loggerів без explicit override."
    does_not_affect: "Не змінює severity, яку code обрав для конкретного event, і не перетворює нормальний control flow на error."
    validation: "Має бути одним із `TRACE`, `DEBUG`, `INFO`, `WARN`, `ERROR`, `FATAL`."

  - name: "`logging.realtime_profile`"
    type: "`std::string`"
    required: true
    default: "`rt_safe`"
    purpose: "Оголошує очікуваний real-time safety profile для logging emission."
    affects: "Допустимість blocking behavior, verbose levels, sampling defaults і rate limiting defaults."
    does_not_affect: "Не змінює pipeline profile `RT-5|RT-20|custom`."
    validation: "На першому етапі дозволені значення: `rt_safe`, `diagnostic`."

  - name: "`logging.structured_messages`"
    type: "`bool`"
    required: true
    default: "`true`"
    purpose: "Вимагає stable key-value message style для нових log messages."
    affects: "Формат нових log messages і можливість пошуку/парсингу за stable fields."
    does_not_affect: "Не вимагає зміни існуючих messages поза scope поточної задачі."
    validation: "`true` для canonical DP1 logging; `false` можливий тільки як compatibility exception."

  - name: "`logging.sanitize_external_strings`"
    type: "`bool`"
    required: true
    default: "`true`"
    purpose: "Вимагає sanitization зовнішніх рядків перед logging."
    affects: "Обробку camera/source identifiers, file names, endpoint labels і external error strings перед emission."
    does_not_affect: "Не дозволяє логувати secrets навіть після sanitization."
    validation: "Canonical default має бути `true`; `false` потребує explicit approval."

  - name: "`logging.max_field_length`"
    type: "`int`"
    required: true
    default: "`256`"
    purpose: "Обмежує довжину одного string field value у log message."
    affects: "Truncation або rejection policy для external strings і verbose details."
    does_not_affect: "Не дозволяє dumping великих containers/images через chunking."
    validation: "Має бути `> 0`; recommended range `64..1024`."

  - name: "`logging.max_messages_per_frame`"
    type: "`int`"
    required: true
    default: "`64`"
    purpose: "Задає верхню межу log emissions або accepted log-worthy diagnostics для одного frame."
    affects: "Per-frame safeguard від log spam і uncontrolled warm-path logging."
    does_not_affect: "Не має приховувати `ERROR` або `FATAL` без summary чи overflow event."
    validation: "Має бути `>= 0`; `0` означає no per-frame logging except lifecycle/errors, якщо це явно прийнято runtime policy."

  - name: "`logging.max_messages_per_tile`"
    type: "`int`"
    required: true
    default: "`16`"
    purpose: "Задає верхню межу log emissions або accepted log-worthy diagnostics для одного tile."
    affects: "Tile-route safeguard від multiplied log volume."
    does_not_affect: "Не змінює кількість tile diagnostics у canonical runtime structures, якщо вони мають окремий bounded budget."
    validation: "Має бути `>= 0`; має бути меншим або рівним `max_messages_per_frame`, якщо обидва limits активні."

  - name: "`logging.mdc`"
    type: "`LoggingMdcConfig`"
    required: true
    default: "enabled із рекомендованими fields"
    purpose: "Конфігурує allowed MDC context propagation."
    affects: "Які context keys bootstrap/runtime може встановлювати в log4cxx MDC."
    does_not_affect: "Не замінює explicit data contracts або `FrameContext`."
    validation: "Вкладені поля мають бути коректними."

  - name: "`logging.mdc.enabled`"
    type: "`bool`"
    required: true
    default: "`true`"
    purpose: "Дозволяє або забороняє використання MDC."
    affects: "Чи повторюваний context може бути встановлений у thread-local/logging context."
    does_not_affect: "Не дозволяє implicit cross-frame state."
    validation: "Тільки boolean value."

  - name: "`logging.mdc.fields`"
    type: "`std::vector<std::string>`"
    required: "true, якщо `mdc.enabled=true`"
    default: "`[\"pipeline_run_id\", \"camera_id\", \"source_id\", \"frame_id\", \"stage\"]`"
    purpose: "Whitelist MDC keys, які можуть бути встановлені runtime/bootstrap кодом."
    affects: "Які поля можуть автоматично додаватися до log context."
    does_not_affect: "Не дозволяє додавати large або unbounded values."
    validation: "Тільки allowed keys із `LOGGING_POLICY.md`; без duplicate keys; список має бути bounded."

  - name: "`logging.sampling`"
    type: "`LoggingSamplingConfig`"
    required: true
    default: "див. nested defaults"
    purpose: "Задає sampling/rate limiting для high-frequency diagnostics."
    affects: "Частоту repeated warm-path messages."
    does_not_affect: "Не має придушувати critical failures без bounded summary."
    validation: "Вкладені поля мають бути коректними."

  - name: "`logging.sampling.frame_summary_every_n`"
    type: "`int`"
    required: true
    default: "`100`"
    purpose: "Задає частоту frame-level summary logs."
    affects: "Emission summary на кожні N frames."
    does_not_affect: "Не вмикає per-frame `INFO` для кожного frame, якщо `N > 1`."
    validation: "Має бути `>= 0`; `0` означає disabled."

  - name: "`logging.sampling.rate_limit_per_event_per_sec`"
    type: "`int`"
    required: true
    default: "`1`"
    purpose: "Задає rate limit для repeated event names."
    affects: "Максимальну частоту однакових repeated events."
    does_not_affect: "Не змінює event severity і не видаляє final summary."
    validation: "Має бути `>= 0`; семантика значення `0` лишається open question."

  - name: "`logging.sampling.duplicate_suppression`"
    type: "`bool`"
    required: true
    default: "`true`"
    purpose: "Придушує повторювані ідентичні events."
    affects: "Log volume при repeated failures."
    does_not_affect: "Не має приховувати state transition або first occurrence."
    validation: "Тільки boolean value."

  - name: "`logging.async`"
    type: "`LoggingAsyncConfig`"
    required: true
    default: "`enabled=true`, `buffer_size=1024`, `blocking=false`, `discard_policy=drop_debug_and_summarize`"
    purpose: "Описує очікувану async logging behavior."
    affects: "Startup/bootstrap validation of log4cxx async appender setup."
    does_not_affect: "Stage code не має створювати або змінювати appenders."
    validation: "Вкладені поля мають бути коректними."

  - name: "`logging.async.enabled`"
    type: "`bool`"
    required: true
    default: "`true`"
    purpose: "Оголошує очікування async appender path для runtime logging."
    affects: "Чи bootstrap перевіряє async logging setup."
    does_not_affect: "Не дозволяє stage code напряму взаємодіяти з appender."
    validation: "Тільки boolean value."

  - name: "`logging.async.buffer_size`"
    type: "`int`"
    required: true
    default: "`1024`"
    purpose: "Описує очікувану місткість async logging buffer."
    affects: "Допустимий burst log capacity."
    does_not_affect: "Не є дозволом на high-volume hot-path logging."
    validation: "Має бути `> 0`; minimum recommended `128`."

  - name: "`logging.async.blocking`"
    type: "`bool`"
    required: true
    default: "`false`"
    purpose: "Визначає, чи logging може блокувати caller при full async buffer."
    affects: "Real-time latency risk."
    does_not_affect: "Не змінює correctness або error handling."
    validation: "Для `rt_safe` має бути `false`."

  - name: "`logging.async.discard_policy`"
    type: "`std::string`"
    required: true
    default: "`drop_debug_and_summarize`"
    purpose: "Визначає поведінку при overload logging buffer."
    affects: "Які lower-priority messages можуть бути dropped або summarized."
    does_not_affect: "Не має silent-drop critical errors."
    validation: "На першому етапі allowed values: `drop_debug_and_summarize`, `block`, `drop_new_debug`."

  - name: "`logging.logger_overrides`"
    type: "`std::vector<LoggerLevelOverride>`"
    required: false
    default: "empty vector"
    purpose: "Містить explicit level overrides для canonical logger names."
    affects: "Thresholds для конкретних loggerів."
    does_not_affect: "Не створює нові logger naming conventions."
    validation: "Кожен override має valid logger і valid level; duplicate logger override заборонений."

  - name: "`LoggerLevelOverride.logger`"
    type: "`std::string`"
    required: true
    default: "немає"
    purpose: "Canonical logger name, до якого застосовується override."
    affects: "Threshold конкретного logger namespace."
    does_not_affect: "Не змінює event names."
    validation: "Має відповідати logger naming convention із `LOGGING_POLICY.md`."

  - name: "`LoggerLevelOverride.level`"
    type: "`std::string`"
    required: true
    default: "немає"
    purpose: "Level для logger override."
    affects: "Threshold конкретного logger namespace."
    does_not_affect: "Не змінює severity, яку code обрав для конкретного event."
    validation: "Має бути одним із `TRACE`, `DEBUG`, `INFO`, `WARN`, `ERROR`, `FATAL`."
```

## Input / Output

Input:

- application configuration authoring file;
- log4cxx configuration file, на який посилається `logging.config_file`;
- `PROJECT_ECOSYSTEM.md` як джерело версії log4cxx;
- `LOGGING_POLICY.md` як джерело behavioral rules.

Output:

- typed `LoggingConfig`;
- startup/bootstrap logging setup;
- bounded runtime logging behavior.

## Constraints

- `logging` не має містити stage parameters.
- `logging` не має містити algorithm thresholds.
- `logging` не має містити input/processing route selection.
- `logging.config_file` не є повною canonical configuration.
- Invalid `logging.default_level` має бути відхилений або явно reported.
- `async.blocking = true` не дозволений для real-time processing threads без
  explicit approval.
- `logger_overrides.logger` має використовувати canonical logger names із
  `LOGGING_POLICY.md`.
- Unbounded string fields заборонені.
- Logging defaults мають бути безпечними для real-time operation.

## Interpretation

Ця картка є child card для `ApplicationConfig.logging`. Вона не замінює root
`ApplicationConfig` і не створює окрему configuration plane поза application
runtime config.

## Failure cases

- Logging config додано в `PipelineConfig C`.
- Stage implementation читає logging config для вибору algorithm behavior.
- `DEBUG` або `TRACE` увімкнено для hot path без sampling/rate limiting.
- Invalid level silently falls back to verbose logging.
- Async appender blocks real-time processing thread.
- Full camera URI або token потрапляє в log через config.
- log4cxx XML використовується як єдине джерело canonical semantics.
- Per-frame/per-tile limits відсутні або дорівнюють unbounded.

## Typical misuse

- Додавати `logging_enabled` у stage parameters.
- Створювати logger override для випадкового file-based logger name.
- Вважати `logging.enabled=false` способом вимкнути diagnostics/status
  handling.
- Використовувати logging config як validation/output configuration.
- Дублювати level thresholds у коді.

## Open questions

- Final default path for DP1 application config file.
- Чи `realtime_profile` має бути enum або string registry.
- Exact allowed values for `discard_policy`.
- Exact registry for `logger_overrides.logger`.
- Чи `max_messages_per_frame` і `max_messages_per_tile` належать до logging
  config або runtime diagnostic budget.
- Чи `config_file` має бути path, URI або config resource id.
- Чи `rate_limit_per_event_per_sec = 0` означає disabled або unlimited.

## Connections

- parent_config: dp1.config.application
- defines_subsection: ApplicationConfig.logging
- governed_by: project-knowledge/00-governance/LOGGING_POLICY.md
- references: project-knowledge/01-project/PROJECT_ECOSYSTEM.md
- uses_context_fields_from: dp1.domain.runtime.frame_context
- uses_context_fields_from: dp1.domain.runtime.tile_context
- related_protocol_boundary: protocols.dp1_dp2.measurement_handoff
- separates_from: dp1.config.pipeline_configuration_c
