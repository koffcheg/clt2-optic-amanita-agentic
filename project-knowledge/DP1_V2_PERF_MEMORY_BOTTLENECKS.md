# DP1-v2 Performance and Memory Bottlenecks Audit

## Scope
This audit covers current DP1 runtime paths and integration boundaries that affect realtime goals (target up to 125 FPS at FullHD).

Analyzed areas:
- `datapro1/src/*` hot path
- IPC receive path and frame handoff
- DP1 -> DP2 transport path
- runtime config defaults in `datapro1/config/config_datapro1.json`

## Realtime Budget Context
- 125 FPS means 8.0 ms per frame total budget.
- Any single stage with unstable tail latency can break SLA, even if average is acceptable.

---

## B01. Blocking network send in frame hot path
Evidence:
- `datapro1/src/dp1_frame_proc.cpp` (call site: `send_res_to_dp2`)
- `datapro1/src/dp1_tr_res2dp2.cpp` (`boost::asio::write` synchronous write)

What it affects:
- End-to-end frame latency (tail spikes)
- Jitter under network congestion
- Throughput stability at high FPS

Pros of current design:
- Simpler control flow
- Immediate delivery semantics

Cons of current design:
- Processing thread stalls on socket I/O
- Hard to keep deterministic 8 ms budget

Solution options:
1. Async producer-consumer transport queue (frame thread enqueues, transport thread sends)
- Impact: decouples compute from network stalls
- Pros: lower jitter, better tail latency
- Cons: queue sizing, backpressure policy complexity

2. Batched transport (multiple measure payloads per send when allowed)
- Impact: lower syscall and socket overhead
- Pros: better throughput
- Cons: potential per-item latency increase, protocol handling complexity

3. Keep sync send but enforce strict drop/backpressure on overload
- Impact: bounded stalls, possible data loss under stress
- Pros: minimal architecture change
- Cons: quality loss during overload

---

## B02. Busy-wait frame completion with 1 ms sleep granularity
Evidence:
- `datapro1/src/datapro1.cpp` (loop waiting on `num_thread_ready`, `sleep_for(1ms)`)

What it affects:
- Directly consumes up to large fraction of 8 ms budget
- Adds avoidable latency floor

Pros:
- Easy to reason about

Cons:
- Coarse latency quantization
- CPU inefficiency and longer frame completion time

Solution options:
1. Replace with condition-variable or latch/barrier completion notification
- Impact: remove polling overhead
- Pros: lower latency and CPU waste
- Cons: synchronization rewrite required

2. Use lock-free completion counters with spinning only on very short windows
- Impact: lower wait cost
- Pros: can be very fast on tuned systems
- Cons: harder correctness and tuning

---

## B03. Repeated tile extraction with border copy and clone
Evidence:
- `datapro1/src/datapro1.cpp` (`splitImage` per tile)
- `datapro1/src/datarpoFragmentation.cpp` (`copyMakeBorder` + `clone`)

What it affects:
- Memory bandwidth pressure
- Allocation/copy overhead per tile per frame
- Cache locality degradation

Pros:
- Clean isolated tile semantics

Cons:
- Full data copy path for each tile
- Scaling penalty with tile count and resolution

Solution options:
1. Use ROI views where possible, pad only when required edge tiles
- Impact: reduce copies
- Pros: significant memory traffic reduction
- Cons: boundary handling complexity

2. Preallocate per-tile buffers and reuse
- Impact: lower allocator overhead and fragmentation
- Pros: predictable latency
- Cons: higher baseline memory footprint

3. Fuse tiling with downstream operations to avoid materializing full tile clones
- Impact: fewer intermediate buffers
- Pros: better cache behavior
- Cons: more invasive refactor

---

## B04. Early 16U -> 8U conversion in compute path
Evidence:
- `datapro1/src/datapro1.cpp` (`convertTo(... CV_8UC1 ...)`)

What it affects:
- Precision loss (mono16 information collapse)
- Extra full-frame/tile memory pass

Pros:
- Compatible with 8-bit-only algorithms

Cons:
- Reduced detection robustness on low-contrast data
- Unnecessary conversion overhead where 16U is supported

Solution options:
1. Keep 16U for compute stages and convert only at mandatory 8U boundaries
- Impact: better precision and fewer conversions
- Pros: improves quality and often speed
- Cons: requires type-safe pipeline contracts

2. If 8U branch needed, generate binary mask directly from 16U compare/threshold
- Impact: avoids generic convert step
- Pros: clear boundary and less overhead
- Cons: needs explicit per-stage mask policy

---

## B05. Expensive contour feature extraction via per-pixel point-in-polygon loops
Evidence:
- `datapro1/src/datarpoSegmentation.cpp` (`pointPolygonTest` inside nested loops)

What it affects:
- CPU hotspot in measure stage
- Poor scaling with object count and contour size

Pros:
- Flexible feature extraction logic

Cons:
- High per-object computational cost
- Branch-heavy loops with weak vectorization

Solution options:
1. Use contour moments/integral-image based stats instead of enumerating all inside pixels
- Impact: big CPU reduction
- Pros: lower complexity per contour
- Cons: may change exact metric definitions

2. Restrict expensive stats to shortlisted contours only
- Impact: less worst-case overhead
- Pros: practical and incremental
- Cons: feature quality trade-off

3. Parallelize contour-level measurement stage
- Impact: better multicore utilization
- Pros: throughput gain
- Cons: contention and merge overhead

---

## B06. Global queue mutex in tile dispatch
Evidence:
- `datapro1/src/datapro1.cpp` (`static std::mutex queue_mut`, pop from shared deque)

What it affects:
- Thread scalability
- Contention as thread count increases

Pros:
- Simple correctness

Cons:
- Serialization point in allegedly parallel phase

Solution options:
1. Static partition of tiles per worker (no shared pop mutex)
- Impact: removes central contention
- Pros: deterministic scheduling
- Cons: potential load imbalance

2. Work-stealing deques per worker
- Impact: good balance + low contention
- Pros: scalable
- Cons: more complex scheduler

---

## B07. Static worker-state lifetime tied to first call parameters
Evidence:
- `datapro1/src/datapro1.cpp` (`static` condition vars, flags, worker param objects initialized with `dp1_num_thread`)

What it affects:
- Multi-camera/process behavior in shared address space
- Reconfiguration safety
- Potential memory/state correctness risks

Pros:
- One-time thread creation

Cons:
- Hidden coupling across calls
- Hard to safely support dynamic configs

Solution options:
1. Move worker pool into explicit per-instance object (`fr_proc_impl` owned)
- Impact: isolation and predictability
- Pros: safer multi-camera scaling
- Cons: refactor of lifecycle

2. Keep static pool but make it camera-keyed instance map
- Impact: reduced coupling
- Pros: smaller refactor than full redesign
- Cons: still complex lifecycle

---

## B08. Optional debug/test/display I/O inside runtime path
Evidence:
- `datapro1/src/datapro1.cpp` (`imwrite` in debug paths)
- `datapro1/src/dp1_frame_proc.cpp` (`imshow`, `video.write`, extra `resize`)
- Config defaults currently enable display/video/save in `datapro1/config/config_datapro1.json`

What it affects:
- Massive latency variance
- Disk and GUI subsystem bottlenecks

Pros:
- Useful diagnostics

Cons:
- Not compatible with high-FPS SLA

Solution options:
1. Force production profile with all debug/display/save disabled by default
- Impact: immediate FPS improvement
- Pros: simplest operational fix
- Cons: less observability by default

2. Offload diagnostics to async telemetry thread and sampled snapshots
- Impact: keeps observability with bounded cost
- Pros: better runtime balance
- Cons: implementation complexity

---

## B09. IPC receive path copies full frame from shared memory
Evidence:
- `datapro1/src/dp1_ipc.cpp` (`memcpy` full frame into local buffer)

What it affects:
- Memory bandwidth and latency
- CPU overhead on every frame

Pros:
- Isolation from producer memory lifetime

Cons:
- Full-frame copy cost each frame

Solution options:
1. Zero-copy or reference-counted shared-memory frame handoff
- Impact: lower copy overhead
- Pros: throughput boost
- Cons: stricter lifetime/synchronization contracts

2. Double/triple shared-memory buffer with ownership flags
- Impact: fewer copies with controlled safety
- Pros: deterministic
- Cons: IPC protocol changes

---

## B10. URI path copies frame again after conversion
Evidence:
- `datapro1/src/dp1_uri_runner.cpp` (`frame.copyTo(just_rc_frames.mat)` after `cvtColor` and `convertTo`)

What it affects:
- Extra per-frame memory traffic

Pros:
- Simpler ownership semantics

Cons:
- Redundant copy in high-FPS context

Solution options:
1. Reuse preallocated destination buffer and decode directly where possible
- Impact: lower copy overhead
- Pros: better memory efficiency
- Cons: requires careful lifetime handling

2. Move semantics and buffer recycling in frame queue
- Impact: reduce allocations/copies
- Pros: low jitter
- Cons: queue API adjustments

---

## B11. Sum-binning implementation is scalar nested loops over CV_32S converted image
Evidence:
- `datapro1/src/dp1_frame_proc.cpp` (`apply_sum_binning`)

What it affects:
- CPU load in preprocess when binning enabled

Pros:
- Exact sum semantics

Cons:
- Scalar loop overhead
- Extra convert to CV_32SC1 and memory passes

Solution options:
1. Vectorized/parallel tile reduction for binning
- Impact: significant preprocess acceleration
- Pros: better multicore/SIMD usage
- Cons: more code complexity

2. Use integral image for fast block sums
- Impact: O(1) block sum after one pass
- Pros: efficient for repeated queries
- Cons: added memory and precision considerations

3. Move binning to camera hardware when available
- Impact: biggest CPU savings
- Pros: improves both compute and transfer volume
- Cons: hardware-dependent and calibration implications

---

## B12. Dynamic allocations in median filter implementation (`MedianFilter2`) if used
Evidence:
- `datapro1/src/dataproMedianFilter2.cpp` (frame clones, vectors per pixel in compute path)

What it affects:
- Extreme memory churn and CPU overhead
- Poor suitability for realtime at FullHD

Pros:
- Functional flexibility

Cons:
- Violates no-allocation hot-path requirement
- Not feasible for 125 FPS target

Solution options:
1. Keep this implementation out of production realtime path
- Impact: avoids severe regression
- Pros: immediate risk reduction
- Cons: reduced feature flexibility

2. Replace with dedicated K3/K5 fixed kernels with preallocated ring buffers
- Impact: predictable low-latency median stage
- Pros: aligns with current DP1-v2 plan
- Cons: implementation effort

---

## B13. High-cost normalization and template matching in optional filters
Evidence:
- `datapro1/src/dataproFilter.cpp` (`matchTemplate`, `normalize`, repeated convertTo)

What it affects:
- CPU-heavy optional paths
- Memory bandwidth via temporary mats

Pros:
- Good algorithmic expressiveness

Cons:
- Expensive in high-FPS operation

Solution options:
1. Keep these filters disabled in high-FPS profile
- Impact: preserves budget
- Pros: operationally simple
- Cons: reduced filtering capability

2. GPU/OpenCL accelerated implementation for these specific operators
- Impact: potential large speedup
- Pros: better throughput if pipeline remains mostly on device
- Cons: fallback and data-transfer overhead management required

---

## B14. Serialization and file save overhead in runtime path
Evidence:
- `datapro1/src/dataproSaveToFile.cpp` (multiple binary writes, Mat clone before save)
- `datapro1/src/dp1_rpc_data_mrsh.cpp` (copy and vector growth paths)

What it affects:
- Latency spikes when save paths enabled
- Memory copy overhead

Pros:
- Useful artifacts and traceability

Cons:
- Disk I/O and extra copy pressure

Solution options:
1. Disable save paths in realtime profile; run sampled capture mode separately
- Impact: immediate latency improvement
- Pros: minimal code change
- Cons: fewer full traces in production

2. Async file writer with bounded queue and drop policy
- Impact: bounded impact on frame thread
- Pros: preserves artifacts with controlled overhead
- Cons: queue management complexity

---

## B15. Per-frame logging volume in hot paths
Evidence:
- Multiple `LOG4CXX_INFO/DEBUG` in frame and worker loops, including per frame timing and scheduler traces

What it affects:
- CPU overhead
- Lock contention in logger backend
- I/O stalls depending on logger sink

Pros:
- Rich diagnostics

Cons:
- Noticeable runtime cost at high FPS

Solution options:
1. Reduce log level in production to WARN/ERROR and sample counters periodically
- Impact: lower overhead
- Pros: easy operational control
- Cons: less immediate debug detail

2. Structured metrics counters instead of frequent textual logging
- Impact: lower runtime overhead, better observability
- Pros: scalable telemetry
- Cons: instrumentation work

---

## B16. Config defaults are not aligned with high-FPS realtime profile
Evidence:
- `datapro1/config/config_datapro1.json` has `display_video=true`, `video_save=true`, `save2file.switched=true`

What it affects:
- Out-of-box performance and memory behavior
- Validation accuracy for 125 FPS goal

Pros:
- Demo-friendly default behavior

Cons:
- Misleading performance expectations for realtime target

Solution options:
1. Add dedicated `realtime_125fps` config profile with strict defaults
- Impact: predictable benchmarking and deployment behavior
- Pros: repeatable and safe operations
- Cons: profile management overhead

2. Runtime guard: refuse startup in high-FPS mode if heavy debug options enabled
- Impact: prevents accidental slow mode
- Pros: robust operation
- Cons: stricter startup checks

---

## B17. Contour pipeline depends on 8-bit mask boundaries and repeated morphology
Evidence:
- `datapro1/src/datarpoSegmentation.cpp` (`threshold`, `dilate`, `findContours`)

What it affects:
- CPU load and sensitivity to threshold tuning
- Potentially large contour counts in noisy scenes

Pros:
- Standard and understandable CV pipeline

Cons:
- Performance highly scene-dependent

Solution options:
1. Add pre-filtered candidate gating before full contour extraction
- Impact: fewer contours and lower compute cost
- Pros: stable performance under noise bursts
- Cons: possible recall trade-off

2. Connected-components based fast path for some modes
- Impact: may reduce overhead versus contour-heavy path
- Pros: predictable for binary masks
- Cons: requires metric parity checks

---

## B18. Threading model does not use explicit stage pipeline backpressure
Evidence:
- Current implementation primarily does per-frame tile parallelism then synchronous continuation

What it affects:
- Limited elasticity under burst load
- Harder QoS control across stages

Pros:
- Simpler control flow

Cons:
- Weak isolation between compute and transport/output phases

Solution options:
1. Introduce explicit stage queues (ingest -> median -> preprocess -> segment -> measure -> pack/send)
- Impact: better backpressure and QoS controls
- Pros: supports adaptive policies
- Cons: architectural complexity

2. Keep current model but decouple at least transport and optional output stages
- Impact: medium gain with lower refactor cost
- Pros: pragmatic migration path
- Cons: less complete than full staged pipeline

---

## Overall Feasibility Summary (Current Code)
- With current hot-path architecture and optional paths enabled, sustained 125 FPS FullHD is unlikely.
- Main blockers are not one operator but combined effects:
  - blocking I/O in frame thread,
  - excessive frame/tile copies,
  - expensive contour feature extraction loops,
  - polling-based synchronization,
  - diagnostics/output running in frame path.

## Recommended Priority Order
1. Remove/block heavy optional runtime paths in high-FPS profile (display/video/save/debug).
2. Decouple transport/output from frame compute thread.
3. Replace polling synchronization with event-driven completion.
4. Reduce memory copies in tiling and ingest handoff.
5. Optimize contour measurement stage (remove per-pixel polygon test loops where possible).
6. Implement and validate fixed K3/K5 temporal median with strict preallocation and no hot-path allocations.
7. Add stage-level telemetry and enforce deadline-based QoS actions.
