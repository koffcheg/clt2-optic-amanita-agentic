# VALIDATION_INDEX

## Purpose

This index is the entry point for testing, validation, and regression-scenario knowledge.

This section records the target testing taxonomy, validation routes, and practical validation scenarios for DP1/DP2 changes when automated tests are not created inside the task.

## Section Documents

- `UNIT_TESTING_GUIDE.md` - target guide for `unit` and `visual-unit` testing.
- `AMANITA_COMPARATOR_E2E_VALIDATION_WORKFLOW.md` - mandatory Amanita + Comparator `system-e2e` run-based validation workflow.

## Testing Type Routes

- `unit` - read `UNIT_TESTING_GUIDE.md`.
- `visual-unit` - read `UNIT_TESTING_GUIDE.md`.
- `integration` - future target checks for component boundaries and interactions.
- `accuracy-regression` - fixed input and expected/ground-truth comparison; may use Amanita + Comparator evidence when the workflow is approved for the task.
- `performance` - runtime, throughput, memory, profiling, and complexity-budget checks.
- `system-e2e` - read `AMANITA_COMPARATOR_E2E_VALIDATION_WORKFLOW.md` for Amanita + Comparator.
- `manual-run-based validation` - read `AMANITA_COMPARATOR_E2E_VALIDATION_WORKFLOW.md` when the run uses Amanita + Comparator.

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

This is a target model only. Creating directories, tests, or fixtures requires approval under `project-knowledge/00-governance/TESTING_POLICY.md`.

## Current Cards

### Canonical Conformance

- `validation.dp1.canonical_conformance` - canonical DP1 validation route: data domains, stage interfaces, stage specs before code generation, configuration `C`, Measurement handoff, profiling, and complexity levels.
  link: `cards/validation.dp1.canonical_conformance.md`

### Stage-Level Algorithm Validation

- `validation.dp1.radiometric_correction.inverse_median` - validation route for
  the `inverse_median` variant of canonical DP1 `radiometric_correction`.
  link: `cards/validation.dp1.radiometric_correction.inverse_median.md`

### Task-Specific Run-Based Validation

- `validation.amnt0004.dp1_binning` - validation card for DP1 sum-binning run-based validation.
  link: `cards/validation.amnt0004.dp1_binning.md`

## Related Documents

- `project-knowledge/00-governance/TESTING_POLICY.md` - governance rules for testing and agent action limits.
- `project-knowledge/06-tasks/TASKS_INDEX.md` - task cards with context for specific validation work.

## Related Tools

- `test.agent/scripts/run_amanita_stage.sh` - run one Amanita stage.
- `test.agent/scripts/run_comparator_stage.sh` - run one Comparator stage.
- `test.agent/scripts/generate_summary.sh` - generate `<TestId>_Summary.md`.
