# DP1_v2 Change Log (AMNT-0006)

## Entry template

### YYYY-MM-DD - Short title

- Code scheme:
- Reused from legacy:
- New implementation:
- Why:
- Expected effect:
- Contracts touched:
- Related files:
- Notes:

---

## 2026-04-13 - Repository build entrypoints aligned to preset flow

- Code scheme: build.system.preset_flow_alignment
- Reused from legacy: Existing `builder/*.sh` entrypoints and docker-compose command surface.
- New implementation: Added `CMakePresets.json` as canonical flow; migrated build scripts to wrapper-mode over presets; documented preset flow in README and PROJECT_ECOSYSTEM.
- Why: Avoid environment-specific drift and establish a single repository-level build source of truth.
- Expected effect: Consistent configure/build commands across local, docker, and automation entrypoints without `.vscode`/machine-specific repo edits.
- Contracts touched: none
- Related files:
  - CMakePresets.json
  - cmake/opencv_deps_preload.cmake
  - builder/build_dp1_dp2.sh
  - builder/build_all.sh
  - builder/build_datapro1.sh
  - builder/build_datapro2.sh
  - builder/build_camerapro.sh
  - builder/build_manager.sh
  - builder/build_calibration.sh
  - README.md
  - project-knowledge/01-project/PROJECT_ECOSYSTEM.md
- Notes: Functional verification remains environment-dependent; unresolved dependency/toolchain issues are treated as external blockers.

Update (same date):
- Added host/docker preset split and wrapper auto-select logic to avoid using docker preset on host accidentally.
- Verified `datapro1_v2` build via `host-debug-datapro1v2` preset.
- Verified `build_all.sh` on host with dependency hints (`OPENCV_DIR`, `BOOST_ROOT`) through canonical wrapper flow.

## 2026-04-13 - Phase A subtask A.1 scaffold-build integration build-verified

- Code scheme: dp1v2.phaseA.a1.scaffold_build
- Reused from legacy: Top-level CMake submodule wiring pattern.
- New implementation: Added minimal `datapro1_v2` module scaffold with target-level CMake, `main.cpp`, and initial `FramePacket` header.
- Why: Start parallel DP1_v2 implementation path with minimal and reversible scope.
- Expected effect: Independent `datapro1_v2` target becomes available for next subtasks.
- Contracts touched: none
- Related files:
  - CMakeLists.txt
  - datapro1_v2/CMakeLists.txt
  - datapro1_v2/src/main.cpp
  - datapro1_v2/include/dp1v2/frame_packet.hpp
- Notes: Build verification executed via repository-approved flow; environment blockers must be reported without repo-level environment hacks.

## 2026-04-10 - Agent execution quickstart layer

- Code scheme: docs.agent.quickstart.execution_layer
- Reused from legacy: Existing phase model, constraints, and validation split from DP1_v2 main plan.
- New implementation: Added a compact execution wrapper with iteration start block, context budget rules, invariant list, phase completion checklists, and iteration-end reporting format.
- Why: Reduce context drift and phase mixing risk during long AI-agent sessions without duplicating the main plan.
- Expected effect: More stable step-by-step execution, fewer oversized patches, and clearer implementation-complete signaling.
- Contracts touched: none
- Related files:
  - project-knowledge/02-dp1/dp1_v2/AGENT_EXECUTION_QUICKSTART.md
  - project-knowledge/02-dp1/dp1_v2/README.md
  - project-knowledge/02-dp1/dp1_v2/DP1_V2_PLAN_MONO8_MONO16.md
  - project-knowledge/02-dp1/dp1_v2/CHANGE_LOG.md
- Notes: This is an operational documentation layer; technical authority stays in DP1_V2_PLAN_MONO8_MONO16.md.

## 2026-04-08 - Documentation workspace initialization

- Code scheme: docs.workspace.bootstrap
- Reused from legacy: N/A
- New implementation: Created DP1_v2 documentation workspace and baseline tracker files.
- Why: Enforce strict traceability requirements from updated plan.
- Expected effect: Better auditability and controlled phased execution.
- Contracts touched: none
- Related files:
  - project-knowledge/02-dp1/dp1_v2/README.md
  - project-knowledge/02-dp1/dp1_v2/PHASE_TRACKER.md
  - project-knowledge/02-dp1/dp1_v2/CHANGE_LOG.md
- Notes: This is documentation-only initialization.

## 2026-04-08 - Plan operational guardrails and validation split

- Code scheme: docs.plan.guardrails_and_acceptance_split
- Reused from legacy: Existing phased DP1_v2 roadmap, compatibility boundaries, and realtime constraints already documented in plan.
- New implementation: Extended the plan with explicit AI-agent operational contract, scoped change boundaries, allowed/forbidden reuse, hot-path engineering rules, memory ownership requirements, phase-level implementation-complete vs external acceptance split, status vocabulary, parity comparison contract, failure/uncertainty handling, task slicing, and iteration reporting format.
- Why: Prevent uncontrolled scope expansion and false acceptance claims in constrained agent environment while preserving project acceptance bar.
- Expected effect: Safer execution discipline, clearer compatibility control, reproducible documentation per step, and explicit separation of agent-verifiable checks from external validation.
- Contracts touched: documentation-level only; no runtime protocol/schema change.
- Related files:
  - project-knowledge/02-dp1/dp1_v2/DP1_V2_PLAN_MONO8_MONO16.md
  - project-knowledge/02-dp1/dp1_v2/CHANGE_LOG.md
- Notes: Plan structure and existing technical decisions were preserved; additions were integrated into existing sections where possible.
