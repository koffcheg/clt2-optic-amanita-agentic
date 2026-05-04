# AMANITA_COMPARATOR_E2E_VALIDATION_WORKFLOW

## Purpose

This document defines the standard Amanita + Comparator end-to-end run-based validation workflow.

It is the canonical source for Amanita + Comparator execution order, run-directory structure, stop-on-failure behavior, and report generation.

Testing type mapping:
- primary type: `system-e2e`;
- execution form: `manual-run-based validation`;
- may provide evidence for `accuracy-regression` when Comparator metrics are evaluated against fixed data;
- may provide evidence for `performance` when DP1, DP2, and Comparator timing is interpreted.

Scope:
- applies only to test execution and test-run orchestration;
- is not a universal policy for normal development tasks.

## Governance Boundary

General testing permissions, approval rules, and target test taxonomy are defined in:
- `project-knowledge/00-governance/TESTING_POLICY.md`

This workflow owns Amanita + Comparator operational details:
- run location and required directory structure;
- staged config and artifact layout;
- script sequence;
- Comparator environment precheck;
- Summary generation;
- stop-on-failure behavior;
- failure reporting and recovery limits.

Production-code immutability during test runs:
- do not change production Amanita or Comparator code during execution test runs;
- only per-run configs/scripts in the run directory, binary/CLI execution, and artifact/report collection are allowed.

Default Amanita run rule:
- test DP1 and DP2 together in one stage by default;
- testing only one module, DP1-only or DP2-only, requires explicit approval.

## 1. Test Location

Root test directory:
- `${AMANITA_RESOURCES_DIR}/tests`

Each run must have a separate directory:
- `${AMANITA_RESOURCES_DIR}/tests/<TestId>`

Each new `TestId` must start from a clean state. Do not reuse staged configs or previous run results. Prepare configs from scratch with a per-run script under `Temp/`.

`TestId` format:
- `<task_id>_YYYYMMDD_HHMMSS_<run_number>`
- example: `AMNT-0005_20260325_213500_01`

## 2. Required Run Structure

Each run must include:
- `Configs/` - Amanita and Comparator configs for this run;
- `RunResults/` - execution artifacts;
- `Logs/` - stdout/stderr, timing, and checks for each step;
- `Temp/` - temporary files and per-run config-modification scripts.

Required config subdirectories:
- `Configs/Amanita/DP1/<StageId>/...`
- `Configs/Amanita/DP2/<StageId>/...`
- `Configs/Comparator/<StageId>/...`

For multi-run scenarios, each run has its own `<StageId>` and subdirectories under `Configs/*` and `Logs/*`.

Expected minimum under `RunResults/`:
- `RunResults/Amanita/DP1`
- `RunResults/Amanita/DP2`
- `RunResults/Comparator`

The run root must contain reports. Required minimum:
- `<TestId>_Summary.md`, generated from Amanita and Comparator artifacts.

Failures must be recorded in the run root:
- `FAILED.txt`

Default `<TestId>_Summary.md` data:
- `TestId`;
- test date;
- dataset;
- short test description;
- DP1 total detected objects across frames, parsed from DP1 JSON;
- DP2 tracked trajectories;
- execution time;
- Comparator report data, including report count and mean overlap;
- short result summary.

A custom Summary field set may be agreed before the run. Custom generation must use per-run scripts stored only under that run's `Temp/` directory.

## 3. Execution Order

Steps run strictly in sequence unless the user explicitly states otherwise:

1. Check the prepared Comparator Python environment.
2. Rebuild Amanita through `builder/build_dp1_dp2.sh`, default `Debug`, then `Release` if agreed.
3. Prepare/modify Amanita and Comparator configs.
4. Run Amanita, DP1 + DP2 as one stage.
5. Validate Amanita artifacts.
6. Run Comparator.
7. Validate Comparator artifacts.
8. Generate `<TestId>_Summary.md` with `test.agent/scripts/generate_summary.sh`.
9. Analyze results from the generated summary.

Amanita stage completion rule:
- for DP1 file-based sources (`imagefile`, `videofile`), EOF is successful completion with exit code `0`;
- non-zero Amanita exit code is a failure and triggers stop-on-failure.

Comparator environment precheck:
- before any Comparator stage, verify that a working Comparator Python environment exists;
- if it is missing or invalid, stop the run;
- do not auto-configure the environment without user approval.

Use only repository stage scripts for execution:
- `test.agent/scripts/run_amanita_stage.sh`
- `test.agent/scripts/run_comparator_stage.sh`

Start the next step only after the previous step exits.

The result of step 9 must be:
- shown in chat;
- appended to the end of `<TestId>_Summary.md` under `Result summary`.

Required step 9 interpretation:
- overall `PASS`/`FAIL` and short conclusion;
- stage stability or stage differences for multi-run;
- Comparator metrics: `comparator_mean_overlap_pct`, `comparator_mean_rms_deviation_area`, `comparator_mean_false_positives_pct`, `comparator_mean_false_negatives_pct`;
- performance interpretation using DP1/DP2/Comparator times;
- explicit risks and next step.

For smoke/synthetic multi-run, state explicitly that metrics do not replace a real candidate benchmark.

Report language:
- one Summary must use one language consistently;
- default Summary language is Ukrainian unless the user asks otherwise.

## 4. Stop-On-Failure Policy

If any step fails:
- stop immediately;
- record the reason in the run directory, for example `FAILED.txt`;
- do not run subsequent steps;
- wait for explicit user instructions.

Unknown non-zero exit codes use stop-and-wait by default.

After failure, do not perform autonomous recovery, rerun the pipeline, or change configs to bypass the failure without explicit user approval.

## 5. Script Policy

Universal run scripts live in the repository:
- `test.agent/scripts/run_amanita_stage.sh`
- `test.agent/scripts/run_comparator_stage.sh`
- `test.agent/scripts/generate_summary.sh`

Config-modification scripts must be per-run scripts stored under that run's `Temp/` directory.

### `generate_summary.sh`

Generate Summary with:

```bash
test.agent/scripts/generate_summary.sh   --run-root <path>   --test-id <id>   [--stage-id <id>]   [--dataset <name>]   [--description <text>]   [--test-date "YYYY-MM-DD HH:MM:SS"]
```

Parameters:
- `--run-root`: test run root directory;
- `--test-id`: test ID in `AMNT-XXXX_YYYYMMDD_HHMMSS_NN` format;
- `--stage-id`: optional stage ID, auto-detected from `Logs/*` when omitted;
- `--dataset`: optional dataset name, auto-detected from Comparator report when possible;
- `--description`: optional short test description;
- `--test-date`: optional test date/time, defaults to current date/time.

For custom Summary fields:
- use separate per-run generation/post-processing scripts;
- store them under the current run's `Temp/` directory;
- do not commit them as universal repository scripts.

Multi-run behavior without explicit `--stage-id`:
- collect all stages from `Logs/Amanita/<StageId>` and `Logs/Comparator/<StageId>`;
- generate one `<TestId>_Summary.md` with a multi-run summary and a `Stage <StageId>` section for each run.

Required output:
- `<TestId>_Summary.md` at `<run-root>` with `TestId`, date, dataset, description, DP1 metrics, DP2 metrics, Comparator aggregates, per-report detail, and conclusion.
