# PROJECT_INDEX

## Purpose

This is the root index for Project Knowledge in `clt2-optic_amanita`. It explains the system, its modules, the high-level pipeline, the Project Knowledge layout, and the recommended entry points for humans and AI agents.

Unlike `AGENTS.md`, this file does not define operating rules for agents. It explains what the system is and where to find subsystem knowledge.

## Project Summary

`clt2-optic_amanita` is a multi-component computer vision project for optical data processing.

Key modules for Project Knowledge:
- `camerapro` - frame capture or frame delivery.
- `datapro1` - DP1, primary frame processing.
- `datapro2` - DP2, downstream processing of DP1 results.
- `common/*` - shared models, frame utilities, turret/common logic.
- `builder/*`, `docker-compose.yml`, `overseer/*` - build, orchestration, remote/automation tooling.

## High-Level Pipeline

Current legacy runtime can be read as:

1. A frame source or upstream component produces input data.
2. `camerapro` and/or runners deliver frames to DP1.
3. `datapro1` performs primary frame processing and produces frame-level results.
4. DP1 may send results to DP2, store `.blob` / `.json`, and create diagnostic artifacts.
5. `datapro2` receives DP1 results and performs downstream processing, aggregation, or tracking.
6. Helper modules and GUI components are used for debugging, validation, and integration scenarios.

Canonical DP1/DP2 development must not treat this legacy runtime as target architecture.

## Knowledge Base State

Project Knowledge separates:
- canonical knowledge - target architecture and formal specification layer;
- legacy knowledge - existing DP1/DP2 behavior and current-code references;
- governance knowledge - agent rules, templates, access control, source-of-truth rules;
- protocol knowledge - shared modules boundaries (e.g. DP1 <-> DP2);
- validation knowledge - conformance and execution validation routes.

## Layout

```text
project-knowledge/
  00-governance/
    CARD_TEMPLATE.md
    KNOWLEDGE_BASE_ROADMAP.md
    CODE_STYLE.md
    TESTING_POLICY.md
    LOGGING_POLICY.md
    PROFILING_POLICY.md
    TASK_CARD_TEMPLATE.md
  01-project/
    PROJECT_ECOSYSTEM.md
  02-dp1/
    DP1_INDEX.md
    DP1_CARDS_INDEX.md
    canonical/
    legacy/
  03-dp2/
    DP2_INDEX.md
    DP2_CARDS_INDEX.md
    canonical/
    legacy/
  04-protocols/
    PROTOCOLS_INDEX.md
    cards/
  05-validation/
    VALIDATION_INDEX.md
    UNIT_TESTING_GUIDE.md
    AMANITA_COMPARATOR_E2E_VALIDATION_WORKFLOW.md
    cards/
  06-tasks/
    TASKS_INDEX.md
    cards/
```

Section roles:
- `00-governance/` - templates, rules, roadmap, and agent-facing policy,
  including code style, testing, logging, and profiling policies.
- `01-project/` - project ecosystem and environment knowledge.
- `02-dp1/` - DP1 knowledge split into canonical and legacy.
- `03-dp2/` - DP2 knowledge split into canonical and legacy.
- `04-protocols/` - shared contracts between modules.
- `05-validation/` - testing taxonomy, validation workflows, regression scenarios, and test assets.
- `06-tasks/` - task cards with scope, constraints, risks, and results.

## Entry Points

### Project Overview

Read:
- this file;
- `01-project/PROJECT_ECOSYSTEM.md`;
- root `README.md`;
- root `CMakeLists.txt`.

### DP1

Read:
- `02-dp1/DP1_INDEX.md`;
- for canonical work: `02-dp1/canonical/DP1_CANONICAL_INDEX.md`;
- for legacy analysis: `02-dp1/legacy/DP1_LEGACY_INDEX.md`.

### DP2

Read:
- `03-dp2/DP2_INDEX.md`;
- for canonical work: `03-dp2/canonical/DP2_CANONICAL_INDEX.md`;
- for legacy analysis: `03-dp2/legacy/DP2_LEGACY_INDEX.md`;
- `04-protocols/*` for shared contracts.

### Shared Contracts, Serialization, Network, File Exchange

Read:
- `04-protocols/PROTOCOLS_INDEX.md`;
- relevant DP1/DP2 cards;
- DP1 handoff/serialization code and DP2 receive-path code when the task explicitly allows code verification.

### Build, Docker, Dependencies, OS

Read:
- `01-project/PROJECT_ECOSYSTEM.md`;
- `builder/Dockerfile`;
- `docker-compose.yml`;
- `builder/*.sh`;
- `README.md`.

### Testing And Validation

Read:
- `05-validation/VALIDATION_INDEX.md`;
- `05-validation/UNIT_TESTING_GUIDE.md`;
- `05-validation/AMANITA_COMPARATOR_E2E_VALIDATION_WORKFLOW.md`;
- `00-governance/TESTING_POLICY.md`;
- `06-tasks/TASKS_INDEX.md`.

## DP1 Status

DP1 has a physical canonical/legacy split.

Canonical DP1 defines:
- product definition and source-of-truth route;
- Stage0 Input Normalization as the canonical input boundary before Prep;
- formal pipeline model `Π`;
- data domains;
- stage-interface placeholders;
- configuration model `C`;
- validation conformance route.

Legacy DP1 describes old `datapro1` behavior. It includes data structures, frame boundaries, runtime buffers, configuration, tile processing, serialization, DP1 -> DP2 transport, and local file output.

Legacy DP1 is not target architecture for canonical DP1.

## DP2 Status

DP2 has a canonical placeholder structure and a legacy-reference card set:
- `03-dp2/canonical/DP2_CANONICAL_INDEX.md`;
- `03-dp2/legacy/DP2_LEGACY_INDEX.md`.

Current legacy coverage includes tracking structures, configuration structures, runtime boundaries, receive path DP1 -> DP2, and intersections with DP1 types.

The next DP2 work is to formalize canonical DP2 product, pipeline, domains, configuration, and validation route.

## Ecosystem Knowledge

All information about language stack, build system, libraries, versions, Docker, OS, config formats, and important external dependencies belongs in:
- `01-project/PROJECT_ECOSYSTEM.md`

Do not scatter environment facts across DP1/DP2 cards.

## Near-Term Knowledge Base Priorities

1. Prepare Stage0.1 implementation from the canonical Input Normalization and CanonicalFrame baseline.
2. Write stage specifications for canonical DP1.
3. Formalize configuration schema `C` and complexity budgets.
4. Formalize canonical DP2 beyond placeholders.
5. Extend `04-protocols/` with concrete canonical payload schemas.
6. Extend `05-validation/` with conformance checks, datasets, and expected checks.
