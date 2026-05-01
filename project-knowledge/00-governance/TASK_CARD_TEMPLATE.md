# TASK_CARD_TEMPLATE.md

<!-- AGENT INSTRUCTIONS: Before creating a new task card:
  1. Open project-knowledge/06-tasks/TASKS_INDEX.md
  2. Find the highest existing AMNT-XXXX number
  3. Increment it by 1 to get the new id
  4. Use that id in the filename and in the `id` field below
  Never use placeholder ids like AMNT-0000.
-->

---
id: AMNT-XXXX
title: Short task title
status: draft
type: task-card
priority: normal
created_by: agent
created_at: YYYY-MM-DD
updated_at: YYYY-MM-DD
related_user_request: Short description of the user request
scope: local | medium | broad
domain_area:
  - dp1
  - dp2
  - protocols
  - ecosystem
related_files: []
related_cards: []
approval_status: pending
task_mode: canonical_development | legacy_analysis | migration | validation | governance
canonical_sources: []
legacy_access: forbidden | allowed
allowed_legacy_sources: []
forbidden_sources: []
source_of_truth_cards: []
stage_specs_required: yes | no
configuration_required: yes | no
validation_route:
---

## Summary

Brief human-readable description of the task.

## Goal

Expected result.

## In scope

What is included in the task.

## Out of scope

What must not be changed without separate approval.

## User constraints

Explicit user constraints.

Examples:
- do not change Project Knowledge without approval;
- do not create tests without approval;
- do not perform broad refactoring;
- do not add dependencies.

## Source access rules

For `canonical_development`, legacy is forbidden by default.

If `legacy_access: allowed`, the task card must list exact `allowed_legacy_sources`.

Legacy materials are not target-architecture sources and must not be used for canonical code generation.

## Assumptions

Starting assumptions. Mark unverified facts explicitly.

## Relevant context to read

Files that must be read before execution.

Examples:
- `project-knowledge/PROJECT_INDEX.md`
- `project-knowledge/01-project/PROJECT_ECOSYSTEM.md`
- `project-knowledge/04-protocols/PROTOCOLS_INDEX.md`
- `datapro1/src/...`

## Planned changes

Short list of expected changes.

## Files expected to be touched

Files likely to be changed.

## Risks

Risks such as contract changes, data-format changes, build/run impact, side effects, or incomplete knowledge context.

## Validation approach

How the result will be checked.

Examples:
- review by code reading;
- local build;
- manual scenario;
- test plan only;
- no validation yet.

## Knowledge-base impact

Possible values:
- none
- maybe
- yes

If `maybe` or `yes`, list affected cards or indexes and whether separate approval is required.

## Execution notes

Short execution notes. Do not turn the card into a large log.

## Result

What was actually done.

## Follow-up

Future work or items requiring separate approval.
