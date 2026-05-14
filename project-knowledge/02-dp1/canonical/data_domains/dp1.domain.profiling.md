---
id: dp1.domain.profiling
title:
  uk: "Canonical-домен профілювання DP1"
  en: "DP1 canonical profiling domain"
tags: [dp1, canonical, data-domain, runtime, profiling, code-generation]
kind: data-domain-card
source_role: canonical
source:
  file: "project-knowledge/02-dp1/canonical/data_domains/dp1.domain.profiling.md"
status: "draft"
---

## Definition

Ця картка визначає canonical profiling domain для DP1 runtime. Profiling domain
містить службові структури для вимірювання часу, обліку обсягу даних, обліку
копій/перетворень, агрегації і звітів.

Profiling domain не є logging, benchmarking, optimization або DP1 -> DP2
payload.

## Assumptions

- Profiling має вимірювати runtime behavior без зміни algorithm behavior.
- Canonical runtime може використовувати `std::chrono::steady_clock` для
  duration measurement, але persisted records мають мати explicit clock
  semantics через `TimestampRef`.
- `dp1.config.application.profiling` задає runtime-перемикачі, bounds, reports
  і external trace policy.
- `PipelineConfig C` задає processing route, stage variants, levels і
  algorithm parameters, але не є власником operational profiling switches.
- Core DP1 stage keys походять із `dp1.config.stage_variant_registry`, але
  profiling identity має залишатися extensible для майбутніх stages або
  infrastructure scopes.
- Profiling records не мають містити image buffers, masks, candidates, segments,
  validated objects або measurements як hidden payload.

## Theorem / Contract

Profiling domain визначає рівні профілювання:

```yaml
profiling_levels:
  - id: "P0"
    name: "Run-level profiling"
    purpose: "Execution context, active configuration reference, runtime metadata."
  - id: "P1"
    name: "Frame-level profiling"
    purpose: "Frame latency and deadline compliance."
  - id: "P2"
    name: "Stage-level profiling"
    purpose: "Execution time of each canonical DP1 stage."
  - id: "P3"
    name: "Operation-level profiling"
    purpose: "Selected expensive operations inside a stage."
  - id: "P4"
    name: "Data-volume / cardinality profiling"
    purpose: "Counters that explain runtime cost."
  - id: "P5"
    name: "Aggregated runtime statistics"
    purpose: "Summary statistics across frames and stage calls."
```

`P0`-`P5` не є algorithm complexity levels. Complexity levels залишаються
`L0`, `L1`, `L2`, `L3`, `Lx`.

Canonical C++ shape нижче є knowledge-level contract. Future implementation task
може змінити storage layout, але не має змінювати semantics без оновлення цієї
картки.

```cpp
enum class ProfilingLevel : std::uint8_t {
    P0Run,
    P1Frame,
    P2Stage,
    P3Operation,
    P4Cardinality,
    P5Aggregation
};

enum class ProfilingMode : std::uint8_t {
    Disabled,
    Lightweight,
    Detailed,
    ExternalTrace
};

struct StageKey {
    std::string id;
};

struct OperationKey {
    std::string id;
};
```

`StageKey.id` для core DP1 stages має збігатися з canonical stage id із
`dp1.config.stage_variant_registry`. Дозволені core stage keys:

```yaml
core_stage_keys:
  - prep
  - radiometric_correction
  - enhancement
  - matched_filtering
  - candidate_extraction
  - segmentation_refinement
  - object_filtering
  - measurement
```

`StageKey` навмисно не є closed permanent enum. Implementation може мати enum,
index або interned id для hot path, але canonical record має бути відтворювано
mapped до stable stage key.

Built-in operation keys:

```yaml
operation_keys:
  - convert_to
  - normalize
  - copy_to
  - clone
  - resize
  - allocation
  - reallocation
  - tile_split
  - tile_merge
  - gaussian_blur
  - median_blur
  - bilateral_filter
  - filter_2d
  - match_template
  - threshold
  - adaptive_threshold
  - morphology_ex
  - find_contours
  - connected_components
  - moments
  - bounding_rect
  - min_area_rect
```

Additional operation keys may be added by approved Project Knowledge change or
by an implementation-local registry if they remain stable, bounded, and mapped
to a parent stage.

P0 run-level structure:

```cpp
struct RunProfile {
    std::string run_id;
    std::string schema_version;
    std::string build_id;
    std::string rt_profile;
    std::uint32_t frame_width = 0;
    std::uint32_t frame_height = 0;
    PixelFormat input_format{};
    InputBitDepth input_bit_depth{};
    const PipelineConfig* config_ref = nullptr;
    const ApplicationConfig* application_config_ref = nullptr;
};
```

P1 frame-level structure:

```cpp
struct FrameTiming {
    std::uint64_t frame_id = 0;
    int camera_id = -1;
    TimestampRef start_time;
    TimestampRef end_time;
    std::int64_t duration_ns = 0;
    std::int64_t deadline_ns = 0;
    bool deadline_missed = false;
    bool dropped_or_skipped = false;
    std::uint32_t executed_stage_count = 0;
};
```

P2 stage-level structure:

```cpp
struct StageTiming {
    StageKey stage_key;
    StageStatus status = StageStatus::Ok;
    std::string variant;
    std::string level;
    PixelFormat input_format{};
    PixelFormat output_format{};
    TimestampRef start_time;
    TimestampRef end_time;
    std::int64_t duration_ns = 0;
};
```

P3 operation-level structure:

```cpp
struct OperationTiming {
    StageKey parent_stage;
    OperationKey operation_key;
    PixelFormat input_format{};
    PixelFormat output_format{};
    std::uint64_t bytes_processed = 0;
    std::int64_t duration_ns = 0;
};
```

P4 cardinality structure:

```cpp
struct CardinalityMetrics {
    std::uint32_t roi_count = 0;
    std::uint32_t tile_count = 0;
    std::uint32_t candidate_count = 0;
    std::uint32_t segment_count = 0;
    std::uint32_t contour_count = 0;
    std::uint32_t connected_component_count = 0;
    std::uint32_t input_object_count = 0;
    std::uint32_t rejected_object_count = 0;
    std::uint32_t validated_object_count = 0;
    std::uint32_t measurement_count = 0;
    std::uint32_t full_frame_pass_count = 0;
};
```

Memory-related counters:

```cpp
struct MemoryMetrics {
    std::uint32_t clone_count = 0;
    std::uint32_t copy_to_count = 0;
    std::uint32_t convert_to_count = 0;
    std::uint32_t temporary_buffer_count = 0;
    std::uint64_t estimated_bytes_copied = 0;
    std::uint64_t estimated_temporary_bytes = 0;
};
```

P5 aggregation structures:

```cpp
struct DurationSummary {
    std::uint64_t calls = 0;
    std::int64_t min_ns = 0;
    std::int64_t avg_ns = 0;
    std::int64_t median_ns = 0;
    std::int64_t p95_ns = 0;
    std::int64_t p99_ns = 0;
    std::int64_t max_ns = 0;
};

struct StageProfileSummary {
    StageKey stage_key;
    std::string variant;
    std::string level;
    DurationSummary duration;
    std::uint64_t budget_exceed_count = 0;
};

struct FrameProfileSummary {
    DurationSummary frame_duration;
    std::uint64_t frame_count = 0;
    std::uint64_t deadline_miss_count = 0;
    std::uint64_t dropped_or_skipped_count = 0;
};

struct RunProfileSummary {
    std::string run_id;
    FrameProfileSummary frames;
    std::vector<StageProfileSummary> stages;
    CardinalityMetrics average_cardinality;
    CardinalityMetrics maximum_cardinality;
    MemoryMetrics memory_metrics;
};
```

Trace and report structures:

```cpp
struct ProfileEvent {
    std::optional<FrameTiming> frame_timing;
    std::optional<StageTiming> stage_timing;
    std::optional<OperationTiming> operation_timing;
};

struct ProfilingTrace {
    std::vector<ProfileEvent> events;
    std::uint32_t max_events = 0;
};

struct FrameProfiling {
    FrameTiming frame_timing;
    ProfilingTrace trace;
    CardinalityMetrics cardinality;
    MemoryMetrics memory_metrics;
};

struct TileProfiling {
    int tile_id = -1;
    int worker_id = -1;
    ProfilingTrace trace;
    CardinalityMetrics cardinality;
    MemoryMetrics memory_metrics;
};

struct TileProfilingResult {
    int tile_id = -1;
    int worker_id = -1;
    std::vector<StageProfileSummary> stages;
    CardinalityMetrics cardinality;
    MemoryMetrics memory_metrics;
};

struct FrameProcessingReport {
    std::string schema_version;
    std::string run_id;
    std::uint64_t frame_id = 0;
    const PipelineConfig* config_ref = nullptr;
    FrameTiming frame_timing;
    std::vector<StageTiming> stage_timings;
    CardinalityMetrics cardinality;
    MemoryMetrics memory_metrics;
};
```

Усі duration fields мають містити одиницю в назві (`_ns`, `_us`, `_ms`) або
мати schema metadata з одиницями. Canonical default для runtime durations -
nanoseconds.

`ProfilingTrace.events` має бути bounded через
`dp1.config.application.profiling`.
Unbounded per-frame raw traces заборонені.

## Fields / Interface

```yaml
fields:
  - name: "`StageKey.id`"
    type: "`std::string` або implementation-local interned id"
    purpose: "Stable stage identity для profiling records."
    validation: "Core DP1 stages мають збігатися з `dp1.config.stage_variant_registry`; extension keys потребують approved registry/boundary."

  - name: "`OperationKey.id`"
    type: "`std::string` або implementation-local interned id"
    purpose: "Stable operation identity для selected expensive operations."
    validation: "Має бути bounded stable value, не runtime-formatted string."

  - name: "`RunProfile.config_ref`"
    type: "`const PipelineConfig*` або stable config snapshot reference"
    purpose: "Відтворюваність runtime measurement."
    validation: "Не має бути null під час run-level report, якщо configuration доступна."

  - name: "`RunProfile.application_config_ref`"
    type: "`const ApplicationConfig*` або stable config snapshot reference"
    purpose: "Фіксує active profiling/logging runtime settings."
    validation: "Не має використовуватись stage code для algorithm branching."

  - name: "`FrameTiming.duration_ns`"
    type: "`std::int64_t`"
    purpose: "Total frame processing duration."
    validation: "Має бути computed from monotonic runtime clock."

  - name: "`StageTiming.variant`"
    type: "`std::string`"
    purpose: "Stage implementation family з active `PipelineConfig C`."
    validation: "Не має змішуватися з `level`."

  - name: "`StageTiming.level`"
    type: "`std::string`"
    purpose: "Algorithm complexity level з active `PipelineConfig C`."
    validation: "Має бути одним із canonical complexity levels."

  - name: "`OperationTiming.bytes_processed`"
    type: "`std::uint64_t`"
    purpose: "Approximate data volume for expensive operation."
    validation: "0 дозволено, якщо bytes estimate unavailable, але operation cost має залишатися у parent stage timing."

  - name: "`CardinalityMetrics`"
    type: "plain numeric counters"
    purpose: "Пояснює runtime cost через обсяг даних."
    validation: "Counters frame-scoped unless explicit tile/run scope is stated."

  - name: "`MemoryMetrics`"
    type: "plain numeric counters"
    purpose: "Lightweight accounting для copies, conversions і temporary buffers."
    validation: "Не замінює offline allocation profiler."

  - name: "`ProfilingTrace.max_events`"
    type: "`std::uint32_t`"
    purpose: "Bound для raw event retention."
    validation: "0 означає disabled raw event retention або implementation-defined no-retention mode; не означає unbounded."

  - name: "`FrameProfiling`"
    type: "frame-scoped profiling aggregate"
    purpose: "Єдиний тип для поля `FrameContext.profiling`."
    validation: "Містить frame timing, bounded trace, cardinality і memory metrics."

  - name: "`TileProfiling`"
    type: "tile execution profiling aggregate"
    purpose: "Єдиний тип для поля `TileContext.profiling`."
    validation: "Містить tile/worker identity, bounded trace, cardinality і memory metrics."

  - name: "`TileProfilingResult`"
    type: "tile result profiling aggregate"
    purpose: "Єдиний тип для поля `TileResult.profiling`."
    validation: "Містить bounded summary для merge/report path і не переносить raw image payload."
```

## Input / Output

Input:

- `dp1.config.application.profiling`;
- `PipelineConfig C`;
- `FrameContext`;
- `TileContext`;
- stage-interface contracts;
- `dp1.domain.time`;
- `dp1.config.stage_variant_registry`.

Output:

- bounded runtime profiling records;
- frame, tile, stage, operation, cardinality, and memory summaries;
- JSON/CSV-compatible reports when explicitly configured;
- aggregated log summaries only through logging boundary.

## Constraints

- Profiling domain не є computation domain.
- Profiling domain не є logging storage.
- Profiling domain не є DP1 -> DP2 protocol payload.
- Profiling records не мають переносити primary image buffers або stage outputs.
- `StageKey` для core stages має бути registry-backed.
- `variant` і `level` мають записуватися окремо.
- Runtime durations мають використовувати monotonic source.
- Persisted timestamps мають мати explicit clock semantics.
- Raw traces і aggregation windows мають бути bounded.
- External profiler integration не є частиною core profiling domain без окремого approved task.

## Interpretation

Profiling domain є службовим runtime support layer. Він дозволяє пояснити,
скільки часу займав кадр, який stage витратив budget, яка operation створила
cost, і який обсяг даних пояснює runtime behavior.

Profiling domain має бути достатньо формальним для future code generation, але
не має визначати algorithm semantics або замінювати stage specs.

## Failure cases

- Stage timings зберігаються як free-form log strings.
- `StageKey` не збігається з registry для core DP1 stage.
- `L0`, `L1`, `L2` використовуються як profiling levels.
- `variant` і `level` змішані в одному полі.
- `FrameContext` зберігає image payload як profiling artifact.
- Raw trace росте без bound.
- Average latency подається як достатня real-time evidence без `p95`, `p99` і `max`.
- External profiler hook додається в core stage code без explicit approval.

## Typical misuse

- Використовувати profiling data для передачі algorithm outputs між stages.
- Додавати `profiling_enabled` у stage parameters замість
  `dp1.config.application.profiling`.
- Створювати operation event для кожного pixel або contour point.
- Використовувати log4cxx як profiling database.
- Додавати closed permanent enum `StageId`, який блокує future stage extension.

## Open questions

- Exact binary/storage layout для implementation DTO.
- Exact JSON schema для persisted profiling reports.
- Exact percentile aggregation algorithm.
- Formal registry для non-core infrastructure profiling scopes.

## Connections

- governed_by: project-knowledge/00-governance/PROFILING_POLICY.md
- configured_by: dp1.config.application.profiling
- root_config: dp1.config.application
- references: dp1.config.pipeline_configuration_c
- references: dp1.config.stage_variant_registry
- constrained_by: dp1.domain.time
- constrained_by: dp1.domain.runtime
- constrained_by: dp1.domain.memory_ownership
- constrained_by: dp1.domain.conversion_rules
- used_by: dp1.domain.runtime.frame_context
- used_by: dp1.domain.runtime.tile_context
- used_by: dp1.domain.runtime.tile_result
- used_by: dp1.pipeline.stage_contract
- used_by: dp1.validation.stage_contract_checks
