# DP1_v2 Agent Execution Quick Start (AMNT-0006)

## Purpose

This file is the execution layer for AI-agent work on DP1_v2.
It does not replace the main plan. It enforces short-cycle execution and context safety.

## Start of every iteration (mandatory)

Fill this block before code edits:

- phase: A | B | C | D
- subtask id: one item from plan task slicing
- goal: one sentence
- files_to_modify: explicit list
- files_read_only: explicit list
- compatibility_risk: none | low | medium | high
- executable_checks: build/smoke checks available now
- external_checks_needed: checks not executable in current environment

If phase/subtask is not explicit, stop and define it first.

## Context budget rules

- One iteration = one subtask.
- Do not mix multiple phases in one patch.
- Keep edits narrow: recommended up to 8 modified files per iteration.
- If scope grows, split into next iteration and document why.
- Re-read before each new iteration:
  - current phase and subtask,
  - DP1->DP2 compatibility freeze constraints,
  - last entry in CHANGE_LOG.md,
  - current PHASE_TRACKER.md status.

## Invariants (must stay true in early phases)

- No DP1->DP2 schema/message-id/serializer drift.
- No broad legacy rewrite in place.
- No new external dependencies unless explicitly approved.
- No hidden hot-path allocations for temporal median stage.

## Implementation-complete checklist by phase

### Phase A

- parallel DP1_v2/datapro1_v2 path exists and builds
- mono input path is implemented in scope
- pack/send compatibility boundary preserved
- planned bottleneck items for this subtask are fixed or explicitly deferred with reason
- baseline parity validation checklist is prepared

### Phase B

- temporal median K3 and K5 modes are implemented
- buffers are preallocated
- no intentional hot-path allocations in temporal median stage
- toggles are wired and documented
- latency/functional external check list is prepared

### Phase C

- telemetry hooks implemented for required stages
- controller states/actuators are implemented
- processing mode policy and fallback are deterministic
- overload/realtime external check list is prepared

### Phase D

- side-by-side procedure and metrics are prepared
- rollback switch path is documented
- status is not marked accepted without external validation evidence

## End of iteration report (mandatory)

Record concise end-of-iteration status:

- files_changed
- implemented
- intentionally_not_changed
- build_result
- smoke_checks_run
- not_executable_here
- external_validation_required
- docs_updated
- next_safe_step

## Allowed status words

- Implemented
- Build-verified
- Smoke-checked
- Ready for external validation
- Externally validation-pending
- Blocked

Do not use accepted/completed wording without external validation record.
