# DP1_v2 Phase Tracker (AMNT-0006)

## Phase status

- Phase A (Iteration 1): planned
- Phase B (Temporal median): planned
- Phase C (QoS/autotune + backend policy): planned
- Phase D (Side-by-side cutover): planned

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
