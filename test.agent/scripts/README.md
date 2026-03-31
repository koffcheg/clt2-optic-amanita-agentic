# test.agent/scripts

Universal stage scripts for AI-agent test runs.

## Scope

Rules in this directory apply only to test execution and test run orchestration.
They do not define policy for regular development tasks.

During test runs, changing production code in Amanita or Comparator is forbidden.
Allowed changes are limited to per-run artifacts (run directories, logs, reports, and run-specific configs).

Each new test must be independent and start from zero state.
Reusing run artifacts or staged configs from previous tests is forbidden.

## Scripts

- `run_amanita_stage.sh`
  - Thin stage orchestrator for Amanita (DP1 + DP2 together by default).
  - Main logic lives in `lib/amanita_stage_lib.sh` and shared helpers in `lib/stage_common.sh`.
  - Requires DP2 and DP1 configs/binaries and shared DP2 port.
  - Performs preflight checks for busy DP2 port and stale DP2 processes.
  - Waits for DP2 readiness before starting DP1.
  - Runs DP1 in blocking mode and only proceeds when DP1 exits.
  - Default timeout is disabled (`--timeout-sec 0`), so long runs are awaited to completion.
  - After DP1 completion, waits a short drain window so DP2 can flush in-flight results.
  - Performs cleanup and verifies DP2 port release after stage.
  - Validates minimal artifact adequacy for Amanita outputs.
  - Writes `FAILED.txt` into run root on any failure.

- `run_comparator_stage.sh`
  - Thin stage orchestrator for Comparator report stage.
  - Main logic lives in `lib/comparator_stage_lib.sh` and shared helpers in `lib/stage_common.sh`.
  - Runs Comparator in blocking mode and only exits after completion.
  - Default timeout is disabled (`--timeout-sec 0`), so long runs are awaited to completion.
  - Places comparator config in both canonical run artifacts and Comparator CLI location:
    - `<TestRun>/Configs/Comparator/<StageId>/config_comparator.json`
    - `<TestRun>/RunResults/Comparator/<StageId>/configs/<Profile>/config_comparator.json`
  - Validates expected report artifact.
  - Writes `FAILED.txt` into run root on any failure.

- `generate_summary.sh`
  - Automatic generation of `<TestId>_Summary.md` based on Amanita and Comparator artifacts.
  - Main logic lives in `lib/summary_lib.sh`.
  - Collects metrics from:
    - DP1: Object count (parsed from output JSON), execution time
    - DP2: Tracked trajectory count, execution time
    - Comparator: Comparison pairs, mean overlap percentage
  - Generates summary with all required fields as per `AI_AGENT_TESTING_WORKFLOW.md`.
  - Should be called after both Amanita and Comparator stages complete successfully.
  - Usage example:
    ```bash
    generate_summary.sh \
      --run-root /path/to/test/run \
      --test-id AMNT-0005_20260328_175905_01 \
      --stage-id stage01 \
      --dataset MyDataset \
      --description "Test with SWIR dataset"
    ```

## Notes

- Testing rules and run algorithm must be read from the knowledge base:
  - `project-knowledge/05-validation/VALIDATION_INDEX.md`
  - `project-knowledge/05-validation/AI_AGENT_TESTING_WORKFLOW.md`
- Run-specific config mutation scripts are not stored here.
- Per policy, run-specific config scripts should be generated into `<TestRun>/Temp`.
- Before Comparator stage, always check that a ready Comparator Python environment already exists and is valid.
- If Comparator Python environment is missing or invalid, stop test execution and ask user to approve/setup environment.
- For a new test, generate configs from source templates/code each time; do not reuse staged configs from previous tests.
- Required stage sequence is strict: config generation -> `run_amanita_stage.sh` -> validation -> `run_comparator_stage.sh` -> validation -> summary.
- Do not start the next stage until the current stage process has exited.
- Stage order must remain strict unless user explicitly allows otherwise.
- Stage logs are written under `<TestRun>/Logs/Amanita/<StageId>` and `<TestRun>/Logs/Comparator/<StageId>`.
- Amanita stage configs are copied under `<TestRun>/Configs/Amanita/DP1/<StageId>` and `<TestRun>/Configs/Amanita/DP2/<StageId>`.
- Comparator stage configs are copied under `<TestRun>/Configs/Comparator/<StageId>`.
- For multi-run scenarios each stage must have its own `<StageId>` subdirectory in `Configs/Amanita/*`, `Configs/Comparator/*`, and `Logs/*`.
- Summary is auto-generated with the default structure; agree fields with the user only when a custom Summary is required.
- On failure, stop, record reason in run artifacts, and wait for user instructions.
- For unknown non-zero exit codes, use stop-and-wait by default.
- Do not perform autonomous recovery actions after a failed stage unless user explicitly approves.
- For long orchestration commands, prefer generating a per-run script in `<TestRun>/Temp` and execute it as a single command (`bash <script>`).
- Avoid pasting very long multi-line commands directly into terminal sessions, because partial interactive rendering can hide real failure points.
