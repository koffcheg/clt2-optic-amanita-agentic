---
id: dp1.config.application.profiling
title:
  uk: "Canonical-конфігурація profiling застосунку DP1"
  en: "Canonical DP1 application profiling configuration"
tags: [dp1, canonical, config, application, runtime, profiling]
kind: config-card
source_role: canonical
source:
  file: "project-knowledge/02-dp1/canonical/configuration/dp1.config.application.profiling.md"
status: "draft"
---

## Definition

`ApplicationConfig.profiling` є canonical runtime-конфігурацією profiling для
застосунку DP1.

Ця картка визначає typed configuration surface для profiling section. Root
`ApplicationConfig` визначає `dp1.config.application`.

## Assumptions

- Profiling керує runtime measurement, bounded trace, aggregation і report
  emission навколо pipeline.
- Налаштування profiling не мають впливати на алгоритмічні результати.
- Profiling structures визначає `dp1.domain.profiling`.
- External profiling tools можуть використовуватися тільки за окремою explicit
  instruction або approved task.
- Logging configuration визначає окрема картка
  `dp1.config.application.logging`.

## Theorem / Contract

Profiling configuration не має вибирати порядок етапів, `stage.variant`,
`stage.level`, алгоритмічні параметри, input route, processing route, семантику
вимірювань або семантику DP1 -> DP2 payload.

Базова форма JSON authoring:

```json
{
  "profiling": {
    "enabled": true,
    "mode": "lightweight",
    "levels": ["P0", "P1", "P2", "P4", "P5"],
    "aggregation_window_frames": 300,
    "raw_trace": {
      "enabled": false,
      "max_frames": 0,
      "max_events_per_frame": 64
    },
    "operation_timing": {
      "enabled": false,
      "include_format_conversions": true,
      "include_memory_copies": true,
      "include_allocations": false
    },
    "reports": {
      "emit_frame_reports": false,
      "emit_window_summary": true,
      "emit_run_summary": true,
      "format": "json",
      "output_dir": ""
    },
    "logging_bridge": {
      "emit_aggregated_summaries": true,
      "summary_every_n_frames": 300,
      "emit_budget_warnings": true
    },
    "external_trace": {
      "enabled": false,
      "backend": "none"
    }
  }
}
```

Typed canonical config model для code generation має існувати незалежно від
JSON representation. JSON є serialization/authoring form, але C++ генерація
має спиратися на typed fields:

```cpp
struct ProfilingRawTraceConfig {
    bool enabled = false;
    int max_frames = 0;
    int max_events_per_frame = 64;
};

struct ProfilingOperationTimingConfig {
    bool enabled = false;
    bool include_format_conversions = true;
    bool include_memory_copies = true;
    bool include_allocations = false;
};

struct ProfilingReportsConfig {
    bool emit_frame_reports = false;
    bool emit_window_summary = true;
    bool emit_run_summary = true;
    std::string format = "json";
    std::string output_dir;
};

struct ProfilingLoggingBridgeConfig {
    bool emit_aggregated_summaries = true;
    int summary_every_n_frames = 300;
    bool emit_budget_warnings = true;
};

struct ProfilingExternalTraceConfig {
    bool enabled = false;
    std::string backend = "none";
};

struct ProfilingConfig {
    bool enabled = true;
    std::string mode = "lightweight";
    std::vector<std::string> levels;
    int aggregation_window_frames = 300;
    ProfilingRawTraceConfig raw_trace;
    ProfilingOperationTimingConfig operation_timing;
    ProfilingReportsConfig reports;
    ProfilingLoggingBridgeConfig logging_bridge;
    ProfilingExternalTraceConfig external_trace;
};
```

## Fields / Interface

```yaml
fields:
  - name: "`profiling`"
    type: "`ProfilingConfig`"
    required: true
    default: "якщо секція відсутня, configuration invalid"
    purpose: "Містить canonical runtime profiling settings для measurement, bounded trace, aggregation, reports і optional external trace boundary."
    affects: "Runtime profiling enablement, selected profiling mode, retained raw trace bounds, aggregation windows, report emission і logging bridge для summaries."
    does_not_affect: "Не змінює порядок stages, stage variants, stage levels, algorithm parameters, processing route або DP1 output semantics."
    validation: "Має пройти validation усіх вкладених полів `ProfilingConfig`; invalid profiling config має бути rejected або explicitly reported."

  - name: "`profiling.enabled`"
    type: "`bool`"
    required: true
    default: "`true`"
    purpose: "Вмикає canonical runtime profiling surface."
    affects: "Чи runtime створює profiling context, summaries і дозволені profiling records."
    does_not_affect: "Не вимикає stage status, diagnostics, error handling або algorithm execution."
    validation: "Тільки boolean value; `false` не має silent-disable mandatory lightweight profiling, якщо runtime/product profile його вимагає."

  - name: "`profiling.mode`"
    type: "`std::string`"
    required: true
    default: "`lightweight`"
    purpose: "Оголошує expected profiling cost/detail mode."
    affects: "Default enabled levels, operation timing policy, raw trace retention і report detail."
    does_not_affect: "Не вибирає algorithm complexity level `L0|L1|L2|L3|Lx`."
    validation: "Allowed values: `disabled`, `lightweight`, `detailed`, `external_trace`; `external_trace` потребує explicit approved task."

  - name: "`profiling.levels`"
    type: "`std::vector<std::string>`"
    required: true
    default: "`[\"P0\", \"P1\", \"P2\", \"P4\", \"P5\"]`"
    purpose: "Список enabled profiling levels."
    affects: "Які profiling structures runtime має заповнювати або агрегувати."
    does_not_affect: "Не змінює algorithm complexity levels і не є stage registry."
    validation: "Кожне значення має бути одним із `P0`, `P1`, `P2`, `P3`, `P4`, `P5`; duplicate values заборонені."

  - name: "`profiling.aggregation_window_frames`"
    type: "`int`"
    required: true
    default: "`300`"
    purpose: "Задає bounded frame window для runtime aggregation."
    affects: "Частоту і memory footprint для frame/stage summaries."
    does_not_affect: "Не змінює frame processing behavior."
    validation: "Має бути `> 0`; занадто велике значення має бути rejected або capped implementation policy."

  - name: "`profiling.raw_trace`"
    type: "`ProfilingRawTraceConfig`"
    required: true
    default: "див. nested defaults"
    purpose: "Керує retention raw profiling events."
    affects: "Чи зберігаються raw frame/stage/operation events beyond aggregation."
    does_not_affect: "Не вмикає logging of raw events."
    validation: "Nested fields мають бути valid; raw trace must remain bounded."

  - name: "`profiling.raw_trace.enabled`"
    type: "`bool`"
    required: true
    default: "`false`"
    purpose: "Дозволяє retained raw profiling trace."
    affects: "Чи runtime зберігає recent raw events для report/debug."
    does_not_affect: "Не вимикає required aggregation summaries."
    validation: "Тільки boolean value; `true` requires finite `max_frames` або equivalent bounded ring buffer."

  - name: "`profiling.raw_trace.max_frames`"
    type: "`int`"
    required: true
    default: "`0`"
    purpose: "Максимальна кількість frames із raw trace retention."
    affects: "Memory bound для retained raw frame traces."
    does_not_affect: "Не задає кількість processed frames у pipeline."
    validation: "`0` означає no raw frame retention; якщо `raw_trace.enabled=true`, значення має бути `> 0` або implementation має rejected config."

  - name: "`profiling.raw_trace.max_events_per_frame`"
    type: "`int`"
    required: true
    default: "`64`"
    purpose: "Максимальна кількість raw profiling events на frame."
    affects: "Bound для `ProfilingTrace` у `FrameContext` або tile aggregation."
    does_not_affect: "Не дозволяє per-pixel/per-contour-point event spam."
    validation: "Має бути `> 0`; recommended default `64`; unbounded values forbidden."

  - name: "`profiling.operation_timing`"
    type: "`ProfilingOperationTimingConfig`"
    required: true
    default: "див. nested defaults"
    purpose: "Керує operation-level timing і lightweight accounting."
    affects: "Чи runtime створює P3 operation timings і які hidden costs мають бути counted."
    does_not_affect: "Не змінює OpenCV operation choices або algorithm route."
    validation: "Nested fields мають бути valid."

  - name: "`profiling.operation_timing.enabled`"
    type: "`bool`"
    required: true
    default: "`false`"
    purpose: "Вмикає detailed operation-level timing."
    affects: "Чи створюються окремі P3 `OperationTiming` records."
    does_not_affect: "Не прибирає operation cost із parent stage timing."
    validation: "Тільки boolean value; `true` має залишатися bounded і не додавати timers у pixel loops."

  - name: "`profiling.operation_timing.include_format_conversions`"
    type: "`bool`"
    required: true
    default: "`true`"
    purpose: "Вимагає timing або counting для format conversions."
    affects: "Visibility of `convertTo`, normalization або route conversion costs."
    does_not_affect: "Не дозволяє implicit format conversions у algorithm code."
    validation: "Для canonical DP1 recommended `true`; `false` allowed only with explicit profiling exception."

  - name: "`profiling.operation_timing.include_memory_copies`"
    type: "`bool`"
    required: true
    default: "`true`"
    purpose: "Вимагає timing або counting для large copies."
    affects: "Visibility of `copyTo`, `clone`, tile/full-frame copy costs."
    does_not_affect: "Не дозволяє hidden full-frame temporary buffers."
    validation: "Для canonical DP1 recommended `true`; `false` allowed only with explicit profiling exception."

  - name: "`profiling.operation_timing.include_allocations`"
    type: "`bool`"
    required: true
    default: "`false`"
    purpose: "Дозволяє lightweight allocation-related accounting when available."
    affects: "Чи runtime records temporary buffer/allocation counters."
    does_not_affect: "Не замінює `heaptrack` або інший offline allocation profiler."
    validation: "Тільки boolean value; detailed allocation tracing requires explicit approved task."

  - name: "`profiling.reports`"
    type: "`ProfilingReportsConfig`"
    required: true
    default: "див. nested defaults"
    purpose: "Керує persisted або emitted profiling reports."
    affects: "Frame report, aggregation window summary і run summary emission."
    does_not_affect: "Не створює log lines per profiling event."
    validation: "Nested fields мають бути valid; report emission must remain bounded."

  - name: "`profiling.reports.emit_frame_reports`"
    type: "`bool`"
    required: true
    default: "`false`"
    purpose: "Дозволяє per-frame structured profiling reports."
    affects: "Чи runtime може emit/report frame-level details."
    does_not_affect: "Не вмикає default per-frame logging."
    validation: "`false` for real-time default; `true` requires bounded output route and explicit report policy."

  - name: "`profiling.reports.emit_window_summary`"
    type: "`bool`"
    required: true
    default: "`true`"
    purpose: "Вмикає aggregation-window summaries."
    affects: "Чи runtime emits/reports P5 summaries every aggregation window."
    does_not_affect: "Не змінює aggregation computation requirement when profiling is enabled."
    validation: "Тільки boolean value."

  - name: "`profiling.reports.emit_run_summary`"
    type: "`bool`"
    required: true
    default: "`true`"
    purpose: "Вмикає run-level profiling summary."
    affects: "Чи shutdown/final report includes run profile summary."
    does_not_affect: "Не вмикає external profiler."
    validation: "Тільки boolean value; recommended `true` для reproducibility."

  - name: "`profiling.reports.format`"
    type: "`std::string`"
    required: true
    default: "`json`"
    purpose: "Формат persisted profiling reports."
    affects: "Serialization target for structured summaries."
    does_not_affect: "Не визначає internal DTO layout."
    validation: "Allowed values: `json`, `csv`, `none`; `csv` only for tabular summaries."

  - name: "`profiling.reports.output_dir`"
    type: "`std::string`"
    required: false
    default: "empty string"
    purpose: "Optional output directory або resource id для profiling reports."
    affects: "Куди report writer може зберігати profiling artifacts."
    does_not_affect: "Не змінює frame processing і не є dataset path."
    validation: "Empty means default/no persisted output depending on report mode; path must not contain credentials."

  - name: "`profiling.logging_bridge`"
    type: "`ProfilingLoggingBridgeConfig`"
    required: true
    default: "див. nested defaults"
    purpose: "Керує тим, які aggregated profiling facts можуть бути emitted через logging."
    affects: "Bounded summary logs and budget warning logs."
    does_not_affect: "Не перетворює raw profiling events на log events."
    validation: "Nested fields мають бути valid і узгоджені з `LOGGING_POLICY.md`."

  - name: "`profiling.logging_bridge.emit_aggregated_summaries`"
    type: "`bool`"
    required: true
    default: "`true`"
    purpose: "Дозволяє log emission для aggregated profiling summaries."
    affects: "Чи logging може містити human-readable profiling summary."
    does_not_affect: "Не створює profiling data store у log4cxx."
    validation: "Тільки boolean value; emission frequency controlled by `summary_every_n_frames`."

  - name: "`profiling.logging_bridge.summary_every_n_frames`"
    type: "`int`"
    required: true
    default: "`300`"
    purpose: "Sampling interval для summary logs."
    affects: "Максимальну частоту aggregated profiling summary logs."
    does_not_affect: "Не змінює aggregation_window_frames unless explicitly tied by implementation."
    validation: "Має бути `>= 0`; `0` disables periodic summary logging."

  - name: "`profiling.logging_bridge.emit_budget_warnings`"
    type: "`bool`"
    required: true
    default: "`true`"
    purpose: "Дозволяє bounded warning logs for repeated budget misses."
    affects: "Чи deadline/budget violations may be reported through logging."
    does_not_affect: "Не змінює deadline calculation або stage budget."
    validation: "Тільки boolean value; warnings must remain rate-limited by logging policy."

  - name: "`profiling.external_trace`"
    type: "`ProfilingExternalTraceConfig`"
    required: true
    default: "див. nested defaults"
    purpose: "Описує optional external trace integration boundary."
    affects: "Чи approved implementation may enable trace hooks for external profiler timeline."
    does_not_affect: "Не додає dependency або profiler API без explicit approved task."
    validation: "Nested fields мають бути valid; default must be disabled."

  - name: "`profiling.external_trace.enabled`"
    type: "`bool`"
    required: true
    default: "`false`"
    purpose: "Вмикає optional external trace integration only when approved."
    affects: "Чи runtime may activate external trace hooks."
    does_not_affect: "Не запускає external profiler tool і не робить його dependency."
    validation: "`true` valid only for separately approved task/instruction."

  - name: "`profiling.external_trace.backend`"
    type: "`std::string`"
    required: true
    default: "`none`"
    purpose: "Назва external trace backend."
    affects: "Trace hook adapter selection when external tracing is explicitly approved."
    does_not_affect: "Не замінює canonical runtime profiling structures."
    validation: "Allowed values: `none`, `tracy`, `perfetto`; non-`none` requires `external_trace.enabled=true` and explicit approval."
```

## Input / Output

Input:

- application configuration authoring file;
- `PROFILING_POLICY.md` як джерело behavioral rules для profiling;
- `dp1.domain.profiling` як джерело profiling structures і bounds semantics.

Output:

- typed `ProfilingConfig`;
- bounded runtime profiling setup, trace retention, aggregation і report policy.

## Constraints

- `profiling` не має містити stage parameters.
- `profiling` не має містити algorithm thresholds.
- `profiling` не має містити input/processing route selection.
- `profiling` не має створювати DP1 -> DP2 payload fields.
- `profiling.external_trace` не має додавати dependency або profiler API без
  explicit approved task.
- Profiling defaults мають бути bounded і safe для real-time operation.

## Interpretation

Ця картка є child card для `ApplicationConfig.profiling`. Вона не замінює root
`ApplicationConfig` і не створює окрему configuration plane поза application
runtime config.

## Failure cases

- Profiling config додано в `PipelineConfig C` як operational switch.
- Stage implementation читає profiling config для вибору algorithm behavior.
- Raw profiling trace увімкнено без finite bound.
- External trace backend увімкнено без окремого approved task.

## Typical misuse

- Додавати `profiling_enabled` у stage parameters.
- Використовувати profiling config як спосіб змінити algorithm route.
- Вважати `profiling.enabled=false` дозволом прибрати required lightweight
  runtime summaries без product-level approval.

## Open questions

- Exact default path/resource id для profiling reports.
- Чи `profiling.mode` має бути enum або string registry.
- Exact registry для non-core external trace backend names.

## Connections

- parent_config: dp1.config.application
- defines_subsection: ApplicationConfig.profiling
- governed_by: project-knowledge/00-governance/PROFILING_POLICY.md
- uses: dp1.domain.profiling
- uses_context_fields_from: dp1.domain.runtime.frame_context
- uses_context_fields_from: dp1.domain.runtime.tile_context
- separates_from: dp1.config.pipeline_configuration_c
