# DP1_v2 Phase Tracker (AMNT-0006)

## Phase status

- Phase A (Iteration 1): in progress (subtask A.2 ingest/frame contract)
- Phase B (Temporal median): planned
- Phase C (QoS/autotune + backend policy): planned
- Phase D (Side-by-side cutover): planned

## Current iteration focus

- Active phase: A
- Active subtask: A.2-ingest-frame-contract
- Scope guard: one-subtask iteration, no phase mixing, no DP1->DP2 contract drift.

## Start of every iteration (AMNT-0008)

- phase: A
- subtask id: A.2-ingest-frame-contract
- goal: Define and wire minimal ingest/frame contract boundaries for DP1_v2 mono path without touching transport/schema compatibility points.
- files_to_modify:
	- datapro1_v2/include/dp1v2/frame_packet.hpp
	- datapro1_v2/src/main.cpp
	- datapro1_v2/CMakeLists.txt
	- project-knowledge/02-dp1/dp1_v2/PHASE_TRACKER.md
	- project-knowledge/02-dp1/dp1_v2/CHANGE_LOG.md
- files_read_only:
	- project-knowledge/02-dp1/dp1_v2/AGENT_EXECUTION_QUICKSTART.md
	- project-knowledge/02-dp1/dp1_v2/DP1_V2_PLAN_MONO8_MONO16.md
	- datapro1/src/dp1_frame_proc.cpp
	- datapro1/src/dp1_tr_res2dp2.cpp
	- datapro1/src/dp1_rpc_data_mrsh.cpp
	- datapro2/src/dp2_rpc_cl.cpp
- compatibility_risk: low
- executable_checks:
	- builder/build_datapro1.sh
	- cmake --preset host-debug-full
	- cmake --build --preset host-debug-datapro1v2
- external_checks_needed:
	- DP1_v2 vs legacy functional parity on target streams
	- End-to-end DP1->DP2 integration run in target environment
	- Runtime behavior confirmation for FullHD 30 FPS baseline

## Iteration invariants (must not be violated)

- Do not change DP1->DP2 contract: schema/message-id/serializer layout/transport semantics.
- Do not add new external dependencies.
- Do not perform broad refactoring or cross-phase changes.
- Do not rewrite legacy DP1 in place.
- Do not introduce intentional hot-path allocations for temporal median scope (future Phase B guardrail remains intact).

## End of iteration report (AMNT-0008 / A.2)

- files_changed:
	- datapro1_v2/include/dp1v2/frame_packet.hpp
	- datapro1_v2/src/main.cpp
	- datapro1_v2/CMakeLists.txt
	- project-knowledge/02-dp1/dp1_v2/PHASE_TRACKER.md
	- project-knowledge/02-dp1/dp1_v2/CHANGE_LOG.md
	- project-knowledge/06-tasks/TASKS_INDEX.md
	- project-knowledge/06-tasks/cards/AMNT-0008.md
- implemented:
	- Explicit `FramePacket` ingest boundary with metadata hints.
	- Deterministic pixel-type resolution policy (header-first, mat-depth fallback).
	- Typed ingest result status: `FramePacketBuildError` + `FramePacketBuildResult`.
	- Zero-allocation error-to-string helper for diagnostics.
- intentionally_not_changed:
	- DP1->DP2 wire/schema/message-id/serializer/transport contract.
	- Legacy DP1 runtime path and cross-phase functionality (B/C/D).
- build_result: Build-verified
- smoke_checks_run:
	- `cmake --preset host-debug-full`
	- `cmake --build --preset host-debug-datapro1v2`
	- `./build-host-debug-full/datapro1_v2/datapro1_v2`
- not_executable_here:
	- Full e2e DP1_v2 ingestion from IPC/campro and URI/file sources.
	- External parity checks on target streams.
- external_validation_required:
	- Legacy-vs-v2 functional parity on agreed target scenarios.
	- End-to-end DP1->DP2 integration behavior in target runtime environment.
- docs_updated: yes (`PHASE_TRACKER.md`, `CHANGE_LOG.md`, `TASKS_INDEX.md`, `AMNT-0008.md`)
- next_safe_step:
	- Start A.3 (source adapters) as a separate narrow subtask within Phase A.

## Mandatory per-phase checkpoint template

For each phase, fill:

- Scope completed
- Functional parity status vs legacy
- Performance comparison vs legacy
- Integration status (DP1 -> DP2 and other modules)
- Artifact compatibility status
- Open risks / blockers
- Decision to proceed to next phase (yes/no + rationale)

## Iteration 1 success criteria (Phase A)

- FullHD 30 FPS stable operation.
- Best-effort (non-SLA) operation up to 130 FPS accepted as unstable mode.
- Mono path implemented in first iteration.
- Legacy-compatible functional behavior achieved without legacy runtime core reuse.
- Major bottlenecks from perf audit addressed in this phase.
