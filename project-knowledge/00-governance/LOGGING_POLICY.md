# LOGGING_POLICY.md

## Purpose

This file defines logging rules for AI-assisted code changes in the real-time
C++ computer vision project.

Logging is a diagnostic emission mechanism. It must help developers understand
runtime behavior, failures, configuration choices, timing problems, validation
runs, and inter-module communication. Logging must not become a hidden data
path, a replacement for stage contracts, or a source of non-deterministic
latency.

Core rule:

```text
Logging must not change real-time behavior of the frame processing pipeline.
```

## Status

Status: active.

This document is an active governance document for logging-related code changes.

## Scope

This policy applies to:

- C++ production code;
- OpenCV-based image-processing code;
- DP1 and DP2 runtime code;
- CameraPro integration and simulation code;
- DP1 -> DP2 protocol boundary diagnostics;
- validation-run diagnostics;
- logging statements generated or modified by an AI coding agent;
- logging-related configuration changes when explicitly requested by the current
  task.

This policy does not define the full observability architecture of the project.
It does not introduce a new logging framework. It does not replace
`CODE_STYLE.md`, `TESTING_POLICY.md`, stage contracts, data-domain cards, or
protocol cards.

## Related Project Knowledge

Read this policy together with:

- `AGENTS.md`;
- `project-knowledge/00-governance/CODE_STYLE.md`;
- `project-knowledge/00-governance/TESTING_POLICY.md`;
- `project-knowledge/00-governance/PROFILING_POLICY.md`;
- `project-knowledge/01-project/PROJECT_ECOSYSTEM.md`;
- `project-knowledge/02-dp1/canonical/DP1_CANONICAL_INDEX.md`;
- `project-knowledge/02-dp1/canonical/configuration/dp1.config.application.logging.md`;
- `project-knowledge/03-dp2/canonical/DP2_CANONICAL_INDEX.md`;
- relevant DP1/DP2 stage, data-domain, runtime, and protocol cards.

This policy refines the existing `CODE_STYLE.md` rules that forbid blocking I/O
in hot per-frame paths and require heavy logging in such paths to be bounded,
configurable, and outside the hottest loop where possible.

If this policy conflicts with explicit user instructions in the current task,
follow the `AGENTS.md` source-of-truth order and report the conflict.

## Logging Technology

The project uses Apache log4cxx 1.3.1.

Do not introduce another logging framework without explicit approval. Do not
replace log4cxx with:

- `std::cout`;
- `std::cerr`;
- `printf` / `fprintf`;
- ad-hoc file writers;
- another C++ logging library;
- a custom logging abstraction that bypasses existing project conventions.

Use the existing log4cxx integration style found in the local module. If the
module already has a logger declaration pattern, preserve it. If no logger
exists, add one only when the task requires diagnostics and the naming follows
this policy.

Do not configure global loggers, appenders, layouts, or thresholds from
algorithmic code.

## Configuration Boundary

Logging configuration is an application-runtime concern.

Canonical DP1 logging configuration is described by:
`project-knowledge/02-dp1/canonical/configuration/dp1.config.application.logging.md`.

This policy defines logging behavior rules. The application configuration card
defines the typed configuration surface.

Algorithmic code must not create or mutate global loggers, appenders, layouts,
or level thresholds. Such settings belong to application startup/bootstrap code
that reads `ApplicationConfig`.

Profiling configuration is a separate application-runtime concern described by
`dp1.config.application.profiling` and `PROFILING_POLICY.md`. Logging may emit
bounded aggregated profiling summaries and budget warnings, but raw profiling
events are not log events and log4cxx is not the profiling data store.

## Real-Time Logging Principle

Before adding a log statement, classify the code location.

| Path class | Meaning | Logging rule |
|---|---|---|
| Cold path | Startup, shutdown, configuration loading, dependency initialization, one-time resource setup. | Logging is allowed when useful and bounded. |
| Warm path | Per-frame summary, per-batch summary, per-stage boundary, per-protocol exchange summary. | Logging is allowed with stable event names, bounded fields, and controlled frequency. |
| Hot path | Per pixel, per matrix element, per tile inner loop, per contour point, per object inner loop, critical per-frame processing. | Logging is forbidden by default. Use only guarded, bounded, explicitly justified diagnostics. |

Hot-path logging must not introduce:

- blocking I/O;
- large string formatting;
- image or matrix serialization;
- full container traversal only for diagnostics;
- dynamic allocation only for diagnostics;
- uncontrolled per-frame `DEBUG` or `TRACE` spam;
- measurable jitter in frame-processing latency.

If a diagnostic requires expensive computation, compute it only when the target
log level is enabled or move it outside the hot path.

## Logger Naming Convention

Logger names must follow the project structure and remain stable.

Preferred namespace pattern:

```text
amanita.<subsystem>[.<module>][.<stage>]
```

Recommended logger names:

```text
amanita.camerapro
amanita.dp1
amanita.dp1.capture
amanita.dp1.prep
amanita.dp1.radiometric_correction
amanita.dp1.enhancement
amanita.dp1.matched_filtering
amanita.dp1.candidate_extraction
amanita.dp1.segmentation_refinement
amanita.dp1.object_filtering
amanita.dp1.measurement
amanita.dp1.output
amanita.dp2
amanita.protocol.dp1_dp2
amanita.validation
amanita.common
```

Do not create random logger names based on file names, temporary task names, or
informal descriptions. Do not write everything to the root logger. Do not attach
appenders directly to many low-level loggers unless the task explicitly concerns
logging configuration.

## Log Levels

Use the standard log4cxx severity model consistently.

| Level | Use in this project | Do not use for |
|---|---|---|
| `TRACE` | Short-term deep diagnostics during local investigation. Disabled in normal real-time runs. | Production per-frame spam, pixel/tile inner loops, permanent high-volume tracing. |
| `DEBUG` | Development diagnostics, selected algorithm variant, bounded stage summaries, non-critical internal state. | Large data dumps, raw images, normal lifecycle events that should be `INFO`. |
| `INFO` | Startup, shutdown, configuration selected, pipeline start/stop, source connection lifecycle, validation-run summary. | High-frequency per-frame logs unless explicitly sampled or rate-limited. |
| `WARN` | Recoverable abnormal conditions: skipped frame, fallback, queue pressure, temporary source loss, deadline miss, recoverable configuration issue. | Expected normal branches or successful fallback without operational relevance. |
| `ERROR` | Operation failed, frame failed, source I/O failed, protocol exchange failed, invalid runtime input prevents processing. | Normal control flow, expected validation rejection, recoverable conditions already represented by status. |
| `FATAL` | Module/application cannot continue safely. | Any error from which the current module can recover. |

Rules:

```text
Do not use ERROR or FATAL for normal control flow.
Do not use INFO for high-frequency per-frame noise.
Do not use TRACE/DEBUG in hot loops unless explicitly guarded and bounded.
Do not log the same failure at multiple layers unless each layer adds distinct context.
```

## Structured Message Format

Log messages must be structured enough to be parsed and searched. Prefer stable
key-value fields in the message body.

Preferred style:

```text
event=stage_finished stage=radiometric_correction variant=inverse_median frame_id=123 camera_id=cam0 duration_us=820 status=ok
```

Avoid vague prose-only messages:

```text
Stage done.
Something went wrong.
Processing failed.
```

Recommended common fields:

| Field | Meaning |
|---|---|
| `event` | Stable event name. Required for new structured messages. |
| `module` | Logical module when logger name is not enough. |
| `stage` | DP1/DP2 stage name. |
| `variant` | Selected stage algorithm/configuration variant. |
| `pipeline_run_id` | Run/session identity when available. |
| `camera_id` | Active camera/source identity when available. |
| `source_id` | Source identity when different from camera identity. |
| `frame_id` | Frame identity in the current source/run. |
| `tile_id` | Tile identity for tile-route summaries only. |
| `duration_us` | Duration in microseconds for short stage measurements. |
| `duration_ms` | Duration in milliseconds for longer operations. |
| `status` | `ok`, `skipped`, `failed`, `fallback`, or a project-defined status. |
| `error_code` | Stable project-defined error code when available. |
| `config_key` | Configuration key involved in a configuration event. |

Not every message needs every field. Include only fields that are relevant and
cheap to obtain.

Do not invent new field names if an existing field name from this policy or a
data-domain card fits.

## Stable Event Names

Use stable event names instead of one-off natural-language variants.

Recommended event names:

```text
pipeline_started
pipeline_stopped
config_loaded
config_rejected
config_defaulted
source_opened
source_lost
source_recovered
frame_received
frame_skipped
frame_processed
stage_started
stage_finished
stage_skipped
stage_failed
deadline_missed
queue_pressure
queue_overflow
fallback_enabled
protocol_send_started
protocol_send_finished
protocol_send_failed
protocol_receive_started
protocol_receive_finished
protocol_receive_failed
resource_exhausted
validation_started
validation_finished
validation_failed
```

Do not create near-duplicates such as:

```text
stage_begin
stage_started_now
begin_stage
started_processing_stage
```

If a new event name is necessary, keep it short, lower-case, snake_case, and
specific.

## Context Propagation

For image-processing diagnostics, useful context includes:

- stage name;
- frame id / frame index;
- camera id or source id;
- pipeline run id when available;
- image size;
- image type/depth/channels;
- ROI or tile id when relevant;
- selected configuration key or stage variant;
- duration or timing status;
- concise error code/status.

The canonical identity model is:

```text
pipeline_run_id + camera_id + frame_id + local_id
```

Do not rely on a local numeric id alone if the object can cross a tile, frame,
run, or DP1 -> DP2 boundary.

Do not use long free-form string ids in hot paths. Prefer compact numeric ids
and reconstruct full diagnostic identity from `FrameContext` or equivalent
runtime context when needed.

## MDC Policy

Mapped Diagnostic Context may be used for repeated per-thread context such as:

- `pipeline_run_id`;
- `camera_id`;
- `source_id`;
- `frame_id`;
- `stage`.

Rules:

```text
Every MDC::put must have a clearly defined cleanup point.
Do not use MDC in reusable library code unless thread ownership and lifetime are explicit.
Do not store large values in MDC.
Do not store unbounded external strings in MDC.
Do not use MDC as a substitute for explicit data contracts.
Do not let MDC from one frame/source leak into another frame/source.
```

Prefer scoped cleanup wrappers if the project already provides them. If no
scoped MDC helper exists, do not introduce one without approval unless the
current task explicitly covers logging infrastructure.

## AsyncAppender and Appender Policy

Real-time processing threads must not depend on synchronous file or network
logging.

If `AsyncAppender` is used, its queue/buffer size and blocking behavior must be
configured deliberately. Blocking behavior can affect the calling thread when
the async buffer is full. In a real-time frame-processing path, this can create
latency spikes.

Rules:

```text
Do not change AsyncAppender blocking behavior from algorithmic code.
Do not change appender buffer size from algorithmic code.
Do not create appenders inside frame-processing code.
Do not attach appenders in stage implementation code.
Do not perform network logging directly from processing stages.
```

If logging throughput becomes a problem, report it as an
infrastructure/configuration issue. Do not compensate by adding ad-hoc local
files or suppressing important errors silently.

## Where To Log

Logging is recommended at stable boundaries and failure points.

Allowed and useful locations:

- application or module startup;
- application or module shutdown;
- configuration file loading;
- configuration validation failure;
- selected DP1 pipeline route;
- selected stage variant;
- camera/source connection state changes;
- stage start/finish only when the frequency is controlled;
- stage failure with concise context;
- frame skipped/dropped with reason;
- real-time deadline miss;
- queue pressure or queue overflow;
- resource exhaustion;
- DP1 -> DP2 protocol send/receive failures;
- validation run start/finish/failure;
- unexpected empty/invalid `cv::Mat` at a stage boundary;
- fallback activation.

Stage-boundary log examples should be summaries, not payload dumps:

```text
event=stage_finished stage=candidate_extraction frame_id=123 candidates=17 duration_us=640 status=ok
```

```text
event=frame_skipped frame_id=124 camera_id=cam0 reason=source_timeout status=skipped
```

## Where Not To Log

Do not log in:

- pixel loops;
- matrix-element loops;
- tight tile inner loops;
- contour-point loops;
- object inner loops;
- allocation-sensitive frame loops;
- code that executes only to produce debug strings in hot paths.

Do not log:

- raw `cv::Mat` content;
- full image buffers;
- full masks;
- full frame dumps;
- raw binary payloads;
- full contours;
- full vectors of candidates, segments, tracks, or measurements;
- repeated per-object messages for every object in a frame;
- repeated identical errors without rate limiting or state transition logic.

## OpenCV-Specific Logging Rules

Allowed OpenCV-related diagnostic fields:

- `rows`;
- `cols`;
- `size`;
- `type`;
- `depth`;
- `channels`;
- ROI rectangle;
- tile count;
- selected pixel format;
- selected input bit depth;
- compact mask convention such as `0/255` or `0/1`;
- candidate/segment/object counts;
- foreground pixel count only if already computed for the algorithm;
- timing summaries already collected by runtime code.

Forbidden OpenCV-related diagnostics:

- serializing `cv::Mat` to a string;
- logging matrix contents;
- cloning or copying images only for logs;
- computing histograms only for logs in hot paths;
- converting images only for logs in hot paths;
- saving debug images through the logging mechanism;
- treating visualization/debug images as canonical algorithm outputs.

If visual artifacts are needed for debugging or validation, use an explicit
validation/debug-output mechanism approved for the task. Do not hide image
persistence behind logging statements.

## Expensive Diagnostics Rule

A log statement is expensive if it requires:

- allocation;
- string formatting before the log call;
- traversal of a container;
- traversal of image/mask data;
- conversion of OpenCV data;
- serialization;
- filesystem access;
- network access;
- locking beyond existing logger behavior.

Expensive diagnostics must be guarded by a log-level check or moved outside the
hot path.

Illustrative pattern:

```cpp
if (logger->isDebugEnabled()) {
    LOG4CXX_DEBUG(logger,
        "event=stage_summary stage=candidate_extraction frame_id=" << frameId
        << " candidates=" << candidates.size()
        << " duration_us=" << durationUs);
}
```

Do not compute expensive values before checking whether the target log level is
enabled.

This example is illustrative. Follow the exact logger type and macro style
already used by the local module.

## Security and Privacy Rules

Do not log secrets or sensitive values.

Forbidden values:

- passwords;
- tokens;
- private keys;
- API keys;
- credentials embedded in camera/source URI;
- full authenticated URI;
- personal data;
- private filesystem paths when a shorter diagnostic identifier is enough;
- raw external input without sanitization;
- binary payloads that may contain unknown data.

External or untrusted strings must be sanitized before logging. At minimum,
prevent log injection through carriage return, line feed, and delimiter
characters.

Do not log a complete configuration object if it may contain credentials or
environment-specific secrets. Log only the selected non-sensitive keys and
values needed for diagnosis.

## Error Handling and Logging

Do not ignore errors silently.

Do not catch broad exceptions only to log and continue with corrupted state.

Do not suppress an exception by replacing it with a log statement unless the
module convention explicitly allows recovery and the resulting state is valid.

When logging an error, include enough context to reproduce or locate the
problem:

```text
event=stage_failed stage=radiometric_correction variant=inverse_median frame_id=123 camera_id=cam0 error_code=invalid_input_mat rows=0 cols=0 status=failed
```

For external I/O and protocol errors, include:

- operation;
- endpoint or sanitized source id;
- status/error code;
- retry/fallback decision;
- bounded message.

Do not include full payloads or credentials.

## Duplicate Logging Rule

Do not log the same event at every layer.

Preferred rule:

```text
Log at the layer that has the most useful context and responsibility.
```

If a lower layer logs a failure, an upper layer should log only if it adds
meaningful context, such as stage name, frame id, source id, selected variant,
retry decision, or final outcome.

Avoid patterns such as:

```text
low-level function logs ERROR
stage logs the same ERROR
pipeline logs the same ERROR
application logs the same ERROR
```

Instead, use one detailed error log plus one higher-level summary only when
operationally useful.

## Sampling and Rate Limiting

High-frequency diagnostics must be sampled or rate-limited.

Examples of acceptable summaries:

```text
event=frame_stats_sampled every_n_frames=100 processed=100 dropped=1 avg_duration_us=780 max_duration_us=1350
```

```text
event=queue_pressure queue=dp1_output depth=128 capacity=256 dropped_since_last_report=3 status=warn
```

Do not add a per-frame `INFO` log for normal successful processing unless the
current task explicitly requires it and the frequency is controlled.

## AI-Agent Decision Procedure

Before adding or modifying logging, the agent must perform this decision
procedure:

1. Identify the code path: cold, warm, or hot.
2. Identify the event type: lifecycle, configuration, stage boundary, error,
   performance, protocol, validation, or security-relevant event.
3. Check whether a similar log already exists.
4. Select the log level according to this policy.
5. Select a stable `event` name.
6. Include only relevant, bounded context fields.
7. Check that all fields are cheap to obtain at that location.
8. Check that no raw image/matrix/container dump is included.
9. Check that no secret, credential, full authenticated URI, or unsanitized
   external string is included.
10. Check that the log will not add blocking I/O or unpredictable latency.
11. Add the smallest useful log statement.
12. Do not change global logging configuration unless the task explicitly
    requires it.
13. If logging requires a new helper, appender, layout, rate limiter, or MDC
    wrapper, report the need and request approval unless the current task
    explicitly includes logging infrastructure changes.

The agent must not add logging mechanically. A new log statement is allowed only
when it improves diagnosis of a concrete behavior, failure, boundary, or
performance risk.

## AI-Agent Prohibitions

The agent must not:

- introduce another logging framework;
- use `std::cout`, `std::cerr`, or `printf` for production diagnostics;
- add log statements in pixel loops or tight tile/object loops;
- log raw `cv::Mat`, masks, frames, binary payloads, or large containers;
- compute expensive diagnostic strings unless the target level is enabled;
- add synchronous file/network logging in processing threads;
- change global logging configuration from algorithmic code;
- log secrets, credentials, full camera URIs, tokens, or private keys;
- log unsanitized external strings;
- add duplicate logs across layers for the same event;
- use `ERROR` or `FATAL` for expected control flow;
- hide state corruption by logging and continuing;
- create tests, fixtures, mocks, or validation artifacts for logging without
  following `TESTING_POLICY.md` approval rules.

## Review Checklist

Use this checklist when reviewing logging changes:

```text
[ ] Is this log useful for diagnosing a real issue or runtime state?
[ ] Is the code location cold, warm, or hot?
[ ] Is the level correct?
[ ] Is the event name stable?
[ ] Is the message structured and bounded?
[ ] Does the log include stage/frame/source context where relevant?
[ ] Are expensive diagnostics guarded by log-level checks?
[ ] Does the log avoid raw image/matrix/container data?
[ ] Does the log avoid secrets and credentials?
[ ] Are external strings sanitized or bounded?
[ ] Does the log avoid duplicate reporting across layers?
[ ] Does it preserve real-time behavior?
[ ] Does it respect existing module logging style?
```

## Examples

### Good: configuration selected

```cpp
LOG4CXX_INFO(logger,
    "event=config_loaded module=dp1 config_key=pipeline.route route=" << routeName
    << " status=ok");
```

### Good: bounded stage summary

```cpp
LOG4CXX_DEBUG(logger,
    "event=stage_finished stage=candidate_extraction frame_id=" << frameId
    << " candidates=" << candidateCount
    << " duration_us=" << durationUs
    << " status=ok");
```

### Good: recoverable frame skip

```cpp
LOG4CXX_WARN(logger,
    "event=frame_skipped frame_id=" << frameId
    << " camera_id=" << cameraId
    << " reason=source_timeout status=skipped");
```

### Good: expensive debug summary guarded

```cpp
if (logger->isDebugEnabled()) {
    const auto summary = buildSmallBoundedSummary(result);
    LOG4CXX_DEBUG(logger,
        "event=stage_summary stage=object_filtering frame_id=" << frameId
        << " " << summary);
}
```

### Bad: raw matrix dump

```cpp
LOG4CXX_DEBUG(logger, "mat=" << mat);
```

Reason: this may dump large image data and distort real-time behavior.

### Bad: pixel-loop logging

```cpp
for (int y = 0; y < image.rows; ++y) {
    for (int x = 0; x < image.cols; ++x) {
        LOG4CXX_TRACE(logger, "pixel=" << static_cast<int>(image.at<uint8_t>(y, x)));
    }
}
```

Reason: this destroys performance and creates unusable logs.

### Bad: secret leakage

```cpp
LOG4CXX_INFO(logger, "camera_uri=" << cameraUri);
```

Reason: camera URI may contain credentials. Log a sanitized source id instead.

### Bad: using error level for normal fallback

```cpp
LOG4CXX_ERROR(logger, "fallback selected");
```

Reason: fallback may be a recoverable operational state. Use `WARN` only if the
fallback indicates an abnormal situation that requires attention.

## External References

These references explain the rationale behind this policy. They are not a
replacement for project-specific rules.

- Apache Log4cxx 1.3.1 documentation: https://logging.apache.org/log4cxx/1.3.1/
- Apache Log4cxx Loggers, Appenders and Layouts: https://logging.apache.org/log4cxx/1.0.0/usage.html
- Apache Log4cxx AsyncAppender: https://logging.apache.org/log4cxx/1.3.0/classlog4cxx_1_1AsyncAppender.html
- Apache Log4cxx MDC / filters documentation: https://logging.apache.org/log4cxx/1.3.1/filters.html
- OpenTelemetry Logs Data Model: https://opentelemetry.io/docs/specs/otel/logs/data-model/
- OWASP Logging Cheat Sheet: https://cheatsheetseries.owasp.org/cheatsheets/Logging_Cheat_Sheet.html
