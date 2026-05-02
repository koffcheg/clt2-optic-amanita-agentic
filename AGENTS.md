# AGENTS.md

## Purpose

This file defines the base operating rules for AI agents in the `clt2-optic_amanita` repository. It routes agents to the right Project Knowledge files and defines how code and documentation stay synchronized.

## Knowledge Base Location

Project Knowledge lives at the repository root under `project-knowledge/`.

Expected layout:
- `AGENTS.md` - agent rules and navigation.
- `project-knowledge/PROJECT_INDEX.md` - project map and domain knowledge entry point.
- `project-knowledge/01-project/PROJECT_ECOSYSTEM.md` - ecosystem, stack, build, dependencies, OS.
- `project-knowledge/02-dp1/` - DP1 knowledge.
- `project-knowledge/03-dp2/` - DP2 knowledge.
- `project-knowledge/04-protocols/` - inter-module contracts, wire formats, IPC/TCP/RPC.
- `project-knowledge/05-validation/` - tests, mocks, regression scenarios, validation notes.
- `project-knowledge/00-governance/CODE_STYLE.md` - code style rules.
- `project-knowledge/00-governance/TESTING_POLICY.md` - testing rules.
- `project-knowledge/06-tasks/TASKS_INDEX.md` - task card index.
- `project-knowledge/06-tasks/cards/` - task cards.
- `project-knowledge/00-governance/TASK_CARD_TEMPLATE.md` - task card template.

## Source Of Truth

If sources conflict, use this order:

1. Explicit user instructions in the current task.
2. `AGENTS.md`.
3. `project-knowledge/00-governance/CODE_STYLE.md`.
4. `project-knowledge/00-governance/TESTING_POLICY.md`.
5. Project Knowledge under `project-knowledge/`.
6. Code, `CMakeLists.txt`, `.h/.hpp/.cpp/.cc`, JSON/XML configs, Dockerfile, shell scripts.
7. Build/run artifacts: `README.md`, `builder/`, `docker-compose.yml`, `overseer/`, `config/`.
8. Old notes, drafts, and stale descriptions.

For target canonical architecture, canonical Project Knowledge defines intended design.

For existing runtime behavior, build behavior, configuration behavior, exchange formats, and protocol implementation facts, code and configs remain authoritative.

If canonical Project Knowledge and code/configs conflict, the agent must report the mismatch, classify it as canonical gap / runtime deviation / evidence gap, list affected files, and propose minimal reconciliation scope. The agent must not silently rewrite code or Project Knowledge to hide the conflict.

## Documentation Language

English is the canonical language for agent-facing documentation. Use short, precise technical English.

Russian or Ukrainian may remain only in human-facing notes or explicitly bilingual fields, such as:

```yaml
title:
  uk:
  en:
```

If a meaning is unclear, write `TODO: confirm with user` instead of inventing details.

## Canonical / Legacy Access

For new canonical DP1/DP2 development, read canonical knowledge first:
- `project-knowledge/02-dp1/canonical/`
- `project-knowledge/03-dp2/canonical/`
- shared canonical contracts in `project-knowledge/04-protocols/`
- validation knowledge in `project-knowledge/05-validation/`

Legacy knowledge describes how old DP1/DP2 work. Legacy is not a target-architecture source.

During `canonical_development`, legacy is forbidden by default. Legacy may be read only when the current task card has `legacy_access: allowed` and lists exact `allowed_legacy_sources`.

If a task card does not allow legacy access, do not use legacy as a design source. If canonical and legacy conflict, canonical wins. If a card and informal text conflict, the card wins.

Card `source_role` controls code-generation source eligibility:
- `source_role: canonical` is used by default for canonical design and code-generation inputs.
- `source_role: legacy-reference` must not be used for code generation unless the active task card explicitly allows legacy access and lists exact legacy sources.
- `source_role: verification` may constrain validation and acceptance, but does not replace canonical product, data, protocol, stage, or configuration cards.
- `source_role: draft` is not sufficient for code generation until promoted.

Code must not be generated from legacy code, legacy-reference cards, or informal text unless explicit task-card rules allow the exact legacy source for a bounded migration task.

Canonical code generation route:

```text
Code = f(Cards, Stage_Spec, C)
```

Where:
- `Cards` are data-structure, data-domain, protocol, and stage-interface cards.
- `Stage_Spec` is the formal small specification for a stage.
- `C` is the pipeline configuration.

If a required card, stage specification, or configuration `C` is missing, stop and propose the missing source. Do not generate code from descriptive text.

## Change Control

### Project Knowledge Changes

Project Knowledge is a controlled project artifact.

Do not do the following without explicit approval:
- edit `project-knowledge/PROJECT_INDEX.md`;
- create, modify, rename, or delete knowledge cards;
- change the Project Knowledge structure;
- edit card templates;
- edit roadmap files;
- perform mass synchronization or note restructuring;
- edit `CODE_STYLE.md`, `TESTING_POLICY.md`, or `PROJECT_ECOSYSTEM.md`.

If Project Knowledge is stale, incomplete, or contradicts code:
1. Describe the problem.
2. List affected files.
3. Propose exact changes.
4. Wait for explicit approval.

Exception: task cards under `project-knowledge/06-tasks/cards/` may be created or updated for the current approved task when the workflow requires it. Task cards are not domain knowledge cards and do not replace Project Knowledge updates.

### Code Changes

Prefer minimal, local, verifiable changes.

Do not do the following without explicit approval:
- broad refactoring;
- architecture changes;
- public contract or exchange-format changes;
- new libraries, frameworks, or external tools;
- CI/CD, Docker, build pipeline, or deployment logic changes;
- mass renames of files, symbols, or directories.

### Testing Changes

Do not do the following without explicit approval:
- create new automated tests;
- create fixtures, mocks, stubs, snapshots, or golden files;
- add test infrastructure;
- mass-rewrite existing tests;
- change the project testing strategy.

During Amanita/Comparator execution test runs, do not change production Amanita or Comparator code.

Allowed actions:
- prepare per-run configs and scripts in the run directory;
- run binaries or CLI commands;
- collect logs, artifacts, and reports.

If tests are needed, describe the test plan and wait for approval instead of creating tests automatically.

## Required Start

Before any non-trivial task:

1. Ask user if card is needed, if not needed then skip list items 2-4
2. Check whether a task card exists for the current task.
3. If no task card exists, create one under `project-knowledge/06-tasks/cards/` from `project-knowledge/00-governance/TASK_CARD_TEMPLATE.md`.
4. Fill in description, scope, constraints, relevant files, expected changes, and risks.
5. Read `project-knowledge/PROJECT_INDEX.md`.
6. Read the relevant subsystem index or topic file.
7. Verify code-backed claims when the task concerns runtime behavior, contracts, or data formats.
8. After changes, check whether Project Knowledge, contracts, pipeline, or ecosystem are affected.
9. If Project Knowledge needs updates outside the approved scope, propose exact changes and wait for approval.

## Reading Routes

### Project Architecture

Read:
- `project-knowledge/PROJECT_INDEX.md`
- `project-knowledge/01-project/PROJECT_ECOSYSTEM.md`
- root `README.md`
- root `CMakeLists.txt`

### Build, Dependencies, Docker, OS, Toolchain

Read:
- `project-knowledge/01-project/PROJECT_ECOSYSTEM.md`
- `README.md`
- `builder/Dockerfile`
- `docker-compose.yml`
- `builder/*.sh`
- relevant `CMakeLists.txt`

### DP1

Read:
- `project-knowledge/02-dp1/DP1_INDEX.md`
- for canonical work: `project-knowledge/02-dp1/canonical/DP1_CANONICAL_INDEX.md`
- for legacy analysis: `project-knowledge/02-dp1/legacy/DP1_LEGACY_INDEX.md`
- legacy code/cards only when the task card explicitly allows legacy access

### DP2

Read:
- `project-knowledge/03-dp2/DP2_INDEX.md`
- for canonical work: `project-knowledge/03-dp2/canonical/DP2_CANONICAL_INDEX.md`
- for legacy analysis: `project-knowledge/03-dp2/legacy/DP2_LEGACY_INDEX.md`
- shared protocol files under `project-knowledge/04-protocols/`
- legacy code/cards only when the task card explicitly allows legacy access

### Inter-Module Contracts, TCP/RPC, Serialization, Exchange Files

Read:
- `project-knowledge/04-protocols/PROTOCOLS_INDEX.md`
- relevant DP1/DP2 cards
- `datapro1/src/dp1_tr_res2dp2.cpp`
- `datapro1/src/dp1_rpc_data_mrsh.cpp`
- DP2 receive-path code

### Tests, Mocks, Scenario Validation

Read:
- `project-knowledge/05-validation/`
- `gtests/`
- `test.mock/`
- `overseer/` when the task concerns orchestration or remote runs

### Code Style And Change Rules

Read:
- `AGENTS.md`
- `project-knowledge/00-governance/CODE_STYLE.md`
- `project-knowledge/00-governance/TESTING_POLICY.md`

### Current Task Scope

Read:
- `project-knowledge/06-tasks/TASKS_INDEX.md`
- current task card `project-knowledge/06-tasks/cards/AMNT-XXXX.md`
- `AGENTS.md`
- relevant domain files when needed

## When Project Knowledge Needs Updates

Project Knowledge needs an update when an agent:
- adds a new entity affecting architecture or contracts;
- changes data format, serialization, network payload, or file output;
- changes build/runtime environment, dependencies, or toolchain;
- changes high-level pipeline or module responsibilities;
- finds that an existing card contradicts code.

By default, do not apply such updates automatically.

Instead:
- list the exact Project Knowledge files that need changes;
- explain why;
- propose the minimal synchronization scope;
- wait for explicit approval.

Possible minimal synchronization scope:
- a relevant card or a new card;
- `PROJECT_INDEX.md` if knowledge navigation or section structure changed;
- `PROJECT_ECOSYSTEM.md` if environment facts changed.

## Card Creation Rules

New cards may be created only with explicit approval or when the current task directly requires Project Knowledge updates.

The project has two card types:
- domain cards - knowledge about system entities, contracts, structures, pipelines, and environment;
- task cards - cards for specific execution tasks.

Project Knowledge also contains agent-facing operational documents. They are not cards, but they may be authoritative routing or governance sources for agents. Examples:
- `AGENTS.md`;
- `project-knowledge/PROJECT_INDEX.md`;
- subsystem indexes;
- `*_CANONICAL_INDEX.md`;
- `*_LEGACY_INDEX.md`;
- governance templates and policies;
- protocol and validation indexes.

Agent-facing operational documents stay in English.

Rules for `project.*`, `dp1.*`, `dp2.*`, `protocols.*`, and `validation.*` ID spaces apply to domain cards. Task cards use `AMNT-XXXX` IDs.

Principles:
- One card describes one entity or one narrow contract.
- Do not mix a structure, an algorithm, and a network protocol in one card unless they are the same concept.
- If information is not code-verified, mark it in `Assumptions` or `Open questions`.
- If an entity is shared between DP1 and DP2, place the card in `04-protocols/` or link it from that canonical index.

Task card rules:
- one task card equals one task;
- filename equals task ID;
- ID format is `AMNT-XXXX`;
- create the task card before non-trivial task execution;
- include description, scope, constraints, planned changes, risks, and validation approach;
- task cards are not authoritative sources for domain knowledge.

Recommended card ID namespaces:
- `project.*` - general architecture and project entities;
- `ecosystem.*` - stack, build, runtime, dependencies;
- `dp1.types.*`, `dp1.frame.*`, `dp1.runtime.*`, `dp1.rpc.*`, `dp1.net.*`, `dp1.io.*`, `dp1.tiles.*`, `dp1.config.*`;
- `dp2.types.*`, `dp2.runtime.*`, `dp2.net.*`, `dp2.track.*`, `dp2.io.*`, `dp2.config.*`;
- `protocols.*` - shared wire/file/message contracts;
- `validation.*` - test assets, scenarios, regressions.

Each new card must include:
- YAML front matter with `id`, `title`, `tags`, `source`, and `status`;
- `Definition`;
- `Assumptions`;
- `Theorem / Contract`;
- `Interpretation`;
- `Failure cases`;
- `Typical misuse`;
- `Open questions`, if needed;
- `Connections`.

The card template is `project-knowledge/00-governance/CARD_TEMPLATE.md`.

## Ecosystem Knowledge

Put all language, standard, library, OS, Docker, build tool, config format, and runtime dependency facts in:
- `project-knowledge/01-project/PROJECT_ECOSYSTEM.md`

This includes:
- languages and standards;
- main libraries and their role;
- target OS and build container;
- build system and minimal commands;
- authoritative source files for environment knowledge.

If the content grows, split it into child documents such as:
- `project-knowledge/01-project/BUILD_AND_RUN.md`
- `project-knowledge/01-project/DEPENDENCIES.md`
- `project-knowledge/01-project/REPO_LAYOUT.md`

## Missing Or Conflicting Knowledge

If Project Knowledge is incomplete, do not invent facts.

Do this instead:
- verify code and config files;
- describe exactly what is missing;
- list cards or indexes that should be updated;
- mark `Open questions`;
- propose creating or updating a draft card, but wait for approval.

## Documentation Definition Of Done

This applies only to tasks where documentation updates were explicitly approved.

Documentation work is done when:
- new or changed technical facts are verified against code;
- relevant cards are updated;
- indexes are updated when navigation changed;
- `PROJECT_ECOSYSTEM.md` is updated when environment facts changed;
- Project Knowledge has no silent conflicts between code and documentation.
