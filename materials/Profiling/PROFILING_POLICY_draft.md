# PROFILING_POLICY.md

## Purpose

This file defines runtime profiling rules for AI-assisted code changes in the
real-time C++20 computer vision project.

Profiling is a runtime measurement and aggregation mechanism. It must expose
frame, stage, operation, data-volume, and aggregate timing behavior without
changing the algorithmic behavior of the frame processing pipeline.

Core rule:

```text
Profiling must measure real-time behavior without becoming part of the algorithm.
```

Profiling is not logging. Logging emits diagnostic messages. Profiling records
structured numeric measurements that can later be aggregated, reported, compared,
and used to find bottlenecks.

Profiling is not benchmarking. Runtime profiling observes actual pipeline
execution. Benchmarking runs controlled experiments for functions, stages,
variants, frame sizes, pixel formats, or complexity levels.

Profiling is not optimization. Profiling provides evidence for optimization. An
AI coding agent must not claim a performance improvement without a before/after
measurement.

## Status

Status: active.

This document is an active governance document for profiling-related code
changes.

## Scope

This policy applies to:

- C++20 production code;
- OpenCV-based image-processing code;
- canonical DP1 runtime code;
- DP1 stage implementations;
- `FrameContext` and runtime-domain structures;
- `StageTiming`, profiling trace, profiling summaries, and frame reports;
- format-conversion and memory-copy timing;
- performance-related validation runs;
- profiling code generated or modified by an AI coding agent;
- profiling-related configuration changes when explicitly requested by the
  current task.

This policy does not define the full testing strategy. It does not introduce a
new mandatory external profiler. It does not replace `CODE_STYLE.md`,
`TESTING_POLICY.md`, `LOGGING_POLICY.md`, stage contracts, data-domain cards,
or task-card rules.

If this policy conflicts with explicit user instructions in the current task,
follow the `AGENTS.md` source-of-truth order and report the conflict.

## Related Project Knowledge

Read this policy together with:

- `AGENTS.md`;
- `project-knowledge/00-governance/CODE_STYLE.md`;
- `project-knowledge/00-governance/TESTING_POLICY.md`;
- `project-knowledge/00-governance/LOGGING_POLICY.md`;
- `project-knowledge/01-project/PROJECT_ECOSYSTEM.md`;
- `project-knowledge/02-dp1/canonical/DP1_CANONICAL_INDEX.md`;
- `project-knowledge/02-dp1/canonical/data_domains/dp1.domain.time.md`;
- `project-knowledge/02-dp1/canonical/data_domains/dp1.domain.runtime.md`;
- `project-knowledge/02-dp1/canonical/data_domains/dp1.domain.memory_ownership.md`;
- `project-knowledge/02-dp1/canonical/data_domains/dp1.domain.conversion_rules.md`;
- `project-knowledge/02-dp1/canonical/data_domains/dp1.domain.opencv_invariants.md`;
- `project-knowledge/02-dp1/canonical/data_domains/structures/runtime/dp1.domain.runtime.frame_context.md`;
- `project-knowledge/02-dp1/canonical/pipeline/dp1.pipeline.stage_contract.md`;
- `project-knowledge/02-dp1/canonical/pipeline/dp1.pipeline.stage_io_matrix.md`;
- `project-knowledge/02-dp1/canonical/configuration/dp1.config.pipeline_configuration_c.md`;
- `project-knowledge/02-dp1/canonical/configuration/dp1.config.complexity_levels.md`;
- relevant stage cards and stage specifications.

This policy uses `P0`-`P5` for profiling levels. These are not algorithm
complexity levels. Algorithm complexity levels remain `L0`, `L1`, `L2`, `L3`,
and `Lx` as defined by the DP1 configuration knowledge.

## Definitions

| Term | Meaning |
|---|---|
| Runtime profiling | Instrumentation inside the running pipeline that records timing, counters, and aggregation data. |
| Frame-level profiling | Measurement of full processing latency for one frame or one local processing route. |
| Stage-level profiling | Measurement of one canonical DP1 stage execution. |
| Operation-level profiling | Measurement of selected expensive operations inside a stage, such as `convertTo`, `copyTo`, `GaussianBlur`, or `findContours`. |
| Cardinality metrics | Data-volume counters that explain runtime cost, such as tile count, candidate count, contour count, or object count. |
| Aggregated profile | Summary statistics across many frames or stage calls: `min`, `avg`, `median`, `p95`, `p99`, `max`, and budget misses. |
| Hot path | Per-frame, per-tile, per-object, per-contour, or per-pixel code where extra overhead can affect real-time behavior. |
| Lightweight profiling | Always-available low-overhead profiling suitable for production or near-production runs. |
| Detailed profiling | Configurable profiling with operation-level timing, memory-copy accounting, and optional trace integration. |

## Profiling Technology

### Runtime timing

Use `std::chrono::steady_clock` as the default time source for runtime duration
measurement.

`steady_clock` is appropriate for duration measurement because it is monotonic.
Do not use `std::chrono::system_clock` for frame, stage, or operation duration
measurement.

OpenCV `cv::TickMeter`, `cv::getTickCount`, and `cv::getTickFrequency` may be
used for local experiments or OpenCV-specific comparisons. They must not replace
the canonical runtime profiling structures required by this policy.

### Runtime instrumentation

Use a project-owned lightweight instrumentation layer for canonical runtime
profiling. The implementation may use RAII helpers such as `ScopedTimer`,
`ScopedStageTimer`, or `ScopedOperationTimer`, but the exact names must follow
the approved implementation task.

The instrumentation layer must write structured numeric records into bounded
runtime structures. It must not emit one log line per profiling event.

### Benchmarking

Google Benchmark is the preferred tool for controlled microbenchmarking and
stage-level performance experiments.

Use benchmarking for:

- comparing stage variants;
- comparing `L0` / `L1` / `L2` complexity implementations;
- comparing pixel routes, such as `CV_8U`, `CV_16U`, and `CV_32F`;
- comparing tile sizes and ROI sizes;
- comparing OpenCV primitive choices;
- regression checks for runtime, throughput, and memory behavior.

Benchmarking is governed by `TESTING_POLICY.md`. Agents must not create new
automated performance tests, fixtures, golden files, or test infrastructure
without explicit approval.

### External profilers

External profilers are useful for deep analysis, but they do not replace the
canonical runtime profiling model.

| Tool | Intended use | Policy status |
|---|---|---|
| Linux `perf` | CPU hotspots, sampling, call graph, PMU counters, cache/branch analysis. | Offline tool. |
| `heaptrack` | Heap allocations, allocation hotspots, stack traces for allocations. | Offline tool. |
| Valgrind Cachegrind / Callgrind | Instruction counts, call graph, cache and branch behavior. | Heavy offline tool, not real-time. |
| Tracy | Optional timeline/frame profiler, CPU/GPU zones, locks, allocations. | Optional detailed tool. |
| Perfetto | Optional C++ trace events and timeline analysis. | Optional detailed tool. |
| Intel VTune | Intel CPU microarchitecture, hotspots, memory access, PMU events. | Optional deep profiler. |
| AMD uProf | AMD CPU profiling, hardware counters, IBS, hotspots. | Optional deep profiler. |
| NVIDIA Nsight Systems | CPU/GPU/CUDA/Jetson timeline and utilization analysis. | Use only for GPU/CUDA/Jetson work. |

Do not introduce a hard dependency on an external profiler in core runtime code
without explicit approval.

Optional profiler hooks may be added only when the task explicitly requests
trace integration or when the existing codebase already provides a compatible
optional instrumentation boundary.

## Runtime Profiling Levels

Runtime profiling must be structured into six levels.

Do not use `L0`, `L1`, or `L2` for profiling levels. Those names belong to
algorithm complexity levels.

| Level | Name | Purpose |
|---|---|---|
| `P0` | Run-level profiling | Captures the processing run, active configuration, build/runtime metadata, and selected real-time profile. |
| `P1` | Frame-level profiling | Captures full frame latency and deadline compliance. |
| `P2` | Stage-level profiling | Captures execution time of each canonical DP1 stage. |
| `P3` | Operation-level profiling | Captures selected expensive operations inside a stage. |
| `P4` | Data-volume / cardinality profiling | Captures data counters that explain runtime cost. |
| `P5` | Aggregated runtime statistics | Captures summary statistics across frames and stage calls. |

### P0 - Run-level profiling

Run-level profiling records the execution context of a pipeline run.

It must capture at least:

- `run_id` or `pipeline_run_id`;
- build identifier when available;
- active DSL/configuration snapshot or stable reference;
- schema version;
- selected real-time profile, such as `RT-5` or `RT-20`;
- input source or camera/source identity;
- frame width and height;
- input pixel format and bit depth;
- active stage list;
- stage `variant`, `level`, and parameter references;
- profiling mode and enabled profiling levels.

A runtime measurement without `P0` context is not reproducible.

### P1 - Frame-level profiling

Frame-level profiling records the total latency of one processed frame or one
explicit local processing route.

It must capture at least:

- `frame_id`;
- `camera_id` or source relation when available;
- processing start timestamp;
- processing end timestamp;
- total frame duration;
- selected deadline or budget;
- whether the deadline was missed;
- dropped/skipped frame indicator when applicable;
- stage count executed for the frame.

Frame-level profiling answers this question:

```text
Did this frame fit the selected real-time budget?
```

### P2 - Stage-level profiling

Stage-level profiling records one timing record per executed canonical DP1 stage
per frame.

Stage-level profiling answers this question:

```text
Which stage consumed the frame budget?
```

### P3 - Operation-level profiling

Operation-level profiling records selected expensive operations inside a stage.

It is required for operations that:

- convert data format;
- normalize or rescale pixel values;
- copy full-frame or large tile buffers;
- allocate or reallocate significant buffers;
- perform full-image or full-tile OpenCV passes;
- perform expensive object extraction or geometry analysis;
- are known or suspected bottlenecks.

Common operation kinds include:

- `convertTo`;
- `normalize`;
- `copyTo`;
- `clone`;
- `resize`;
- tile split;
- tile merge;
- ROI extraction when it causes a copy;
- buffer allocation or reallocation.

Operation-level profiling answers this question:

```text
What inside the stage caused the cost?
```

### P4 - Data-volume / cardinality profiling

Data-volume profiling records counters that explain runtime cost.

It must capture relevant counters such as:

- tile count;
- ROI width and height;
- tile width and height;
- number of candidates;
- number of segments;
- number of contours;
- number of connected components;
- number of validated objects;
- number of measurements;
- number of full-frame passes when known;
- kernel size or template size when it materially affects cost.

Timing without cardinality metrics is insufficient for DP1 performance analysis.

Data-volume profiling answers this question:

```text
Why did the frame or stage take this amount of time?
```

### P5 - Aggregated runtime statistics

Aggregated runtime statistics summarize multiple frames or stage calls.

For frame-level and stage-level data, aggregation must support:

- call count;
- `min`;
- `avg`;
- `median` when practical;
- `p95`;
- `p99`;
- `max`;
- deadline miss count;
- budget exceed count;
- average and maximum cardinality counters when relevant.

Average time is not sufficient for real-time analysis. `p95`, `p99`, and `max`
are required for meaningful frame-latency and stage-latency summaries.

Aggregated profiling answers this question:

```text
How does this configuration behave across a run, not just on one frame?
```

## Required Runtime Structures

The exact C++ form must be defined by an approved implementation task. However,
profiling-related code must be compatible with the structures below or with
project-approved equivalents.

### `ProfilingConfig`

Purpose: controls profiling mode, enabled levels, aggregation window, and report
emission.

Recommended fields:

```cpp
struct ProfilingConfig {
    bool enabled = true;
    bool lightweight_mode = true;
    bool operation_timing_enabled = false;
    bool format_conversion_timing_enabled = true;
    bool memory_copy_timing_enabled = true;
    bool emit_frame_reports = false;
    bool emit_run_summary = true;
    std::uint32_t aggregation_window_frames = 300;
};
```

Rules:

- `enabled = false` must not silently remove required production profiling if
  the selected runtime profile requires lightweight profiling.
- Detailed operation timing may be configurable.
- Format-conversion and memory-copy accounting must remain available.

### `RunProfile`

Purpose: captures `P0` run-level profiling metadata.

Recommended fields:

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
};
```

Rules:

- Store a stable configuration reference or snapshot reference.
- Do not store image data in `RunProfile`.
- Avoid runtime string creation in the hot path. Initialize run metadata before
  frame processing starts.

### `FrameTiming`

Purpose: captures `P1` frame-level timing.

Recommended fields:

```cpp
struct FrameTiming {
    std::uint64_t frame_id = 0;
    TimestampRef start_time{};
    TimestampRef end_time{};
    std::int64_t duration_ns = 0;
    std::int64_t deadline_ns = 0;
    bool deadline_missed = false;
    bool dropped_or_skipped = false;
};
```

Rules:

- Store durations as integer nanoseconds or microseconds.
- Use `ClockType::SteadyRuntime` for runtime-local duration timestamps.
- Do not serialize raw C++ `time_point` values as canonical payload.

### `StageTiming`

Purpose: captures `P2` stage-level timing.

Recommended fields:

```cpp
enum class StageId {
    Prep,
    RadiometricCorrection,
    Enhancement,
    MatchedFiltering,
    CandidateExtraction,
    SegmentationRefinement,
    ObjectFiltering,
    Measurement,
    Visualization,
    Persistence
};

struct StageTiming {
    StageId stage_id{};
    StageStatus status{};
    StageVariantId variant{};
    ComplexityLevel level{};
    PixelFormat input_format{};
    PixelFormat output_format{};
    TimestampRef start_time{};
    TimestampRef end_time{};
    std::int64_t duration_ns = 0;
};
```

Rules:

- Each executed canonical stage must produce exactly one stage-level timing
  record per frame.
- `variant` and `level` must come from the active configuration.
- Do not mix `variant` and `level`.
- Do not use `StageTiming` as a hidden output channel for algorithm results.

### `OperationTiming`

Purpose: captures `P3` operation-level timing.

Recommended fields:

```cpp
enum class ProfiledOperationKind {
    ConvertTo,
    Normalize,
    CopyTo,
    Clone,
    Resize,
   ...
};

struct OperationTiming {
    StageId parent_stage{};
    ProfiledOperationKind operation_kind{};
    PixelFormat input_format{};
    PixelFormat output_format{};
    std::uint64_t bytes_processed = 0;
    std::int64_t duration_ns = 0;
};
```

Rules:

- Operation timings must be children of a stage timing record.
- Operation timings must use stable enum identifiers when possible.
- Do not allocate strings for operation names in hot loops.
- If an operation is not measured separately, it must be covered by a clearly
  named parent stage scope.

### `CardinalityMetrics`

Purpose: captures `P4` counters.

Recommended fields:

```cpp
struct CardinalityMetrics {
    std::uint32_t tile_count = 0;
    std::uint32_t candidate_count = 0;
    std::uint32_t segment_count = 0;
    std::uint32_t contour_count = 0;
    std::uint32_t connected_component_count = 0;
    std::uint32_t validated_object_count = 0;
    std::uint32_t measurement_count = 0;
};
```

Rules:

- Counters must be frame-scoped unless explicitly defined otherwise.
- Counters must be updated at the stage that creates or finalizes the counted
  entities.
- Counters must not require traversal of large containers only for diagnostics
  unless the traversal is already part of the algorithm.

### `MemoryMetrics`

Purpose: captures lightweight memory-related counters when available.

Recommended fields:

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

Rules:

- Lightweight memory metrics are not a replacement for `heaptrack` or other
  allocation profilers.
- Count explicit full-frame and large-tile copies.
- Treat allocation spikes as performance defects unless explicitly justified by
  the stage specification.

### `StageProfile`

Purpose: captures `P5` aggregated stage statistics.

Recommended fields:

```cpp
struct StageProfile {
    StageId stage_id{};
    StageVariantId variant{};
    ComplexityLevel level{};
    std::uint64_t calls = 0;
    std::int64_t min_ns = 0;
    std::int64_t avg_ns = 0;
    std::int64_t median_ns = 0;
    std::int64_t p95_ns = 0;
    std::int64_t p99_ns = 0;
    std::int64_t max_ns = 0;
    std::uint64_t budget_exceed_count = 0;
};
```

Rules:

- Aggregation must preserve enough samples or summary state to compute required
  percentiles accurately enough for the selected report mode.
- If approximate percentile aggregation is used, the report must say so.

### `FrameProcessingReport`

Purpose: summarizes frame or run execution data.

Recommended contents:

- `run_id`;
- `frame_id` or frame range;
- active configuration reference;
- `FrameTiming`;
- stage timing list or stage summary;
- cardinality metrics;
- memory metrics when available;
- deadline status;
- warnings about missing or disabled profiling scopes.

Rules:

- `FrameProcessingReport` must not contain image payloads.
- Reports must be serializable to JSON-compatible structures when requested.
- Per-frame reports must be disabled by default in real-time production runs
  unless explicitly configured.

### RAII timing helpers

A scoped timer is the preferred implementation pattern for stage and operation
timing.

Example shape:

```cpp
class ScopedStageTimer {
public:
    ScopedStageTimer(ProfilingTrace& trace,
                     StageId stage,
                     StageVariantId variant,
                     ComplexityLevel level);
    ~ScopedStageTimer();

    ScopedStageTimer(const ScopedStageTimer&) = delete;
    ScopedStageTimer& operator=(const ScopedStageTimer&) = delete;
};
```

Rules:

- Use RAII timers to close timing records on normal exits and error exits.
- Destructors must not throw.
- Destructors must not perform blocking I/O.
- Destructors must not format long strings.
- Timer construction and destruction must have bounded overhead.

## Time Source Policy

Use `std::chrono::steady_clock` for duration measurement.

Allowed:

```cpp
auto start = std::chrono::steady_clock::now();
auto end = std::chrono::steady_clock::now();
auto duration_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
```

Forbidden for duration profiling:

```cpp
auto start = std::chrono::system_clock::now();
```

Rules:

- `system_clock` may be used for human-readable wall-clock timestamps in run
  metadata, not for latency measurement.
- Persisted profiling timestamps must carry explicit clock semantics.
- Runtime-local profiling timestamps should map to `ClockType::SteadyRuntime`.
- Store durations as signed or unsigned integer nanoseconds or microseconds.
- Do not mix milliseconds, microseconds, and nanoseconds without explicit field
  names or unit metadata.

## Stage Instrumentation Rules

Every canonical DP1 stage implementation must have a stage-level profiling
scope.

For a stage function shaped as:

```text
process(input, context, config) -> output
```

instrumentation must capture:

- stage identifier;
- selected `variant`;
- selected algorithm complexity `level`;
- input format;
- output format;
- start time;
- end time;
- duration;
- status.

Rules:

- Create one `StageTiming` record per executed stage per frame.
- If a stage is disabled by configuration, do not create a fake execution timing
  record. If needed, record disabled status separately.
- If a stage exits with an error, close the timing record and record the error
  status.
- Do not hide multiple canonical stages inside one timing record.
- Do not split one canonical stage into unrelated timings unless the parent
  stage timing remains present.
- Do not use profiling to pass algorithm outputs between stages.

Good:

```cpp
ScopedStageTimer stage_timer(
    context.profiling_trace,
    StageId::SegmentationRefinement,
    config.variant,
    config.level);

auto segments = runSegmentation(mask, config.parameters);
context.cardinality.segment_count = static_cast<std::uint32_t>(segments.size());
```

Bad:

```cpp
auto t0 = std::chrono::steady_clock::now();
// segmentation code
LOG_DEBUG(logger, "segmentation took ...");
```

The bad example mixes ad-hoc timing with logging and does not create a
structured profiling record.

## Operation Instrumentation Rules

Operation-level profiling is required for selected expensive or hidden-cost
operations when detailed profiling is enabled. Some operation accounting must
remain available even in lightweight mode, especially for format conversion and
large copies.

Instrument or explicitly include in a named parent scope:

- data-format conversions;
- normalization/rescaling;
- full-frame or large-tile copies;
- deep copies through `cv::Mat::clone()`;
- `copyTo()` when it copies image data;
- allocation or reallocation of large temporary buffers;
- full-image or full-tile OpenCV operations;
- tile split and tile merge;
- candidate/segment/object extraction operations.

Rules:

- `convertTo`, `normalize`, `copyTo`, and `clone` must not be hidden costs.
- If a conversion or copy is necessary, measure it or count it.
- If operation-level timing is disabled, the parent stage must still include the
  operation cost.
- Do not create operation timers in per-pixel inner loops.
- Do not record an operation event for every contour point or every pixel.
- Group repeated small operations under a bounded parent scope when necessary.

Good:

```cpp
{
    ScopedOperationTimer op_timer(
        context.profiling_trace,
        StageId::RadiometricCorrection,
        ProfiledOperationKind::ConvertTo,
        src.type(),
        CV_32FC1,
        estimateBytes(src));

    src.convertTo(dst, CV_32F);
}
```

Bad:

```cpp
cv::Mat tmp = input.clone();
// no timing, no counter, no explanation
```

## Data-Volume Metrics Rules

Cardinality metrics are part of profiling. They are not optional diagnostics
when they explain stage cost.

Rules:

- `Prep` must expose tile/ROI metrics when it creates tiles or ROI routes.
- `Candidate extraction` must expose candidate count when it creates candidate
  hypotheses.
- `Segmentation refinement` must expose segment, contour, or component counts
  when it creates these entities.
- `Object filtering` must expose input object count and validated object count
  when practical.
- `Measurement` must expose measurement count.
- Metrics must be frame-scoped unless a different scope is explicitly required.
- Do not recompute expensive counts only for profiling if the algorithm already
  has the count available.

Recommended stage-to-metric mapping:

| Stage | Required or recommended counters |
|---|---|
| `Prep` | `tile_count`, ROI size, tile size, overlap size when relevant. |
| `Radiometric correction` | full-frame/tile pass count, history size for stateful variants when relevant. |
| `Enhancement` | kernel size, pass count when relevant. |
| `Matched filtering` | kernel/template size, response-map dimensions when relevant. |
| `Candidate extraction` | candidate count, threshold mode when relevant. |
| `Segmentation refinement` | segment count, contour count, connected component count. |
| `Object filtering` | input object count, rejected count, validated object count. |
| `Measurement` | measurement count, photometry domain when relevant. |

## Aggregation Rules

Runtime profiling must support aggregation over a frame window or full run.

Required frame-level aggregates:

- frame count;
- `min` duration;
- `avg` duration;
- `median` duration when practical;
- `p95` duration;
- `p99` duration;
- `max` duration;
- deadline miss count;
- dropped/skipped frame count when applicable.

Required stage-level aggregates:

- calls;
- `min` duration;
- `avg` duration;
- `median` duration when practical;
- `p95` duration;
- `p99` duration;
- `max` duration;
- budget exceed count when a stage budget exists.

Rules:

- Average time alone is insufficient.
- Percentiles must be available for real-time analysis.
- Aggregation windows must be bounded in memory.
- Per-frame raw traces must be bounded or explicitly configured.
- If approximation is used for percentiles, the report must indicate it.

## Real-Time Overhead Rules

Profiling must have bounded overhead.

In hot paths, do not introduce:

- blocking I/O;
- per-event log emission;
- uncontrolled dynamic allocation;
- unbounded `std::string` construction;
- unbounded `std::map` / `std::unordered_map` updates;
- mutex contention inside per-pixel, per-contour-point, or per-object inner loops;
- image serialization;
- full container traversal only for profiling;
- exception-heavy control flow for normal timing events.

Preferred implementation patterns:

- preallocated vectors for per-frame stage events;
- fixed-size arrays when the number of stages is known;
- enum identifiers instead of dynamically created names;
- ring buffers for bounded recent-frame traces;
- aggregation windows with bounded memory;
- optional detailed operation tracing controlled by configuration;
- no per-frame text formatting unless explicitly configured.

`FrameContext` must remain a small metadata object. It must not own primary
image buffers, masks, candidates, segments, validated objects, or measurements
as hidden payloads.

## Logging Boundary

Profiling records are not log messages.

Rules:

- Do not write one log line per frame-stage timing event in normal runtime.
- Do not write one log line per operation timing event in normal runtime.
- Do not use log4cxx as the profiling data store.
- Do not serialize images, matrices, masks, candidates, or contours into logs as
  part of profiling.
- Logging may emit aggregated profiling summaries, budget violations, and
  explicitly requested diagnostics.
- Logging frequency must remain bounded and configurable.

Allowed logging examples:

- run-level profiling summary at shutdown;
- warning when `RT-5` or `RT-20` budget is repeatedly missed;
- aggregated stage profile every configured window;
- explicit diagnostic report in a manual validation run.

Forbidden logging example:

```cpp
LOG_DEBUG(logger, "frame=" << frame_id << " stage=" << name << " time=" << duration);
```

This is forbidden as a default per-frame profiling mechanism.

## Configuration Boundary

Profiling configuration is a runtime/application concern.

If the current task requires profiling configuration, use the canonical DP1
configuration model and do not invent hidden switches in algorithmic code.

A possible configuration shape is:

```json
{
  "profiling": {
    "enabled": true,
    "mode": "lightweight",
    "levels": ["P0", "P1", "P2", "P4", "P5"],
    "operation_timing": {
      "enabled": false,
      "include_format_conversions": true,
      "include_memory_copies": true
    },
    "aggregation_window_frames": 300,
    "emit_frame_reports": false,
    "emit_run_summary": true
  }
}
```

Rules:

- Do not modify the canonical DSL schema without explicit approval.
- Do not add unversioned configuration fields.
- Do not add hidden environment-variable switches unless the task explicitly
  requests them.
- Do not disable mandatory lightweight profiling in production unless the
  product-level task explicitly allows it.
- If exact DSL fields are not yet defined, propose the configuration extension
  instead of silently implementing it.

## Benchmarking Boundary

Runtime profiling and benchmarking are different activities.

Runtime profiling answers:

```text
How did this configured pipeline run behave on actual frames?
```

Benchmarking answers:

```text
How does this function, stage, variant, or operation behave under controlled repeated inputs?
```

Use runtime profiling for:

- frame budget compliance;
- per-stage bottleneck detection;
- per-run summaries;
- production or near-production telemetry;
- explaining behavior under actual configuration.

Use benchmarks for:

- isolated variant comparison;
- OpenCV primitive comparison;
- tile-size comparison;
- pixel-format comparison;
- controlled performance regression checks;
- validating an optimization outside the live pipeline.

Agents must follow `TESTING_POLICY.md`. Without explicit approval, do not create
new automated performance tests, fixtures, mocks, datasets, golden files, or
benchmark infrastructure.

If benchmarking is needed, report:

- what should be benchmarked;
- why runtime profiling is insufficient;
- expected inputs;
- metrics to collect;
- acceptance threshold;
- proposed location under the test/validation structure.

## External Profiler Boundary

External profilers may explain bottlenecks, but they do not replace
`FrameTiming`, `StageTiming`, `StageProfile`, or `FrameProcessingReport`.

Rules:

- Do not make core DP1 runtime code depend on `perf`, `heaptrack`, Valgrind,
  Tracy, Perfetto, VTune, AMD uProf, or Nsight without explicit approval.
- Do not add vendor-specific profiler APIs to canonical stage code unless the
  task specifically requires optional integration.
- If optional trace hooks are added, they must compile out or remain disabled
  when not configured.
- External profiler findings must be mapped back to DP1 stages, operations, and
  configuration variants when reported.

Recommended use:

| Situation | Tool |
|---|---|
| Unknown CPU hotspot | Linux `perf`. |
| Suspected allocation spike | `heaptrack`. |
| Cache/branch behavior investigation | Valgrind Cachegrind / Callgrind. |
| Multithreaded timeline, queues, locks, frame spikes | Tracy or Perfetto. |
| Intel CPU microarchitecture analysis | Intel VTune. |
| AMD CPU microarchitecture analysis | AMD uProf. |
| CUDA/GPU/Jetson timeline | NVIDIA Nsight Systems. |

## Memory Profiling Rules

Memory behavior is part of real-time profiling because allocations and copies can
create latency spikes.

Rules:

- Treat `cv::Mat::clone()` as an expensive operation.
- Treat `copyTo()` as an expensive operation when it copies image data.
- Treat `convertTo()` as both a format conversion and a full data pass.
- Treat `normalize()` as a full data pass unless proven otherwise.
- Do not create full-frame temporary buffers in tile routes unless the stage
  specification explicitly allows it.
- Prefer reusable tile-local buffers owned by `TileContext` for tile routes.
- Do not store profiling-owned image payloads in `FrameContext`.
- Count or time large copies and conversions.
- Use `heaptrack` or another approved offline memory profiler when allocation
  behavior cannot be explained through lightweight counters.

A new allocation in the frame hot path must be justified by one of:

- required algorithm state defined by a stage specification;
- one-time initialization outside the hot path;
- bounded reusable buffer setup;
- explicitly approved implementation task.

## OpenCV-Specific Profiling Rules

OpenCV calls may hide full-frame passes, allocations, format conversions, and
copies. AI-generated code must make these costs visible.

Potentially expensive OpenCV operations include:

- `cv::Mat::clone`;
- `cv::Mat::copyTo`;
- `cv::Mat::convertTo`;
- `cv::normalize`;
- `cv::resize`;
- `cv::GaussianBlur`;
- `cv::medianBlur`;
- `cv::bilateralFilter`;
- `cv::filter2D`;
- `cv::matchTemplate`;
- `cv::threshold`;
- `cv::adaptiveThreshold`;
- `cv::morphologyEx`;
- `cv::findContours`;
- `cv::connectedComponents`;
- `cv::moments`;
- `cv::boundingRect` over large contour sets;
- `cv::minAreaRect` over large contour sets.

Rules:

- Any new OpenCV full-frame or per-tile operation must be inside a stage timing
  scope.
- Format-changing operations must be timed or counted explicitly.
- Full-frame copies must be timed or counted explicitly.
- Repeated OpenCV operations inside loops must be grouped or sampled in a way
  that does not create excessive profiling overhead.
- Do not assume that an OpenCV call is cheap because it is a single function
  call.
- Do not use OpenCV timing utilities as an inconsistent parallel profiling
  system unless the task is explicitly experimental.

## Reporting Format

Profiling reports must be structured and machine-readable when persisted.

Preferred report formats:

- JSON for structured run/frame summaries;
- CSV for tabular performance analysis;
- log summary only for human-readable aggregated diagnostics.

A run summary should include:

```json
{
  "schema_version": "1.0",
  "run_id": "...",
  "rt_profile": "RT-20",
  "frame_count": 300,
  "frame_time_ns": {
    "min": 0,
    "avg": 0,
    "p95": 0,
    "p99": 0,
    "max": 0
  },
  "deadline_miss_count": 0,
  "stages": [
    {
      "stage_id": "SegmentationRefinement",
      "variant": "open_close_contours",
      "level": "L1",
      "calls": 300,
      "avg_ns": 0,
      "p95_ns": 0,
      "p99_ns": 0,
      "max_ns": 0
    }
  ],
  "cardinality": {
    "tile_count_avg": 0,
    "candidate_count_avg": 0,
    "segment_count_avg": 0,
    "validated_object_count_avg": 0
  }
}
```

Rules:

- Reports must include units in field names or schema metadata.
- Reports must include active configuration reference or snapshot reference.
- Reports must not contain raw image data.
- Reports must not contain unbounded per-pixel or per-contour details.
- Reports must distinguish detection stages from infrastructure stages.

## AI-Agent Decision Procedure

When adding or changing runtime code, an AI coding agent must follow this
procedure.

1. Determine whether the code is in a cold path, warm path, or hot path.
2. Determine whether the code belongs to a canonical DP1 stage.
3. If it is a stage implementation, preserve or add one `StageTiming` record for
   the stage execution.
4. If the code performs format conversion, normalization, full-frame/full-tile
   copy, large allocation, or expensive OpenCV processing, add operation timing
   or ensure the cost is included in a named parent scope.
5. If the code creates tiles, candidates, segments, contours, objects, or
   measurements, update the corresponding cardinality metric.
6. Use `std::chrono::steady_clock` or the approved project wrapper for
   durations.
7. Do not add per-frame logging for profiling data.
8. Do not modify the DSL or canonical data cards without explicit approval.
9. Do not add new external profiler dependencies without explicit approval.
10. If optimization is requested, capture or request a baseline before claiming
    improvement.
11. In the final report, state which profiling points, counters, or aggregation
    outputs were added or changed.

## AI-Agent Prohibitions

AI coding agents must not:

- use `std::chrono::system_clock` for duration profiling;
- introduce ad-hoc timers with inconsistent units;
- use `L0`, `L1`, or `L2` as profiling levels;
- create a stage implementation without stage-level timing when profiling is in
  scope;
- hide `convertTo`, `normalize`, `copyTo`, or `clone` costs;
- write profiling data as one log line per frame or per operation;
- store image payloads, masks, candidates, segments, validated objects, or
  measurements in `FrameContext` as profiling data;
- allocate strings, maps, or unbounded containers in pixel loops or hot object
  loops only for profiling;
- add external profiler APIs to core code without approval;
- change canonical DSL fields without approval;
- create performance tests or benchmark infrastructure without approval;
- optimize a stage and claim improvement without before/after measurements;
- treat average runtime as sufficient evidence for real-time behavior;
- ignore `p95`, `p99`, and `max` latency when evaluating real-time behavior.

## Review Checklist

Use this checklist when reviewing profiling-related changes.

- Does each executed canonical stage produce one `StageTiming` record?
- Is total frame processing time measured?
- Is the active configuration or configuration reference available in run
  profiling?
- Are `variant` and `level` recorded separately?
- Are units explicit, such as `_ns` or `_us`?
- Is `steady_clock` or an approved monotonic wrapper used for durations?
- Are format conversions measured or counted?
- Are large copies measured or counted?
- Are tile, candidate, segment, contour, object, and measurement counts recorded
  where relevant?
- Are per-frame raw traces bounded?
- Are aggregation windows bounded?
- Are `p95`, `p99`, and `max` available in summaries?
- Is logging separated from profiling?
- Is profiling overhead bounded in hot paths?
- Does the change avoid hidden image payloads inside `FrameContext`?
- Does the change avoid new profiler dependencies unless approved?
- Does the final report state what profiling evidence was added or changed?

## Examples

### Good: stage-level timing

```cpp
Output MeasurementStage::process(const Input& input,
                                 FrameContext& context,
                                 const MeasurementStageConfig& config) {
    ScopedStageTimer timer(
        context.profiling_trace,
        StageId::Measurement,
        config.variant,
        config.level);

    auto output = computeMeasurements(input, config);
    context.cardinality.measurement_count =
        static_cast<std::uint32_t>(output.records.size());

    return output;
}
```

### Good: operation timing for format conversion

```cpp
{
    ScopedOperationTimer timer(
        context.profiling_trace,
        StageId::Enhancement,
        ProfiledOperationKind::ConvertTo,
        input.type(),
        CV_32FC1,
        estimateBytes(input));

    input.convertTo(processing, CV_32F);
}
```

### Good: bounded summary logging

```cpp
if (profile_summary.deadline_miss_count > 0) {
    LOG_WARN(logger, "DP1 profiling summary: deadline misses detected in aggregation window");
}
```

### Bad: wall-clock duration measurement

```cpp
auto start = std::chrono::system_clock::now();
// processing
```

Use `steady_clock` or an approved monotonic wrapper instead.

### Bad: per-frame profiling through logging

```cpp
LOG_DEBUG(logger, "stage Measurement took " << duration_ns << " ns");
```

Use structured profiling records and aggregated summaries instead.

### Bad: hidden copy cost

```cpp
cv::Mat tmp = frame.clone();
runAlgorithm(tmp);
```

If the copy is necessary, it must be justified and measured or counted.

## External References

These references explain the technologies and practices used by this policy.
They are background material and do not override Project Knowledge.

- C++ `std::chrono::steady_clock` reference:
  https://en.cppreference.com/w/cpp/chrono/steady_clock
- OpenCV timing utilities and `TickMeter`:
  https://docs.opencv.org/4.x/d9/d6f/classcv_1_1TickMeter.html
- OpenCV performance measurement tutorial:
  https://docs.opencv.org/4.x/dc/d71/tutorial_py_optimization.html
- Google Benchmark user guide:
  https://google.github.io/benchmark/user_guide.html
- Linux `perf` documentation:
  https://perfwiki.github.io/main/
- Red Hat guide to recording and analyzing performance profiles with `perf`:
  https://docs.redhat.com/en/documentation/red_hat_enterprise_linux/10/html/monitoring_and_managing_system_status_and_performance/recording-and-analyzing-performance-profiles-with-perf
- heaptrack allocation profiler:
  https://github.com/KDE/heaptrack
- Valgrind manual:
  https://valgrind.org/docs/manual/manual.html
- Tracy Profiler:
  https://github.com/wolfpld/tracy
- Perfetto tracing SDK:
  https://perfetto.dev/docs/instrumentation/tracing-sdk
- Intel VTune Profiler documentation:
  https://www.intel.com/content/www/us/en/docs/vtune-profiler/user-guide/current/overview.html
- AMD uProf documentation:
  https://www.amd.com/en/developer/uprof.html
- NVIDIA Nsight Systems documentation:
  https://developer.nvidia.com/nsight-systems
