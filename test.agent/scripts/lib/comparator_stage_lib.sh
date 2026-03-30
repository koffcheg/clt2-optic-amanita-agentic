#!/usr/bin/env bash

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
source "$SCRIPT_DIR/lib/stage_common.sh"

comparator_usage() {
  cat <<'EOF'
Usage:
  run_comparator_stage.sh \
    --python-bin <path> \
    --cli-path <path> \
    --comparator-config <path> \
    --run-root <path> \
    --test-id <id> \
    --stage-id <id> \
    --profile <name> \
    [--timeout-sec <n>]

Notes:
  --timeout-sec 0 means "wait until process exits" (default).
EOF
}

comparator_init_defaults() {
  PYTHON_BIN=""
  CLI_PATH=""
  CMP_CFG=""
  RUN_ROOT=""
  TEST_ID=""
  STAGE_ID=""
  PROFILE=""
  TIMEOUT_SEC="0"
}

comparator_parse_args() {
  while [[ $# -gt 0 ]]; do
    case "$1" in
      --python-bin) PYTHON_BIN="$2"; shift 2 ;;
      --cli-path) CLI_PATH="$2"; shift 2 ;;
      --comparator-config) CMP_CFG="$2"; shift 2 ;;
      --run-root) RUN_ROOT="$2"; shift 2 ;;
      --test-id) TEST_ID="$2"; shift 2 ;;
      --stage-id) STAGE_ID="$2"; shift 2 ;;
      --profile) PROFILE="$2"; shift 2 ;;
      --timeout-sec) TIMEOUT_SEC="$2"; shift 2 ;;
      -h|--help) comparator_usage; exit 0 ;;
      *) echo "Unknown argument: $1" >&2; comparator_usage; exit 2 ;;
    esac
  done
}

comparator_validate_args() {
  assert_executable "$PYTHON_BIN" "python bin"
  assert_file "$CLI_PATH" "cli path"
  assert_file "$CMP_CFG" "comparator config"
  assert_non_empty "$RUN_ROOT" "run root"
  assert_non_empty "$TEST_ID" "test id"
  assert_non_empty "$STAGE_ID" "stage id"
  assert_non_empty "$PROFILE" "profile"
}

comparator_prepare_layout() {
  CFG_ROOT="$RUN_ROOT/Configs/Comparator/$STAGE_ID"
  CMP_STAGE_DIR="$RUN_ROOT/RunResults/Comparator/$STAGE_ID"
  CMP_PROFILE_CFG_DIR="$CMP_STAGE_DIR/configs/$PROFILE"
  LOG_DIR="$RUN_ROOT/Logs/Comparator/$STAGE_ID"
  TMP_DIR="$RUN_ROOT/Temp"

  rm -rf "$CFG_ROOT" "$CMP_STAGE_DIR" "$LOG_DIR"
  mkdir -p "$CFG_ROOT" "$CMP_STAGE_DIR" "$CMP_PROFILE_CFG_DIR" "$LOG_DIR" "$TMP_DIR"
  copy_if_needed "$CMP_CFG" "$CFG_ROOT/config_comparator.json"
  copy_if_needed "$CMP_CFG" "$CMP_PROFILE_CFG_DIR/config_comparator.json"

  CMP_STDOUT="$LOG_DIR/stdout.log"
  CMP_TIME="$LOG_DIR/time.txt"
  CMP_RC="$LOG_DIR/rc.txt"
  CMP_CHECKS="$LOG_DIR/checks.txt"
}

comparator_run_blocking() {
  local rc
  if [[ "$TIMEOUT_SEC" == "0" ]]; then
    set +e
    /usr/bin/time -f 'real_sec=%e\nuser_sec=%U\nsys_sec=%S\nmax_rss_kb=%M' -o "$CMP_TIME" \
      "$PYTHON_BIN" "$CLI_PATH" report --run-dir "$CMP_STAGE_DIR" --profile "$PROFILE" > "$CMP_STDOUT" 2>&1
    rc=$?
    set -e
  else
    set +e
    /usr/bin/time -f 'real_sec=%e\nuser_sec=%U\nsys_sec=%S\nmax_rss_kb=%M' -o "$CMP_TIME" \
      timeout "${TIMEOUT_SEC}s" "$PYTHON_BIN" "$CLI_PATH" report --run-dir "$CMP_STAGE_DIR" --profile "$PROFILE" > "$CMP_STDOUT" 2>&1
    rc=$?
    set -e
  fi

  echo "$rc" > "$CMP_RC"
  [[ "$rc" -eq 0 ]] || stage_fail "$RUN_ROOT" "$TEST_ID" "$STAGE_ID" "comparator" "Comparator exited with code $rc"
  RC="$rc"
}

comparator_validate_artifacts() {
  REPORT_DIR="$CMP_STAGE_DIR/report"
  [[ -d "$REPORT_DIR" ]] || stage_fail "$RUN_ROOT" "$TEST_ID" "$STAGE_ID" "comparator" "Comparator report directory is missing: $REPORT_DIR"

  REPORT_COUNT=$(find "$REPORT_DIR" -maxdepth 1 -type f -name '*.json' | wc -l)
  [[ "$REPORT_COUNT" -gt 0 ]] || stage_fail "$RUN_ROOT" "$TEST_ID" "$STAGE_ID" "comparator" "Comparator produced no report json files in: $REPORT_DIR"

  REPORT_JSON=$(find "$REPORT_DIR" -maxdepth 1 -type f -name '*.json' | sort | head -n 1)

  if grep -q 'Traceback\|KeyError\|AttributeError\|Aborted!\|Error processing pair' "$CMP_STDOUT"; then
    stage_fail "$RUN_ROOT" "$TEST_ID" "$STAGE_ID" "comparator" "Comparator fatal error markers found in log"
  fi
}

comparator_write_checks() {
  cat > "$CMP_CHECKS" <<EOF
scenario=$STAGE_ID
test_id=$TEST_ID
rc=$RC
report_json_count=$REPORT_COUNT
report_path=$REPORT_JSON
timeout_sec=$TIMEOUT_SEC
EOF
}