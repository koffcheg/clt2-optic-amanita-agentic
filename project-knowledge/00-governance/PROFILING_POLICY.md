# PROFILING_POLICY.md

## Purpose

This file defines runtime profiling rules for AI-assisted code changes in the
real-time C++20 computer vision project.

Profiling is a runtime measurement and aggregation mechanism. It exposes frame,
stage, operation, data-volume, memory-copy, and aggregate timing behavior
without changing the algorithmic behavior of the frame processing pipeline.

Core rule:

```text
Profiling must measure real-time behavior without becoming part of the algorithm.
```

Profiling is not logging. Logging emits diagnostic messages. Profiling records
structured numeric measurements that can be aggregated, reported, compared, and
used to find bottlenecks.

Profiling is not benchmarking. Runtime profiling observes actual configured
pipeline execution. Benchmarking runs controlled experiments for functions,
stages, variants, frame sizes, pixel formats, or complexity levels.

Profiling is not optimization. Profiling provides evidence for optimization. An
AI coding agent must not claim a performance improvement without before/after
measurement evidence.

## Status

Status: active.

This document is an active governance document for profiling-related code and
Project Knowledge changes.

## Scope

This policy applies to:

- C++20 production code;
- OpenCV-based image-processing code;
- canonical DP1 runtime code;
- DP1 stage implementations;
- `FrameContext`, `TileContext`, `TileResult`, and runtime-domain structures;
- `ProfilingTrace`, `StageTiming`, profiling summaries, and frame reports;
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
- `project-knowledge/02-dp1/canonical/data_domains/dp1.domain.profiling.md`;
- `project-knowledge/02-dp1/canonical/data_domains/dp1.domain.time.md`;
- `project-knowledge/02-dp1/canonical/data_domains/dp1.domain.runtime.md`;
- `project-knowledge/02-dp1/canonical/data_domains/dp1.domain.memory_ownership.md`;
- `project-knowledge/02-dp1/canonical/data_domains/dp1.domain.conversion_rules.md`;
- `project-knowledge/02-dp1/canonical/data_domains/dp1.domain.opencv_invariants.md`;
- `project-knowledge/02-dp1/canonical/data_domains/structures/runtime/dp1.domain.runtime.frame_context.md`;
- `project-knowledge/02-dp1/canonical/pipeline/dp1.pipeline.stage_contract.md`;
- `project-knowledge/02-dp1/canonical/configuration/dp1.config.application.profiling.md`;
- `project-knowledge/02-dp1/canonical/configuration/dp1.config.pipeline_configuration_c.md`;
- `project-knowledge/02-dp1/canonical/configuration/dp1.config.stage_variant_registry.md`;
- relevant stage cards and stage specifications.

This policy uses `P0`-`P5` for profiling levels. These are not algorithm
complexity levels. Algorithm complexity levels remain `L0`, `L1`, `L2`, `L3`,
and `Lx` as defined by DP1 configuration knowledge.

## Definitions

| Term | Meaning |
|---|---|
| Runtime profiling | Instrumentation inside the running pipeline that records timing, counters, and aggregation data. |
| Frame-level profiling | Measurement of full processing latency for one frame or one explicit local processing route. |
| Stage-level profiling | Measurement of one canonical DP1 stage execution. |
| Operation-level profiling | Measurement of selected expensive operations inside a stage, such as `convertTo`, `copyTo`, `GaussianBlur`, or `findContours`. |
| Cardinality metrics | Data-volume counters that explain runtime cost, such as tile count, candidate count, contour count, or object count. |
| Aggregated profile | Summary statistics across many frames or stage calls: `min`, `avg`, `median`, `p95`, `p99`, `max`, and budget misses. |
| Hot path | Per-frame, per-tile, per-object, per-contour, or per-pixel code where extra overhead can affect real-time behavior. |
| Lightweight profiling | Always-available low-overhead profiling suitable for production or near-production runs. |
| Detailed profiling | Configurable profiling with operation-level timing, memory-copy accounting, and optional trace integration. |

## Profiling Technology

Use `std::chrono::steady_clock` as the default time source for runtime duration
measurement. `steady_clock` is monotonic and suitable for interval measurement.
Do not use `std::chrono::system_clock` for frame, stage, or operation duration
measurement.

OpenCV `cv::TickMeter`, `cv::getTickCount`, and `cv::getTickFrequency` may be
used for local experiments or OpenCV-specific comparisons. They must not replace
the canonical runtime profiling structures required by
`dp1.domain.profiling`.

Use a project-owned lightweight instrumentation layer for canonical runtime
profiling. The implementation may use RAII helpers such as `ScopedTimer`,
`ScopedStageTimer`, or `ScopedOperationTimer`, but exact names belong to the
approved implementation task.

The instrumentation layer must write structured numeric records into bounded
runtime structures. It must not emit one log line per profiling event.

## Runtime Profiling Levels

Runtime profiling is structured into six levels.

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

The canonical structure definitions for these levels are in
`dp1.domain.profiling`.

## Stage Identity

Profiling records must use stable stage identifiers.

Core canonical DP1 stages use the stage names from
`dp1.config.stage_variant_registry`:

```text
prep
radiometric_correction
enhancement
matched_filtering
candidate_extraction
segmentation_refinement
object_filtering
measurement
```

Profiling identity must remain extensible. Do not hard-code a closed permanent
`StageId` set in canonical knowledge. Future stages or infrastructure scopes may
be added by approved Project Knowledge changes. Implementation code may cache
validated stage identifiers as enums or indexes for performance, but persisted
or canonical profiling records must remain mappable to stable stage keys.

Infrastructure scopes such as visualization, persistence, report writing, or
transport may be profiled as non-core scopes only when the task explicitly
defines them. They must not be presented as one of the eight canonical DP1
semantic stages unless the stage registry is updated.

## Stage Instrumentation Rules

Every executed canonical DP1 stage implementation must have one stage-level
profiling scope.

For a stage function shaped as:

```text
process(input, context, config) -> output
```

instrumentation must capture:

- stable stage key;
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
  record. Record disabled status separately when needed.
- If a stage exits with an error, close the timing record and record error
  status.
- Do not hide multiple canonical stages inside one timing record.
- Do not split one canonical stage into unrelated timings unless the parent
  stage timing remains present.
- Do not use profiling to pass algorithm outputs between stages.

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

## Data-Volume Metrics Rules

Cardinality metrics are part of profiling. They are not optional diagnostics
when they explain stage cost.

Rules:

- `prep` must expose tile/ROI metrics when it creates tiles or ROI routes.
- `candidate_extraction` must expose candidate count when it creates candidate
  hypotheses.
- `segmentation_refinement` must expose segment, contour, or component counts
  when it creates these entities.
- `object_filtering` must expose input object count and validated object count
  when practical.
- `measurement` must expose measurement count.
- Metrics must be frame-scoped unless a different scope is explicitly required.
- Do not recompute expensive counts only for profiling if the algorithm already
  has the count available.

Recommended stage-to-metric mapping:

| Stage | Required or recommended counters |
|---|---|
| `prep` | `tile_count`, ROI count/size, tile size, overlap size when relevant. |
| `radiometric_correction` | Full-frame/tile pass count, conversion/copy counts, history size for stateful variants when relevant. |
| `enhancement` | Kernel size, pass count when relevant. |
| `matched_filtering` | Kernel/template size, response-map dimensions when relevant. |
| `candidate_extraction` | Candidate count, threshold mode when relevant. |
| `segmentation_refinement` | Segment count, contour count, connected component count. |
| `object_filtering` | Input object count, rejected count, validated object count. |
| `measurement` | Measurement count, photometry domain when relevant. |

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

Average time alone is insufficient. Percentiles and maximum latency are required
for real-time analysis. Aggregation windows and raw traces must be bounded in
memory. If approximate percentile aggregation is used, the report must indicate
it.

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
- fixed-size arrays or validated indexes when the number of active stages is known;
- stable keys instead of dynamically created names;
- ring buffers for bounded recent-frame traces;
- aggregation windows with bounded memory;
- optional detailed operation tracing controlled by
  `dp1.config.application.profiling`;
- no per-frame text formatting unless explicitly configured.

`FrameContext` must remain a small metadata object. It must not own primary
image buffers, masks, candidates, segments, validated objects, or measurements
as hidden profiling payloads.

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
- Logging frequency must remain bounded and configurable through
  `dp1.config.application.logging` and
  `dp1.config.application.profiling.logging_bridge`.

Allowed logging examples:

- run-level profiling summary at shutdown;
- warning when `RT-5` or `RT-20` budget is repeatedly missed;
- aggregated stage profile every configured window;
- explicit diagnostic report in a manual validation run.

Forbidden default mechanism:

```cpp
LOG_DEBUG(logger, "frame=" << frame_id << " stage=" << name << " time=" << duration);
```

Use structured profiling records and aggregated summaries instead.

## Configuration Boundary

Profiling configuration is an application-runtime concern.

Canonical DP1 profiling configuration is described by:
`project-knowledge/02-dp1/canonical/configuration/dp1.config.application.profiling.md`.

Runtime switches, raw trace retention, report emission, logging bridge, and
external trace backend selection belong to `dp1.config.application.profiling`.

`PipelineConfig C` may provide stage/variant/level context and profiling
requirements tied to a processing profile, but algorithmic stage parameters must
not contain hidden profiling switches.

Rules:

- Do not modify canonical configuration fields without explicit approval.
- Do not add unversioned configuration fields.
- Do not add hidden environment-variable switches unless the task explicitly
  requests them.
- Do not disable mandatory lightweight profiling in production unless the
  product-level task explicitly allows it.
- If exact configuration fields are not yet defined, propose the configuration
  extension instead of silently implementing it.

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

Use runtime profiling for frame budget compliance, per-stage bottleneck
detection, per-run summaries, production or near-production telemetry, and
explaining behavior under actual configuration.

Use benchmarks for isolated variant comparison, OpenCV primitive comparison,
tile-size comparison, pixel-format comparison, controlled performance
regression checks, and validating an optimization outside the live pipeline.

Agents must follow `TESTING_POLICY.md`. Without explicit approval, do not create
new automated performance tests, fixtures, mocks, datasets, golden files, or
benchmark infrastructure.

## External Profiler Boundary

External profilers may explain bottlenecks, but they do not replace canonical
runtime profiling records or summaries.

External profilers must be used only when the current user instruction or
approved task explicitly requests them.

Rules:

- Do not make core DP1 runtime code depend on `perf`, `heaptrack`, Valgrind,
  Tracy, Perfetto, VTune, AMD uProf, or Nsight without explicit approval.
- Do not add vendor-specific profiler APIs to canonical stage code unless the
  task specifically requires optional integration.
- If optional trace hooks are added, they must compile out or remain disabled
  when not configured.
- External profiler findings must be mapped back to DP1 stages, operations, and
  configuration variants when reported.

Recommended use by explicit instruction:

| Situation | Tool |
|---|---|
| Unknown CPU hotspot | Linux `perf`. |
| Suspected allocation spike | `heaptrack`. |
| Cache/branch behavior investigation | Valgrind Cachegrind / Callgrind. |
| Multithreaded timeline, queues, locks, frame spikes | Tracy or Perfetto. |
| Intel CPU microarchitecture analysis | Intel VTune. |
| AMD CPU microarchitecture analysis | AMD uProf. |
| CUDA/GPU/Jetson timeline | NVIDIA Nsight Systems. |

## Reporting Format

Profiling reports must be structured and machine-readable when persisted.

Preferred report formats:

- JSON for structured run/frame summaries;
- CSV for tabular performance analysis;
- log summary only for human-readable aggregated diagnostics.

Rules:

- Reports must include units in field names or schema metadata.
- Reports must include active configuration reference or snapshot reference.
- Reports must not contain raw image data.
- Reports must not contain unbounded per-pixel or per-contour details.
- Reports must distinguish core DP1 detection/measurement stages from
  infrastructure scopes.

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
8. Do not modify canonical data cards or configuration fields without approval.
9. Do not add new external profiler dependencies without approval.
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
- change canonical configuration fields without approval;
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
