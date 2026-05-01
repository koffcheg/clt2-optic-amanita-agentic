# TESTING_POLICY.md

## Purpose

This file defines project testing rules and AI-agent limits for creating, changing, and running tests.

## Status

Status: active.

The canonical AI-agent run workflow is:
- `project-knowledge/05-validation/AI_AGENT_TESTING_WORKFLOW.md`

The stop-on-failure and orchestration rules in that workflow apply to test execution only. Normal development tasks may have separate policy agreed per task.

## Base Agent Rule

Without explicit approval, agents must not:
- create new automated tests;
- create fixtures, mocks, stubs, snapshots, or golden files;
- add or change test infrastructure;
- mass-rewrite existing tests;
- change the project testing strategy.

If an agent believes testing is needed, it must:
- describe the risk;
- explain why a test would help;
- propose a test plan;
- wait for approval.

## Testing Role In The Project

Testing is hybrid:
- C++ modules use build + targeted validation scenarios;
- DP1/DP2 integration uses run-based artifact validation;
- AI-agent test runs use a structured stop-on-failure pipeline.

Default Amanita run rule:
- run DP1 + DP2 together by default;
- isolated DP1-only or DP2-only runs require explicit approval.

## Test Categories

Current categories:
- unit/integration tests through existing `gtests/`;
- run-based validation for DP1/DP2;
- dataset-based validation through Amanita + Comparator;
- manual verification where automation is not approved.

## When Testing Must Be Proposed Or Run

Testing is required to propose or run when changes affect:
- DP1/DP2 algorithms;
- data formats or serialization contracts;
- runtime/config behavior;
- build/run paths that may affect integration scenarios;
- logic already covered by a validation scenario.

## When Agents Must Not Add Tests Automatically

Do not add tests automatically when:
- the task does not request it;
- module testing strategy is undefined;
- correct testing requires new infrastructure;
- large fixtures, datasets, or mocks are needed;
- expected check format is not agreed.

## Test Plan Instead Of Test Creation

When an agent cannot create a test, propose a test plan covering:
- what to verify;
- suitable verification type;
- required input data;
- expected result;
- covered risks;
- manual checks;
- future automation candidates.

## Existing Tests

Do not mass-change existing tests without explicit approval. Local edits to existing tests are allowed only when they are part of the approved task.

## Fixtures, Mocks, Stubs, Datasets

- Do not create new fixtures/mocks/stubs without explicit approval.
- Store the dataset pool for AI-agent validation under `${AMANITA_RESOURCES_DIR}/datasets`.
- Keep large binary artifacts in the resource directory, not in git history.

## Manual Verification

Manual verification is acceptable when automation is unavailable or not approved, and the result is backed by run-directory artifacts.

Minimum AI-agent run reporting:
- run structure follows `AI_AGENT_TESTING_WORKFLOW.md`;
- step logs are under `Logs/Amanita/<StageId>` and `Logs/Comparator/<StageId>`;
- per-stage configs are under `Configs/Amanita/.../<StageId>` and `Configs/Comparator/<StageId>`;
- `<TestId>_Summary.md` is generated at run root by `test.agent/scripts/generate_summary.sh`;
- failures are recorded in `FAILED.txt`.

Generate the summary after both stages complete successfully:

```bash
test.agent/scripts/generate_summary.sh   --run-root <path>   --test-id <id>   [--stage-id <id>]   [--dataset <name>]   [--description <text>]   [--test-date "YYYY-MM-DD HH:MM:SS"]
```

For multi-run:
- without `--stage-id`, generate one aggregated report across all stages;
- with `--stage-id`, generate the summary only for that stage.

## Performance And Benchmarks

Performance/benchmark checks are not mandatory by default. Run them only when the user asks or the task is about performance regression.

## Running Tests

- Use integrated project test flows for CMake tests.
- Use `05-validation/AI_AGENT_TESTING_WORKFLOW.md` for Amanita + Comparator AI-agent testing.
- Before Comparator stage, verify the prepared Comparator Python environment.
- If the Comparator Python environment is missing or invalid, stop and ask the user to configure it.
- During test-run execution, do not change production Amanita/Comparator code. Only per-run configs, artifacts, logs, and reports may be changed.
- Each new test run must start from a clean state. Do not reuse staged configs or artifacts from previous tests.
- Use strictly `test.agent/scripts/run_amanita_stage.sh` -> validate -> `test.agent/scripts/run_comparator_stage.sh` -> validate.
- Amanita stage is complete only when DP1 + DP2 run together, unless explicitly agreed otherwise.
- For DP1 file-based sources (`imagefile`, `videofile`), EOF is a successful stage completion with exit code `0`.
- Pipeline steps run strictly in sequence unless explicitly stated otherwise.
- Move to the next step only after the current process exits.
- Report step 9 analysis in chat and append it to `<TestId>_Summary.md` under `Result summary`.
- A single Summary must use one language consistently. Default report language is Ukrainian unless the user asks otherwise.
- Stop on the first failure.
- Unknown non-zero exit codes use stop-and-wait by default.
- After failure, autonomous recovery is forbidden without explicit user instruction.

## Validation Without Tests

When automated tests are not added, acceptable alternatives are:
- local build;
- run-based manual scenario;
- output artifact and log review;
- expected/actual comparison through Comparator;
- result documentation in `<TestId>_Summary.md`.

## Test Infrastructure Limits

Without explicit approval, agents must not:
- add new test frameworks;
- add mocking libraries;
- change test directory structure;
- add CI jobs only for tests;
- create large test artifacts in the repository.

## Open Questions

- Are unit tests required for core C++ logic, or are integration/manual checks enough?
- Are mocks allowed in this project?
- What is the minimum test plan for a bug fix?

## Definition Of Done For This Document

This document is operational when it defines:
- test types used by the project;
- what agents may and may not do without approval;
- when a test plan is required;
- acceptable manual or alternative checks;
- rules for running tests and changing existing tests.
