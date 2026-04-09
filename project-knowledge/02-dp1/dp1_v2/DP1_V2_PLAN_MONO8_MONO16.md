# DP1-v2 / datapro1_v2 Refactoring Plan

## 1. Task framing and hard constraints

This document defines a **new DP1 implementation (DP1_v2, executable/module naming candidate: datapro1_v2)** that runs in parallel to legacy DP1 and supports:
- Input formats: mono8..mono16
- Compute backends: CPU baseline, GPU-capable design (NVIDIA/AMD/Intel)
- Concurrency: multithreading + optional multiprocessing
- Reuse: maximal reuse of current OpenCV stack and stable transport contracts
- Runtime objective: realtime with adaptive load control under variable load
- Mandatory preprocessing feature: inter-frame per-pixel temporal median filter (from `materials/`) behind feature-toggle
- Target operating point for current iteration:
  - stable realtime on FullHD at 30 FPS
  - allowed non-stable operation up to 130 FPS (best-effort ceiling, not SLA)

User constraints embedded in this plan:
- Do not rewrite legacy DP1 in place.
- Build DP1_v2 as a deep refactoring of proven legacy ideas and code schemes, while removing weak architecture and performance bottlenecks.
- Keep DP1->DP2 integration stable at first migration stages.
- Add automatic parameter adaptation to maintain realtime behavior.
- Keep binning and temporal median as independently switchable feature-toggles.
- In the first iteration, deliver functional parity with legacy DP1 for external behavior (inputs, outputs, artifacts, integration points) without reusing legacy code as runtime core.
- Use existing project solutions and existing libraries first; do not add new external libraries in this task.
- Enforce clean-code constraints: no magic numbers/strings in hot logic, configuration-driven knobs, extension-ready module boundaries.

## 1.1 Naming convention

- Working product name: `DP1_v2`.
- Runtime artifact/module name candidate: `datapro1_v2`.
- During implementation and documentation, both labels are acceptable, but each document must explicitly state which name is used in that context.

## 1.2 AI-agent operational contract

- Agent scope is strict execution of this DP1_v2 plan, not generic "improve codebase" activity.
- If implementation convenience conflicts with plan decisions, plan decisions always win.
- If a decision is not explicitly defined in this plan, the agent must choose the narrowest safe and reversible option.
- Any narrow fallback decision must be documented immediately (see Section 16 decision-note rule).
- Agent must not silently perform broad architectural steps outside active phase/subtask scope.

## 1.3 Scope boundaries and change limits

- Legacy DP1 may be read freely for analysis and extraction references.
- Legacy DP1 must not be rewritten in place.
- Small verified helper-level extraction/adaptation from legacy is allowed when it does not carry legacy runtime architecture into v2.
- Copying legacy pipeline into a new folder and calling it v2 is forbidden.
- On early phases, do not change DP1->DP2 payload schema, message id, serializer layout, or transport semantics.
- Avoid mass rename/move, broad cleanup-refactor, and edits outside active phase scope.
- Do not enable heavy debug/display/save/blocking diagnostics by default in runtime path.

## 1.4 Allowed reuse vs forbidden reuse

Allowed reuse:
- Reuse proven algorithmic ideas from legacy DP1.
- Reuse small helper functions or narrow fragments with mandatory documentation.
- Reuse stable transport and serialization boundaries.
- Reuse existing configs, build logic, and integration contracts.

Forbidden reuse:
- Using legacy DP1 as runtime core of DP1_v2.
- Porting legacy bottlenecks into v2 (blocking behavior, busy-wait loops, excessive copying, debug-heavy runtime defaults).
- Introducing hidden compatibility drift under "refactoring" wording.
- Replacing deterministic behavior with more abstract but less predictable code in hot/realtime path.

## 2. Current-state anchors (code schemes + exact lines)

### CS-01. Legacy background subtraction bootstrap (tile-level)
- File: `datapro1/src/datapro1.cpp`
- Lines:
  - 60: `createBackgroundSubtractorKNN(...)`
  - 64: `createBackgroundSubtractorMOG2(...)`
- Meaning: subtractors are instantiated per fragment/tile in legacy flow.

### CS-02. Legacy depth collapse 16->8 before key filters
- File: `datapro1/src/datapro1.cpp`
- Line:
  - 111: `var.vec_frag[index].convertTo(..., CV_8UC1, 1.0/256, 0)`
- Meaning: precision is reduced early; mono16 dynamic range is not preserved through core path.

### CS-03. Display path conversion to 8-bit visualization
- File: `datapro1/src/datapro1.cpp`
- Line:
  - 349: `out_frame.convertTo(..., CV_8UC3, 1.0/256, 0)`
- Meaning: this is acceptable as view/output conversion, but should not leak into compute path.

### CS-04. Input source normalization in URI runner (8-bit source -> 16-bit container)
- File: `datapro1/src/dp1_uri_runner.cpp`
- Lines:
  - 53: `cvtColor(..., COLOR_BGR2GRAY)`
  - 54: `convertTo(..., CV_16UC1, 256, 0)`
- Meaning: ingestion already supports 16-bit containerization but downstream path still truncates.

### CS-05. Frame pre-stage with optional sum-binning
- File: `datapro1/src/dp1_frame_proc.cpp`
- Lines:
  - 32: `to_gray_if_needed(...)`
  - 40: `apply_sum_binning(...)`
  - 57: `convertTo(..., CV_32SC1)` for accumulation
  - 337-338: binning invocation before `datapro1(...)`
- Meaning: there is a natural insertion point for DP1-v2 preprocess stage and adaptive binning.

### CS-06. Legacy helper conversions for calibration/median branches
- File: `datapro1/src/dp1_frame_proc.cpp`
- Lines:
  - 322 and 351: `convertTo(..., CV_8UC1, 1.0/256, 0)`
- Meaning: these are candidate split points between compute-depth and auxiliary-depth branches.

### CS-07. DP1->DP2 transport boundary
- File: `datapro1/src/dp1_frame_proc.cpp`
- Line:
  - 379: `send_res_to_dp2(data_res)`
- Meaning: DP1-v2 must preserve this boundary contract in first rollout.

### CS-08. DP1 message envelope and payload serialization
- File: `datapro1/src/dp1_tr_res2dp2.cpp`
- Lines:
  - 86: `send_res_to_dp2(...)`
  - 96: `ms.write_native(dp1_to_dp2_rpc_msg_new_measure)`
  - 97: `serialize_dp1_res(ms, data)`
- Meaning: message ID and payload layout are hard compatibility points.

### CS-09. Binary layout guards in serializer/deserializer
- File: `datapro1/src/dp1_rpc_data_mrsh.cpp`
- Lines:
  - 89: `serialize_dp1_res(...)`
  - 100: writes `sizeof(TOptionsMeasurement)`
  - 108: `deserialize_dp1_res(...)`
  - 121, 128, 134: structure size mismatch checks
- Meaning: binary compatibility is explicit and must remain intact during phased migration.

### CS-10. DP2 receive path consuming DP1 payload
- File: `datapro2/src/dp2_rpc_cl.cpp`
- Lines:
  - 67: `on_new_dp1_meas(...)`
  - 70: `deserialize_dp1_res(ms, dp1_data)`
  - 73: push into `all_cam_mes[cam_index]`
- Meaning: DP2 already trusts current payload schema; schema changes must be versioned or deferred.

## 3. Verified OpenCV capability baseline (current environment)

Local build facts observed earlier in this workspace environment:
- OpenCL support is enabled.
- CUDA support is not enabled in the currently used OpenCV build.

Implication for roadmap:
- Phase-1 acceleration target is CPU + optional UMat/OpenCL path.
- CUDA backend should be architecturally pluggable, but treated as optional deployment profile unless OpenCV is rebuilt with CUDA modules.

## 4. OpenCV mono16 compatibility matrix (internet-verified)

### Sources consulted
- OpenCV filtering/depth combinations and operators: `group__imgproc__filter.html`
- OpenCV miscellaneous transforms / thresholding: `group__imgproc__misc.html`
- OpenCV color conversions: `group__imgproc__color__conversions.html`
- OpenCV shape/contour APIs: `group__imgproc__shape.html`
- Background subtractor API and constraints: `classcv_1_1BackgroundSubtractor.html`, `classcv_1_1BackgroundSubtractorMOG2.html`, `classcv_1_1BackgroundSubtractorKNN.html`

### Compatibility summary for DP1-v2 decisions

1. Filtering and morphology (median/gaussian/filter2D/erode/dilate/morphologyEx)
- Status for mono16: generally supported for CV_16U in imgproc filtering family.
- Caveat: `medianBlur` has kernel-size-dependent restrictions; CV_16U is documented for small kernels (3/5), larger apertures have stricter type limits.
- Design action: keep compute path in 16U where possible, constrain median kernel presets by type/backend profile.

2. Thresholding
- Status for mono16: threshold API supports 8U and 16U for key modes; OTSU documented for CV_8UC1 and CV_16UC1.
- Design action: use 16U-native thresholding when feasible; do not force global pre-collapse to 8U.

3. Color conversions
- Status for mono16: many conversions accept 16U, but support depends on specific conversion code.
- Design action: enforce per-conversion compatibility checks in pipeline builder.

4. Contour extraction and connected components
- `findContours`: requires 8-bit single-channel binary image (or CV_32SC1 labels only in specific retrieval modes).
- `connectedComponents`: documented for 8-bit single-channel input.
- Design action: contour/CC stages must consume explicit binary masks (8U). This is not a regression; it is an expected type boundary.

5. Background subtraction (MOG2/KNN)
- API notes indicate floating-point frames are expected in [0,255] when used as float.
- Output mask is 8-bit binary.
- Design action: for mono16 sources, introduce explicit normalization policy before subtractors (and keep policy configurable and measurable).

6. Inter-frame temporal median in OpenCV
- OpenCV does not provide a built-in inter-frame temporal median operator for streaming buffers.
- Design action: temporal median must be implemented as dedicated DP1-v2 kernel module (not expected as native OpenCV call).

## 5. Target DP1-v2 architecture (new module)

## 5.1 Core principles
- Keep **native depth domain** as long as possible (mono16 preserved through preprocess/filter/segmentation-prep).
- Introduce **explicit type boundaries** where OpenCV APIs require 8U masks.
- Separate **compute representation** from **display/export representation**.
- Keep transport contract to DP2 unchanged in initial phases.

Engineering quality bar:
- Prefer simple, explicit, measurable code over generic/clever abstractions.
- When SLA is violated, deterministic behavior has priority over maximum throughput.
- Prefer reversible migration steps over aggressive one-shot optimization.
- Stable CPU-first correctness has priority over premature GPU expansion.

## 5.2 Proposed module map
- `dp1v2_ingest`
  - Inputs from camerapro/URI runner.
  - Outputs `FramePacket` with metadata: bit depth, timestamp, exposure, camera id.
- `dp1v2_temporal_median`
  - Inter-frame per-pixel temporal median for K=3 and K=5 (mandatory).
  - Produces optional `Med_t` and required signed residual frame for downstream stages.
  - Controlled by feature-toggle and independent from binning toggle.
- `dp1v2_preprocess`
  - Grayscale canonicalization, optional sum-binning, denoise, covariance filter.
  - Backend-aware implementations (CPU/OpenCL/CUDA future).
- `dp1v2_segment`
  - Background subtraction, thresholding, morphology.
  - Controlled depth transitions (16U compute -> 8U mask).
- `dp1v2_measure`
  - Blob/contour extraction, geometric stats, object candidates.
- `dp1v2_pack`
  - Map to existing `TDataRes` and call existing transport boundary (`send_res_to_dp2`).
- `dp1v2_qos`
  - Adaptive control loop driven by realtime telemetry.

## 5.4 Temporal median requirements (aligned with `materials/`)

### Functional requirements
- Temporal median is computed per pixel over time only (no spatial window).
- Modes:
  - `FixedK3` (exact, mandatory)
  - `FixedK5` (exact, mandatory)
- `stride >= 1` controls frame sampling into temporal buffer:
  - `1`: each frame
  - `2`: every second frame
  - `3`: every third frame
- Runtime defaults for high-FPS realtime:
  - `temporal_mode = Causal`
  - `output_mode = HoldLastMedian`

### Output typing
- For uint8 input: residual type must be int16.
- For uint16 input: residual type must be int32.

### Performance constraints
- No dynamic allocations inside `processFrame` hot path.
- Ring buffers and work buffers are preallocated in constructor/reset.
- Kernel for K=3 and kernel for K=5 are separate procedures selected once at initialization.

### Feature toggles
- `feature.temporal_median.enabled`
- `feature.temporal_median.mode` (`FixedK3|FixedK5`)
- `feature.temporal_median.stride`
- `feature.temporal_median.output_mode` (`SelectedOnly|HoldLastMedian`)
- `feature.binning.enabled`
- `feature.binning.factor`
- `feature.compute.backend` (`CPU|OPENCL|CUDA` with automatic fallback)

## 5.3 Data contracts in DP1-v2 internals
- `FramePacket`
  - `cv::Mat frame`
  - `enum PixelType { MONO8, MONO10, MONO12, MONO14, MONO16 }`
  - `uint64_t frame_id`
  - `TimePoint exposure_start`
  - `CameraId camera_id`
- `ProcContext`
  - mutable knobs: binning factor, median kernel, morphology iterations, bg history/learning-rate, queue limits.
- `ProcTelemetry`
  - stage latencies, queue depth, drop rate, mask fill ratio, detections/frame, CPU/GPU utilization proxy.

## 5.5 Realtime C++ hot-path engineering rules

- No dynamic allocations in hot per-frame path.
- No blocking I/O in hot path.
- No heavy logging in hot path.
- No hidden deep copies of frame buffers.
- No implicit data-depth conversion without explicit boundary function and documented reason.
- No exceptions crossing hot paths unless explicitly allowed by local project conventions.
- Avoid virtual dispatch in hottest loops unless justified and measured.
- Ring buffers, work buffers, and queue nodes should be preallocated when feasible.
- Ownership and lifetime of frame/work buffers must be explicit.
- Prefer predictable data flow and controlled memory layout over excessive abstraction.
- Each backend transition must be explicit, measurable, and easy to disable.

## 5.6 Memory ownership model requirements

Before implementing any large processing stage, explicitly define and document:
- who owns input frame buffers;
- which stages may mutate buffers in place;
- where copies are allowed and where copies are forbidden;
- which buffers are reused between frames;
- who owns queue nodes and payloads;
- where handoff is move/reference/view/copy.

Implicit ownership models are not allowed.

## 6. Compute backend strategy (CPU/GPU)

## 6.1 Backend abstraction
- Define backend-neutral operations:
  - `blur`, `median`, `filter2d`, `morphology`, `threshold`, `bg_subtract`, `contours`.
- Runtime chooses implementation by capability and policy:
  - `CPU_MAT`
  - `OPENCL_UMAT` (if available and profitable)
  - `CUDA_GpuMat` (future profile only if OpenCV CUDA build is provided)

## 6.2 Practical decision logic
- Start frame on CPU Mat.
- If OpenCL path enabled and frame size above threshold, promote selected stages to UMat.
- Keep fallback deterministic: any failed backend stage must downgrade safely to CPU.
- Do not mix backend transitions excessively (avoid churn penalties).

Additional rules for temporal median:
- `FixedK3` and `FixedK5` CPU kernels are baseline and mandatory.
- GPU path is selectable by config but must be optional and fail-safe.
- If GPU path is unavailable or violates latency SLA, switch stage to CPU automatically.
- For first production rollout, prioritize deterministic CPU implementation for K=3/K=5; enable GPU mode per camera profile.

## 6.3 Vendor neutrality
- NVIDIA: CUDA profile optional, not baseline.
- AMD/Intel: OpenCL path is first-class accelerator route.
- Pure CPU must remain authoritative and tested reference.

## 6.4 CPU/GPU processing mode selection
- Add explicit processing mode selector in DP1-v2 config:
  - `processing_mode = CPU_ONLY | GPU_PREFERRED | GPU_ONLY`
- Recommended defaults:
  - `CPU_ONLY` for bring-up and deterministic validation.
  - `GPU_PREFERRED` for high-load production if stability is proven.
- `GPU_ONLY` should be restricted to validated deployments and must fail fast at startup if capability checks fail.

## 7. Multithreading and multiprocessing model

## 7.1 Threading model (default)
- Stage pipeline with bounded queues:
  - ingest -> preprocess -> segment -> measure -> pack/send
- One worker group per stage (or merged stages under low load).
- Backpressure policy:
  - bounded queues + frame skipping policy by age/SLA.

Iteration requirement impact:
- Stable mode target: FullHD 30 FPS (frame period 33.3 ms).
- Stress mode target: best-effort up to 130 FPS (non-SLA, allowed instability).
- Temporal median stage and segmentation stage must expose separate latency metrics and queue depth counters.

## 7.2 Multiprocessing model (optional)
- Process-per-camera or process-per-camera-group for CPU isolation.
- Shared memory ring for frames + control channel for QoS and health.
- Use only where thread model cannot keep jitter within realtime budget.

Recommended production topology for high-FPS:
- Primary: process-per-camera for isolation and failure containment.
- Inside each process: multithreaded stage pipeline with bounded lock-free or low-contention queues.
- Optional core pinning and NUMA-aware memory placement for temporal median + segmentation workers.

## 7.3 Realtime priorities
- Prioritize deterministic latency over maximal throughput once SLA is violated.
- Maintain monotonic ordering by frame_id per camera.

## 8. Adaptive load control (autotuning)

## 8.1 Control goal
Maintain realtime budget:
- Let frame period be $T$.
- Target processing latency percentile: $p95(proc\_latency) \le \alpha T$ with guard margin (e.g. $\alpha=0.8$).

## 8.2 Observability signals
- `L_p95`: stage or total processing latency p95 over sliding window.
- `Q`: queue depth / queue age.
- `D`: drop ratio.
- `M`: mask density or feature count stability proxy.

## 8.3 Actuators (ordered by quality impact)
1. Background model learning rate (small adaptation).
2. Morphology iterations / kernel size.
3. Median kernel size (respecting type limits).
4. Binning factor increase.
5. Frame decimation (last-resort under overload).

Temporal median specific actuators:
6. Temporal `stride` increase under overload (while preserving `HoldLastMedian`).
7. Controlled switch `FixedK5 -> FixedK3` when deadline misses persist.

## 8.4 Controller policy
- Use finite-state control with hysteresis:
  - `GREEN`: $L_p95 < 0.7T$ and low queue.
  - `YELLOW`: $0.7T \le L_p95 < 0.9T$.
  - `RED`: $L_p95 \ge 0.9T$ or queue overflow trend.
- Transition rules require persistence for N windows to avoid oscillation.
- Cooldown timers prevent rapid knob toggling.

## 8.5 Safety constraints
- Hard bounds per knob (e.g., max binning, min morphology quality).
- If quality floor reached and SLA still violated, explicit controlled dropping is enabled and reported.

## 9. C++ vs Python for the new DP1

## 9.1 C++ recommendation (primary)
- Existing runtime, transport, memory layout, and low-level integrations are C++.
- Lower jitter and allocation control for realtime critical path.
- Easier zero-copy/near-zero-copy integration with current DP1/DP2 structures.

## 9.2 Python role (supporting)
- Suitable for:
  - offline parameter search,
  - experiment harness,
  - telemetry analysis,
  - optional prototyping of heuristics.
- Not recommended for production hard-realtime DP1 core path in current project topology.

## 10. Integration strategy with DP2 and camerapro

## 10.1 DP2 compatibility (phase-preserving)
- Keep message id and payload schema unchanged initially:
  - `dp1_to_dp2_rpc_msg_new_measure`
  - `serialize_dp1_res` / `deserialize_dp1_res`
- Preserve `TOptionsMeasurement` binary size and ordering.
- Any schema extension must be versioned and negotiated after side-by-side validation.

Compatibility guardrail for early phases:
- Any planned change that touches payload schema/message id/serializer layout/transport semantics requires separate explicit approval before implementation.

## 10.2 camerapro interface impact
- Existing IPC contract (`cp_ipc_cam2dp1_if.h`) is frame-buffer oriented and agnostic to internal DP1 stages.
- DP1-v2 should consume frames through existing interface and avoid upstream contract changes in first stages.

## 11. Migration plan (phased rollout)

### Phase A (Iteration 1). Functional parity + bottleneck-first refactor
- Build DP1_v2/datapro1_v2 executable/module path alongside legacy DP1.
- Implement mono pipeline in Iteration 1 scope (mono8..mono16 ingest and processing boundaries).
- Keep DP2 contract untouched (`dp1_to_dp2_rpc_msg_new_measure`, payload schema, serializer layout).
- Preserve legacy-compatible external behavior:
  - startup/runner expectations,
  - artifact I/O behavior and formats,
  - interoperability with other modules.
- Immediately address major bottlenecks from `DP1_V2_PERF_MEMORY_BOTTLENECKS.md` in this phase:
  - B01 (blocking send in hot path),
  - B02 (busy-wait completion loop),
  - B03 (excessive tile copying),
  - B05 (expensive per-pixel polygon loops),
  - B08/B16 (heavy debug/display/save defaults in runtime path).
- Deliverable: DP1_v2 runs with legacy-level functionality and improved performance stability on FullHD 30 FPS.

Implementation-complete criteria (agent-executable):
- `datapro1_v2` builds and starts in agent-accessible mode as parallel implementation.
- Input path is implemented and smoke-checkable in current environment.
- Results are packed through existing transport compatibility boundary in implementation.
- No intentional wire/schema drift is introduced.
- Initial bottleneck fixes are implemented or explicitly deferred with reason.
- Baseline validation checklist against legacy is prepared.

External validation required for acceptance:
- Functional parity against legacy on target scenarios.
- Real integration behavior confirmation with neighboring modules.
- Runtime behavior confirmation in target execution environment.

### Phase B. Temporal median integration
- Keep and implement temporal median scope in this phase (`FixedK3`/`FixedK5`, preallocated buffers, no hot-path allocations).
- Temporal median remains feature-toggle controlled and independent from binning toggle.
- Validate that temporal median does not break parity/integration constraints from Phase A.

Implementation-complete criteria (agent-executable):
- K3 and K5 temporal median modes exist in code.
- Ring/work buffers are preallocated by implementation.
- Temporal median stage does not introduce intentional hot-path allocations.
- Toggle behavior is documented.
- External latency/functional check list is prepared.

External validation required for acceptance:
- Real latency impact on target data streams.
- Real functional-output impact.
- Behavior under target stream conditions.

### Phase C. QoS/autotune and backend selection
- Implement telemetry + controller state machine.
- Add CPU/OpenCL/CUDA backend policy and safe fallback.
- Add processing-mode selector (`CPU_ONLY|GPU_PREFERRED|GPU_ONLY`) and per-stage fallback rules.
- Deliverable: stable p95 latency under variable load without catastrophic quality collapse.

Implementation-complete criteria (agent-executable):
- Stage telemetry hooks are implemented.
- Controller states and actuators are explicitly described and implemented.
- Fallback rules are deterministic and implemented.
- Processing mode policy is implemented and documented.
- Overload/realtime validation checklist is prepared.

External validation required for acceptance:
- Stable behavior under real load.
- Correctness of QoS decisions under target conditions.
- Latency target conformance in real environment.

### Phase D. Side-by-side validation and cutover
- Run legacy and v2 in parallel on same streams.
- Compare output consistency metrics and realtime metrics.
- Controlled cutover by camera/group with rollback switch.

Agent restriction for this phase:
- Agent must not claim full Phase D completion if full side-by-side execution is not executable in current environment.
- In that case, agent prepares code/artifacts/procedure/metrics for side-by-side and sets status to `Ready for external validation`.

### Mandatory checkpoint after each phase
- After every phase (A/B/C/D), execute comparative validation against legacy DP1 where current environment allows; otherwise prepare and document external comparative validation steps.
- Record differences in:
  - functional behavior,
  - performance metrics,
  - integration compatibility,
  - artifact compatibility.
- No transition to next phase without documented phase checkpoint.

Phase status vocabulary:
- `Implemented`
- `Build-verified`
- `Smoke-checked`
- `Ready for external validation`
- `Externally validation-pending`
- `Blocked`

`Accepted` status is allowed only when external validation is explicitly recorded as executed.

Agent wording restriction:
- Agent must not state `Phase completed`, `Acceptance passed`, or equivalent claims when only build/static/smoke checks were available.

## 12. Validation plan (without adding new test infrastructure automatically)

Operational validation split:
- Agent-executable checks: build, compile-time checks, startup checks, and limited smoke checks available in current environment.
- External validation checks: functional/integration/realtime/performance/side-by-side checks that require external setup, bench, target streams, or manual validation.

General completion rule:
- A phase may be marked implementation-complete by agent only if:
  - code builds,
  - required code path is implemented,
  - available compile-time/startup/smoke checks are executed,
  - phase documentation is updated,
  - compatibility assumptions are checked as far as environment allows,
  - known deviations are explicitly documented,
  - external validation steps for final acceptance are listed,
  - no undocumented broad-risk issue remains.
- A phase must not be marked accepted until project acceptance checks are externally executed.

### 12.1 Runtime validation scenarios
- mono8 baseline parity (legacy vs v2).
- mono16 precision-retention scenarios (low-contrast targets).
- overload scenarios for controller (burst motion/noise).
- multi-camera synchronization and queue stress.
- temporal median scenarios: `FixedK3`, `FixedK5`, `stride=1/2/3`, `HoldLastMedian` behavior.
- stable FullHD 30 FPS scenarios for CPU_ONLY and GPU_PREFERRED profiles.
- stress scenarios up to 130 FPS in best-effort mode (explicitly marked as non-SLA).

### 12.2 Metrics to track
- Latency: p50/p95/p99 end-to-end.
- Throughput: FPS in/out.
- Drops: frame drop rate and reasons.
- Detection quality proxies: count stability, track continuity, false-positive spikes.
- Stage latencies: temporal median, preprocess, segment, measure, pack/send.
- Deadline miss ratio vs 33.3 ms frame period in stable 30 FPS mode.
- Separate deadline miss ratio for stress 130 FPS exploratory mode.
- Toggle state timeline (binning/temporal median mode/stride/backend) for postmortem analysis.

### 12.3 Acceptance gates
- Gate 1: no transport/schema regressions with DP2.
- Gate 2: realtime SLA met in stable target profile (FullHD 30 FPS).
- Gate 3: stress profile up to 130 FPS is measurable and controlled (allowed non-stable behavior).
- Gate 4: mono16 path shows non-inferior detection robustness vs forced 8-bit collapse.

## 12.5 Parity comparison contract

Avoid non-verifiable wording such as "looks similar" or "works".
For each validation checkpoint, explicitly record:
- scenario executed;
- outputs/artifacts compared;
- detected differences;
- difference classification: acceptable / unknown / regression;
- realtime behavior classification: improved / neutral / regressed.

If a check is not executable in current environment, record explicitly:
- `not executable in current environment`;
- why unavailable;
- required external execution steps;
- expected result that must be externally confirmed.

## 12.4 Realtime budget for stable FullHD 30 FPS
- Frame period: 33.3 ms.
- Recommended engineering target:
  - p95(DP1 end-to-end) <= 25.0 ms
  - p99(DP1 end-to-end) <= 30.0 ms
  - queue age bounded below one frame period in steady state
- If p95 exceeds budget, QoS controller applies degradations in this order:
  - increase temporal stride
  - downgrade K5 to K3
  - increase binning factor
  - controlled frame decimation (last resort)

## 13. Main risks and mitigations

1. OpenCV op-type mismatch on some operations
- Mitigation: explicit compatibility table in pipeline builder; fail-fast at startup.

2. Backend thrashing (CPU<->OpenCL transitions)
- Mitigation: stickiness policy and minimum residency window per backend.

3. QoS oscillation
- Mitigation: hysteresis + cooldown + bounded parameter rates.

4. Hidden contract drift to DP2
- Mitigation: freeze envelope and serializer checks in initial releases.

5. Latency spikes from memory churn
- Mitigation: pre-allocation pools for stage buffers and queue nodes.

6. Temporal median becomes bottleneck at high FPS
- Mitigation: mandatory no-allocation hot path, separate kernels for K3/K5, thread-level parallelization, optional GPU path with safe fallback.

7. GPU mode instability or backend unavailability
- Mitigation: explicit processing mode selector with startup capability checks and deterministic CPU fallback.

8. Required compatibility cannot be preserved without contract change
- Mitigation: hard stop, document blocking issue, request direction before broad modifications.

9. High uncertainty on architecture decision not fixed by plan
- Mitigation: choose conservative reversible option, create decision note, and defer broad step.

## 13.1 Failure and uncertainty handling rules

- If backend is unavailable, use safe fallback according to policy.
- If required compatibility cannot be preserved, stop and document blocking issue before broad changes.
- If a change touches transport/schema in early phases, do not implement without separate approval.
- If a feature misses latency budget without quality collapse path, use degradation order from Section 12.4 (do not invent a new policy ad hoc).
- Under high uncertainty, choose conservative and reversible option.

Soft stop:
- If external validation is unavailable, continue only within safe scope and set `Ready for external validation` or `Externally validation-pending`.

Hard stop:
- Stop and request direction if:
  - DP1->DP2 compatibility cannot be preserved without contract change,
  - required behavior is ambiguous and narrow option is still high-risk,
  - change requires new external dependencies,
  - change forces large unplanned legacy rewrite,
  - work no longer fits phased order.

## 14. Concrete implementation checklist

1. Create DP1-v2 module skeleton and config section.
2. Introduce `FramePacket`, `ProcContext`, `ProcTelemetry` types.
3. Implement ingest adapter from existing frame sources.
4. Implement preprocess path with 16U-preserving operations.
5. Implement segmentation with explicit type-boundary adapters.
6. Map measurements to existing `TDataRes` and reuse send boundary.
7. Add telemetry collection per stage.
8. Implement QoS state machine and knob application.
9. Add backend policy selection (CPU/OpenCL now, CUDA profile hook).
10. Run side-by-side validation and cutover by rollout plan.
11. Add temporal median module with K3/K5 kernels and preallocated ring buffers.
12. Add feature toggles for temporal median, binning, and processing mode.
13. Implement profiling suite for stable 30 FPS SLA and separate 130 FPS stress mode.

## 14.1 Mandatory task slicing before coding

Before any coding, agent must map work to phase and subtask; mixing multiple phases in one uncontrolled patch is forbidden.

Recommended subtask skeleton:
1. skeleton/build integration
2. ingest/frame contract
3. pack/send compatibility boundary
4. preprocess baseline
5. segmentation baseline
6. telemetry hooks
7. temporal median K3
8. temporal median K5
9. QoS controller
10. backend policy
11. side-by-side validation support

For each subtask, agent must explicitly record:
- goal;
- files to modify;
- files to read only;
- expected output;
- compatibility risks;
- validation step;
- documentation update required.

Small-step rule:
- Prefer small vertical slices that can be built and smoke-checked.
- If prerequisite is missing, implement the narrowest prerequisite first and document why.

## 15. Final recommendation

- Build DP1-v2 in C++ as a parallel executable/module.
- Preserve DP2 wire contract in first iterations.
- Keep mono16 natively through most compute stages; convert only where API requires binary 8U masks.
- Use CPU + OpenCL as production baseline now; keep CUDA as optional profile pending OpenCV rebuild with CUDA modules.
- Make adaptive load control mandatory for realtime stability, with explicit quality floors and telemetry-driven decisions.
- Treat inter-frame temporal median (K3/K5) as first-class mandatory stage with feature-toggle and strict no-allocation hot path.
- Keep CPU implementation as authoritative reference; enable GPU mode as selectable profile with deterministic fallback.

## 16. Documentation and traceability rules for AMNT-0006

All implementation documentation for this task must be stored in:
- `project-knowledge/02-dp1/dp1_v2/`

Mandatory documentation constraints:
- Every completed step must be documented.
- For each change, document:
  - code scheme name,
  - what was reused from legacy,
  - what was implemented new,
  - why the decision was made,
  - expected effect (functional/performance/maintainability).
- Track contract-sensitive decisions separately (DP1->DP2 transport, serialization, artifact formats).
- Keep implementation notes concise but sufficient for reproducibility and audit.

Per-step implementation note is mandatory in `project-knowledge/02-dp1/dp1_v2/` and must include:
- phase / subtask id;
- changed files;
- code scheme name;
- what was reused from legacy;
- what was implemented new;
- why the decision was made;
- expected functional effect;
- expected performance effect;
- compatibility impact;
- validation executed;
- result / next step.

Open-question handling rule:
- If an architectural decision is not fixed by plan, do not silently expand scope.
- Create a short decision note:
  - question,
  - options considered,
  - chosen narrow option,
  - why safest,
  - future revisit point.

Iteration report format (agent):
- Iteration start:
  - current phase,
  - current subtask,
  - planned file changes,
  - risks,
  - checks executable by agent,
  - checks requiring external validation.
- Iteration end:
  - files changed,
  - what was implemented,
  - what was intentionally not changed,
  - build result,
  - smoke checks run,
  - checks not executable in current environment,
  - external validation still required,
  - docs updated,
  - next safe step.
