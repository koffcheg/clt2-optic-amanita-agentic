# VALIDATION_INDEX

## Purpose

This index is the entry point for testing, validation, and regression-scenario knowledge.

This section also records practical validation scenarios for DP1/DP2 changes when automated tests are not created inside the task.

For AI-agent test execution, the canonical workflow is:
- `AI_AGENT_TESTING_WORKFLOW.md`

## Section Documents

- `AI_AGENT_TESTING_WORKFLOW.md` - mandatory Amanita + Comparator test-run workflow for AI agents.

## Current Cards

- `validation.dp1.canonical_conformance` - canonical DP1 validation route: data domains, stage interfaces, stage specs before code generation, configuration `C`, Measurement handoff, profiling, and complexity levels.
  link: `cards/validation.dp1.canonical_conformance.md`

- `validation.amnt0004.dp1_binning` - validation card for legacy DP1 sum-binning feature.
  link: `cards/validation.amnt0004.dp1_binning.md`

- `validation.dp1.radiometric_correction.inverse_median` - validation route for
  the `inverse_median` variant of canonical DP1 `radiometric_correction`.
  link: `cards/validation.dp1.radiometric_correction.inverse_median.md`

## Related Documents

- `project-knowledge/00-governance/TESTING_POLICY.md` - governance rules for testing and agent action limits.
- `project-knowledge/06-tasks/TASKS_INDEX.md` - task cards with context for specific validation work.

## Related Tools

- `test.agent/scripts/run_amanita_stage.sh` - run one Amanita stage.
- `test.agent/scripts/run_comparator_stage.sh` - run one Comparator stage.
- `test.agent/scripts/generate_summary.sh` - generate `<TestId>_Summary.md`.
