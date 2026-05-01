# KNOWLEDGE_BASE_ROADMAP

## Purpose

Bring Project Knowledge to a state where an AI agent can:
- understand the project structure quickly;
- find domain knowledge without excessive searching;
- keep code and documentation synchronized;
- create new cards from one template;
- avoid mixing architecture, ecosystem facts, and local details in one file.

## Agent-Ready Definition

Project Knowledge is agent-ready when:
- the repository has `AGENTS.md`;
- `PROJECT_INDEX.md` exists as the project map;
- `PROJECT_ECOSYSTEM.md` is the single place for environment knowledge;
- key subsystems have indexes and canonical reading routes;
- new cards can be created from a single template;
- shared contracts are not hidden inside one subsystem;
- Project Knowledge does not contradict code on critical paths;
- canonical and legacy knowledge are clearly separated.

## Priorities

### P0. Integrate Project Knowledge Into The Repository

Status: completed.

### P1. Normalize Top-Level Documentation

Status: completed and maintained.

### P2. DP1 Knowledge

Status: in progress.

Focus: canonical/legacy split, canonical DP1 cards, validation route, and synchronization with shared protocol cards.

### P3. DP2 Knowledge

Status: in progress.

Focus: canonical placeholders, legacy-reference isolation, and future canonical DP2 detail.

### P4. Shared Contracts

Status: in progress.

Focus: canonical protocol boundary cards, including DP1 -> DP2 Measurement handoff.

### P5. Validation Layer

Status: in progress.

Completed: `05-validation/VALIDATION_INDEX.md` and canonical AI-agent testing workflow.

Focus: regression scenarios and dataset -> expected check mapping.

## Recommended Improvements

### Separate File Roles

- `AGENTS.md` - agent rules.
- `PROJECT_INDEX.md` - project map.
- `PROJECT_ECOSYSTEM.md` - stack and environment.
- Cards - atomic technical facts.

### Keep `AGENTS.md` Operational

`AGENTS.md` should remain an operational layer, not a full project description. Long descriptions belong in indexes and topic files.

### Keep Environment Knowledge Canonical

Stack, build, Docker, OS, and dependency facts belong in `01-project/PROJECT_ECOSYSTEM.md`.

### Keep Shared Protocols In A Shared Layer

Shared modules boundaries (e.g. DP1 <-> DP2) belong in `04-protocols/`, not only in DP1 or DP2.

### Strengthen Card Metadata

Required fields:
- `id`
- `source.file`
- `source.lines`
- `status`
- `tags`

Useful fields:
- `kind`
- `source_role`
- `last_verified`
- `verified_against`
- `review_notes`

### Prefer Indexes Over Unstructured Card Growth

Each major section needs an index. The index tells agents where to start.

### Use Maturity Statuses

Recommended statuses:
- `draft`
- `verified`
- `needs-review`
- `stale`

## Ecosystem Knowledge Location

Store all language, standard, library, Docker, toolchain, OS, config-format, and module-layout facts in:
- `01-project/PROJECT_ECOSYSTEM.md`

If the content grows, split it into:
- `01-project/BUILD_AND_RUN.md`
- `01-project/DEPENDENCIES.md`
- `01-project/REPO_LAYOUT.md`
