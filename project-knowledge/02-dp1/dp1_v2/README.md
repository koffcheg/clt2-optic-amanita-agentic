# DP1_v2 Documentation Workspace (AMNT-0006)

## Purpose

This directory is the canonical place for implementation documentation for DP1_v2/datapro1_v2 within AMNT-0006.

## Required documentation rules

- Document every completed implementation step.
- For each change, include:
  - code scheme name,
  - what was reused from legacy DP1,
  - what is new,
  - why this decision was made,
  - expected impact (functional, performance, maintainability).
- Keep DP1 -> DP2 contract-sensitive notes explicit.
- Keep notes concise, reproducible, and auditable.

## Initial document set

- DP1_V2_PLAN_MONO8_MONO16.md - primary refactoring plan for DP1_v2/datapro1_v2.
- DP1_V2_PERF_MEMORY_BOTTLENECKS.md - performance and memory bottleneck audit baseline.
- PHASE_TRACKER.md - phase status and mandatory legacy comparison checkpoints.
- CHANGE_LOG.md - chronological implementation notes and decisions.

## Naming

- Product name: DP1_v2.
- Runtime artifact/module name candidate: datapro1_v2.
- Every document should state which name is used in context.
