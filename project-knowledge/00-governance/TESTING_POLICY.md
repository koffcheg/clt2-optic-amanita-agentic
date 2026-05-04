# TESTING_POLICY.md

## Purpose

This file is the governance source for project testing rules and AI-agent limits.

It defines:
- the target testing taxonomy;
- the target test-directory model;
- what agents may and may not create or change without approval;
- where to read detailed guidance for each testing type.

## Status

Status: active.

Detailed testing documents:
- `project-knowledge/05-validation/UNIT_TESTING_GUIDE.md` - unit and visual-unit testing guide.
- `project-knowledge/05-validation/AMANITA_COMPARATOR_E2E_VALIDATION_WORKFLOW.md` - Amanita + Comparator system-e2e run-based validation workflow.
- `project-knowledge/05-validation/VALIDATION_INDEX.md` - validation knowledge entry point and validation-card index.

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

## Target Test Types

The project uses these target test types:

- `unit` - isolated automated checks for small units of logic.
- `visual-unit` - a unit-test subtype for computer-vision behavior using small controlled visual inputs and expected values, properties, or invariants.
- `integration` - checks for interaction between several target components or boundaries.
- `accuracy-regression` - checks that fixed inputs produce expected outputs, ground-truth alignment, or stable algorithmic metrics.
- `performance` - checks for runtime, throughput, memory behavior, profiling, or complexity budgets.
- `system-e2e` - full pipeline validation across major runtime components and environment assumptions.
- `manual-run-based validation` - validation performed through run directories, configs, logs, artifacts, reports, and explicit human/agent interpretation rather than automated test files.

Amanita + Comparator validation is classified as:
- primary type: `system-e2e`;
- execution form: `manual-run-based validation`;
- optional evidence type: `accuracy-regression`;
- optional evidence type: `performance`.

## Target Test Directory Model

The target repository test layout is:

```text
tests/
  unit/
    dp1/
    dp2/
    common/
  visual-unit/
    dp1/
    dp2/
  integration/
    dp1/
    dp2/
    dp1_dp2/
  accuracy-regression/
    dp1/
    dp2/
    dp1_dp2/
  performance/
    dp1/
    dp2/
    pipeline/
  system-e2e/
    amanita-comparator/
  fixtures/
    images/
    configs/
    expected/
```

This policy defines the target model only. Creating this structure or adding files to it requires explicit approval.

## Unit And Visual-Unit Rules

Unit and visual-unit tests must follow `05-validation/UNIT_TESTING_GUIDE.md`.

Default rules:
- prefer deterministic synthetic in-memory data;
- keep tests small and independent;
- use explicit assertions;
- test project behavior, not third-party library behavior;
- classify file-based datasets, full pipeline runs, deployment, or long-running scenarios as non-unit test types.

GoogleTest is the target C++ unit-test framework. CTest is the target CMake-level test runner when tests are integrated into the build.

## Amanita + Comparator Run-Based Validation

Amanita + Comparator validation is governed by:
- `project-knowledge/05-validation/AMANITA_COMPARATOR_E2E_VALIDATION_WORKFLOW.md`

Classification:
- primary type: `system-e2e`;
- execution form: `manual-run-based validation`;
- optional evidence type: `accuracy-regression`;
- optional evidence type: `performance`.

Default Amanita run rule:
- run DP1 + DP2 together by default;
- isolated DP1-only or DP2-only runs require explicit approval.

During Amanita + Comparator validation, agents must not change production Amanita or Comparator code. Only per-run configs, artifacts, logs, and reports may be changed.

The workflow's stop-on-failure and orchestration rules apply to test execution only. Normal development tasks may have separate policy agreed per task.

## When Testing Must Be Proposed Or Run

Testing is required to propose or run when changes affect:
- DP1/DP2 algorithms;
- data formats or serialization contracts;
- runtime/config behavior;
- build/run paths that may affect integration or system-e2e scenarios;
- logic already covered by a validation scenario.

For new or changed algorithmic code, the proposed validation coverage must include:
- normal scenarios;
- boundary scenarios;
- invalid or error-input scenarios;
- known regression risks, if any.

## When Agents Must Not Add Tests Automatically

Do not add tests automatically when:
- the task does not request it;
- the module testing strategy is undefined;
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

## Expected Result Changes

Agents must not change expected results only to make tests or validation pass.

If expected behavior changes, classify the change before editing expected outputs:
- bug fix - the previous expected result encoded incorrect behavior;
- contract change - accepted behavior changed and requires approval;
- regression - new behavior is unintended and must not be accepted as expected.

Expected-result changes require an explicit explanation in the task result or test plan.

## Fixtures, Mocks, Stubs, Datasets

- Do not create new fixtures, mocks, stubs, snapshots, golden files, or datasets without explicit approval.
- Store the dataset pool for Amanita + Comparator validation under `${AMANITA_RESOURCES_DIR}/datasets`.
- Keep large binary artifacts in the resource directory, not in git history.

## Performance

Performance checks are not mandatory by default.

Run or require them only when:
- the user asks;
- the task concerns performance regression;
- the stage specification or validation card defines a complexity or runtime budget.

Performance results must be interpreted separately from unit-test pass/fail.

## Validation Without Automated Tests

When automated tests are not added, acceptable alternatives are:
- local build;
- run-based manual scenario;
- output artifact and log review;
- expected/actual comparison through an approved validation workflow;
- result documentation in the approved report format.

## Test Infrastructure Limits

Without explicit approval, agents must not:
- add new test frameworks;
- add mocking libraries;
- change test directory structure;
- add CI jobs only for tests;
- create large test artifacts in the repository.

## Open Questions

- What is the first approved target module for new unit tests?
- What fixture size limits should be used for `accuracy-regression` tests?
- Which performance thresholds are mandatory for canonical DP1/DP2 stages?

## Definition Of Done For This Document

This document is operational when it defines:
- test types used by the project;
- target test-directory model;
- what agents may and may not do without approval;
- when a test plan is required;
- acceptable manual or alternative checks;
- routes to detailed testing and validation documents.
