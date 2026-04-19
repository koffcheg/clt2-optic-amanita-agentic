# DP1_v2 Phase Tracker (AMNT-0006)

## Phase status

- Phase 0 (Bootstrap + config foundation): in progress
- Phase A (Functional parity + bottleneck-first refactor): planned, partially pre-implemented out of order (A.1/A.2 parked for reintegration)
- Phase B (Temporal median): planned
- Phase C (QoS/autotune + backend policy): planned
- Phase D (Side-by-side cutover): planned

## Current iteration focus

- Active phase: 0
- Active subtask: 0.3-startup-diagnostics-and-config-extension-points
- Scope guard: one-subtask iteration, no phase mixing, no DP1->DP2 contract drift, no new algorithmic migration before Phase 0 completion.

## Start of every iteration (AMNT-0009)

- phase: 0
- subtask id: 0.1-runnable-bootstrap-and-config-foundation
- goal: Establish strict startup/config prerequisite flow (launch + config read/validate/normalize) before resuming algorithmic migration subtasks.
- files_to_modify:
	- project-knowledge/02-dp1/dp1_v2/DP1_V2_PLAN_MONO8_MONO16.md
	- project-knowledge/02-dp1/dp1_v2/AGENT_EXECUTION_QUICKSTART.md
	- project-knowledge/02-dp1/dp1_v2/PHASE_TRACKER.md
	- project-knowledge/02-dp1/dp1_v2/CHANGE_LOG.md
	- project-knowledge/06-tasks/TASKS_INDEX.md
	- project-knowledge/06-tasks/cards/AMNT-0009.md
- files_read_only:
	- project-knowledge/02-dp1/dp1_v2/DP1_V2_PERF_MEMORY_BOTTLENECKS.md
	- project-knowledge/02-dp1/dp1_v2/README.md
	- project-knowledge/06-tasks/cards/AMNT-0008.md
	- datapro1_v2/src/main.cpp
	- datapro1_v2/include/dp1v2/frame_packet.hpp
- compatibility_risk: low
- executable_checks:
	- markdown consistency review
	- no runtime contract changes in code
- external_checks_needed:
	- confirm execution order agreement with project stakeholders
	- run next implementation iteration for Phase 0 runtime/config code

## Iteration invariants (must not be violated)

- Do not change DP1->DP2 contract: schema/message-id/serializer layout/transport semantics.
- Do not add new external dependencies.
- Do not perform broad refactoring or cross-phase changes.
- Do not rewrite legacy DP1 in place.
- Do not introduce intentional hot-path allocations for temporal median scope (future Phase B guardrail remains intact).
- Do not resume A.* algorithmic migration until Phase 0 checklist is Build-verified + Smoke-checked.

## End of iteration report (AMNT-0009 / 0.1)

- files_changed:
	- project-knowledge/02-dp1/dp1_v2/DP1_V2_PLAN_MONO8_MONO16.md
	- project-knowledge/02-dp1/dp1_v2/AGENT_EXECUTION_QUICKSTART.md
	- project-knowledge/02-dp1/dp1_v2/PHASE_TRACKER.md
	- project-knowledge/02-dp1/dp1_v2/CHANGE_LOG.md
	- project-knowledge/06-tasks/TASKS_INDEX.md
	- project-knowledge/06-tasks/cards/AMNT-0009.md
- implemented:
	- Added mandatory Phase 0 (bootstrap + config foundation) ahead of algorithmic migration.
	- Reordered checklist/subtask skeleton to follow general -> specific sequence.
	- Marked existing A.1/A.2 artifacts as parked for reintegration after Phase 0 completion.
- intentionally_not_changed:
	- DP1->DP2 wire/schema/message-id/serializer/transport contract.
	- Legacy DP1 runtime path and algorithm code.
	- Existing A.2 code artifacts (`frame_packet.hpp`, `main.cpp`) in `datapro1_v2`.
- build_result: N/A (documentation-only iteration)
- smoke_checks_run:
	- documentation consistency pass
- not_executable_here:
	- runtime validation of Phase 0 code path (not implemented in this documentation iteration)
- external_validation_required:
	- stakeholder confirmation of revised phase ordering
	- next code iteration to implement Phase 0 runtime/config prerequisites
- docs_updated: yes (`DP1_V2_PLAN_MONO8_MONO16.md`, `AGENT_EXECUTION_QUICKSTART.md`, `PHASE_TRACKER.md`, `CHANGE_LOG.md`, `TASKS_INDEX.md`, `AMNT-0009.md`)
- next_safe_step:
	- Implement Phase 0 runtime/config code subtask 0.2, then resume parked A.2 work only after Phase 0 completion.

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

## Parked artifacts awaiting reintegration

- A.1 scaffold-build artifacts: implemented out of order, reusable after Phase 0 completion.
- A.2 ingest/frame contract artifacts: implemented out of order, reusable after Phase 0 completion.

## Delta update (AMNT-0009 / Phase 0.2)

- Status: Build-verified + Smoke-checked
- Implemented:
	- Minimal typed config layer for `datapro1_v2` (`SourceConfig`, `Dp2ConnConfig`, `RuntimeConfig`).
	- JSON load/validation for minimal base sections: `config.source` + `config.dp2conn`.
	- Startup logic extracted from `main.cpp` to `startup.hpp/.cpp`.
	- Startup contract fixed to CLI-only config path: `datapro1_v2 <cam_index> <config_path>`.
- Validation:
	- `cmake --preset host-debug-full`
	- `cmake --build --preset host-debug-datapro1v2`
	- `./build-host-debug-full/datapro1_v2/datapro1_v2 0 datapro1_v2/config/config_datapro1_v2.json`
- Next safe step:
	- Phase 0.3: improve startup diagnostics contract and prepare first config extension points for A.3 source adapters.

Note:
- Detailed task narrative is intentionally kept in `project-knowledge/06-tasks/cards/AMNT-0009.md` to avoid duplication.
