#!/usr/bin/env bash

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
source "$SCRIPT_DIR/lib/stage_common.sh"

amanita_usage() {
  cat <<'EOF'
Usage:
  run_amanita_stage.sh \
    --dp2-bin <path> \
    --dp2-log-config <path> \
    --dp2-config <path> \
    --dp2-port <port> \
    --dp1-bin <path> \
    --dp1-log-config <path> \
    --dp1-config <path> \
    --run-root <path> \
    --test-id <id> \
    --stage-id <id> \
    [--expected-marker <text>] \
    [--forbid-marker <text>] \
    [--timeout-sec <n>] \
    [--ready-timeout-sec <n>] \
    [--drain-timeout-sec <n>]

Notes:
  --timeout-sec 0 means "wait until process exits" (default).
EOF
}

amanita_init_defaults() {
  DP1_BIN=""
  DP1_LOG_CFG=""
  DP1_CFG=""
  DP2_BIN=""
  DP2_LOG_CFG=""
  DP2_CFG=""
  DP2_PORT=""
  RUN_ROOT=""
  TEST_ID=""
  STAGE_ID=""
  EXPECTED_MARKER=""
  FORBID_MARKER=""
  TIMEOUT_SEC="0"
  READY_TIMEOUT_SEC="20"
  DP2_DRAIN_TIMEOUT_SEC="6"
  DP2_PID=""
}

amanita_parse_args() {
  while [[ $# -gt 0 ]]; do
    case "$1" in
      --dp2-bin) DP2_BIN="$2"; shift 2 ;;
      --dp2-log-config) DP2_LOG_CFG="$2"; shift 2 ;;
      --dp2-config) DP2_CFG="$2"; shift 2 ;;
      --dp2-port) DP2_PORT="$2"; shift 2 ;;
      --dp1-bin) DP1_BIN="$2"; shift 2 ;;
      --dp1-log-config) DP1_LOG_CFG="$2"; shift 2 ;;
      --dp1-config) DP1_CFG="$2"; shift 2 ;;
      --run-root) RUN_ROOT="$2"; shift 2 ;;
      --test-id) TEST_ID="$2"; shift 2 ;;
      --stage-id) STAGE_ID="$2"; shift 2 ;;
      --expected-marker) EXPECTED_MARKER="$2"; shift 2 ;;
      --forbid-marker) FORBID_MARKER="$2"; shift 2 ;;
      --timeout-sec) TIMEOUT_SEC="$2"; shift 2 ;;
      --ready-timeout-sec) READY_TIMEOUT_SEC="$2"; shift 2 ;;
      --drain-timeout-sec) DP2_DRAIN_TIMEOUT_SEC="$2"; shift 2 ;;
      -h|--help) amanita_usage; exit 0 ;;
      *) echo "Unknown argument: $1" >&2; amanita_usage; exit 2 ;;
    esac
  done
}

amanita_validate_args() {
  assert_executable "$DP2_BIN" "dp2 bin"
  assert_file "$DP2_LOG_CFG" "dp2 log config"
  assert_file "$DP2_CFG" "dp2 config"
  assert_non_empty "$DP2_PORT" "dp2 port"
  assert_executable "$DP1_BIN" "dp1 bin"
  assert_file "$DP1_LOG_CFG" "dp1 log config"
  assert_file "$DP1_CFG" "dp1 config"
  assert_non_empty "$RUN_ROOT" "run root"
  assert_non_empty "$TEST_ID" "test id"
  assert_non_empty "$STAGE_ID" "stage id"
}

amanita_prepare_layout() {
  CFG_ROOT="$RUN_ROOT/Configs/Amanita/DP1/$STAGE_ID"
  CFG_DP2_ROOT="$RUN_ROOT/Configs/Amanita/DP2/$STAGE_ID"
  AMN_DP1_ROOT="$RUN_ROOT/RunResults/Amanita/DP1"
  AMN_DP2_ROOT="$RUN_ROOT/RunResults/Amanita/DP2"
  LOG_DIR="$RUN_ROOT/Logs/Amanita/$STAGE_ID"
  TMP_DIR="$RUN_ROOT/Temp"

  # Every stage run starts from a clean stage-local state.
  rm -rf "$CFG_ROOT" "$CFG_DP2_ROOT" "$LOG_DIR"
  mkdir -p "$CFG_ROOT" "$CFG_DP2_ROOT" "$AMN_DP1_ROOT" "$AMN_DP2_ROOT" "$LOG_DIR" "$TMP_DIR"

  DP1_CFG_RUN="$CFG_ROOT/config_datapro1.json"
  DP1_LOG_CFG_RUN="$CFG_ROOT/dp1_log.xml"
  DP2_CFG_RUN="$CFG_DP2_ROOT/config_datapro2.json"
  DP2_LOG_CFG_RUN="$CFG_DP2_ROOT/dp2_log.xml"

  copy_if_needed "$DP1_CFG" "$DP1_CFG_RUN"
  copy_if_needed "$DP1_LOG_CFG" "$DP1_LOG_CFG_RUN"
  copy_if_needed "$DP2_CFG" "$DP2_CFG_RUN"
  copy_if_needed "$DP2_LOG_CFG" "$DP2_LOG_CFG_RUN"

  # Execute only staged per-run configs to avoid accidental cross-run coupling.
  DP1_CFG="$DP1_CFG_RUN"
  DP1_LOG_CFG="$DP1_LOG_CFG_RUN"
  DP2_CFG="$DP2_CFG_RUN"
  DP2_LOG_CFG="$DP2_LOG_CFG_RUN"

  DP2_STDOUT="$LOG_DIR/dp2_stdout.log"
  DP1_STDOUT="$LOG_DIR/stdout.log"
  DP1_TIME="$LOG_DIR/time.txt"
  DP1_RC="$LOG_DIR/rc.txt"
  DP1_CHECKS="$LOG_DIR/checks.txt"
  DP2_DRAIN_LOG="$LOG_DIR/dp2_drain.txt"
}

amanita_cleanup_dp2() {
  if [[ -n "$DP2_PID" ]] && kill -0 "$DP2_PID" >/dev/null 2>&1; then
    kill "$DP2_PID" >/dev/null 2>&1 || true
    wait "$DP2_PID" >/dev/null 2>&1 || true
  fi
}

amanita_preflight() {
  if ss -ltn | awk '{print $4}' | grep -Eq ":${DP2_PORT}$"; then
    stage_fail "$RUN_ROOT" "$TEST_ID" "$STAGE_ID" "amanita" "preflight failed: dp2 port ${DP2_PORT} is already in use"
  fi

  # Exclude current script process and its direct parent to avoid false positives
  # from command-line arguments containing --dp2-bin path.
  if pgrep -af -- "$DP2_BIN" | awk -v self="$$" -v parent="$PPID" '$1!=self && $1!=parent {found=1} END{exit found?0:1}'; then
    stage_fail "$RUN_ROOT" "$TEST_ID" "$STAGE_ID" "amanita" "preflight failed: running dp2 process detected ($DP2_BIN)"
  fi
}

amanita_validate_ports_in_configs() {
  python3 - "$DP2_CFG" "$DP1_CFG" "$DP2_PORT" <<'PY'
import json
import sys
from pathlib import Path

dp2_cfg = json.loads(Path(sys.argv[1]).read_text(encoding='utf-8'))
dp1_cfg = json.loads(Path(sys.argv[2]).read_text(encoding='utf-8'))
port = int(sys.argv[3])
dp2_port = int(dp2_cfg.get('server', {}).get('port', -1))
dp1_port = int(dp1_cfg.get('config', {}).get('dp2conn', {}).get('port', -1))
if dp2_port != port:
    print(f"dp2 config port mismatch: expected {port}, got {dp2_port}")
    sys.exit(2)
if dp1_port != port:
    print(f"dp1 config port mismatch: expected {port}, got {dp1_port}")
    sys.exit(2)
PY
}

amanita_validate_out_folder() {
  OUT_DIR=$(python3 - "$DP1_CFG" <<'PY'
import json
import sys
from pathlib import Path
cfg = json.loads(Path(sys.argv[1]).read_text(encoding='utf-8'))
print(cfg['config']['test']['out_folder'])
PY
)

  EXPECTED_OUT="$RUN_ROOT/RunResults/Amanita/DP1"
  [[ "$OUT_DIR" == "$EXPECTED_OUT" ]] || stage_fail "$RUN_ROOT" "$TEST_ID" "$STAGE_ID" "amanita" "dp1 out_folder must be '$EXPECTED_OUT', got '$OUT_DIR'"

  read -r DP2_IN_PATH DP2_OUT_PATH <<< "$(python3 - "$DP2_CFG" <<'PY'
import json
import sys
from pathlib import Path
cfg = json.loads(Path(sys.argv[1]).read_text(encoding='utf-8'))
strobe = cfg.get('strobe-method-par', {})
print(strobe.get('dp1_out_folder', ''), strobe.get('dp2_out_folder', ''))
PY
)"

  [[ "$DP2_IN_PATH" == "$EXPECTED_OUT" ]] || stage_fail "$RUN_ROOT" "$TEST_ID" "$STAGE_ID" "amanita" "dp2 dp1_out_folder must be '$EXPECTED_OUT', got '$DP2_IN_PATH'"
  [[ "$DP2_OUT_PATH" == "$RUN_ROOT/RunResults/Amanita/DP2" ]] || stage_fail "$RUN_ROOT" "$TEST_ID" "$STAGE_ID" "amanita" "dp2 dp2_out_folder must be '$RUN_ROOT/RunResults/Amanita/DP2', got '$DP2_OUT_PATH'"
}

amanita_start_dp2() {
  # Clean stage outputs to ensure independent run from zero state.
  rm -rf "$AMN_DP1_ROOT/data_bin" "${AMN_DP1_ROOT}frame_input" "$AMN_DP2_ROOT"
  mkdir -p "$AMN_DP2_ROOT"
  rm -f "$DP2_STDOUT"

  "$DP2_BIN" "$DP2_CFG" "$DP2_LOG_CFG" > "$DP2_STDOUT" 2>&1 &
  DP2_PID=$!
}

amanita_wait_dp2_ready() {
  local ready=0
  for _ in $(seq 1 "$READY_TIMEOUT_SEC"); do
    if ! kill -0 "$DP2_PID" >/dev/null 2>&1; then
      stage_fail "$RUN_ROOT" "$TEST_ID" "$STAGE_ID" "amanita" "dp2 terminated before readiness"
    fi
    if ss -ltn | awk '{print $4}' | grep -Eq ":${DP2_PORT}$"; then
      ready=1
      break
    fi
    sleep 1
  done
  [[ "$ready" -eq 1 ]] || stage_fail "$RUN_ROOT" "$TEST_ID" "$STAGE_ID" "amanita" "dp2 readiness timeout on port ${DP2_PORT}"
}

amanita_run_dp1_blocking() {
  local rc
  if [[ "$TIMEOUT_SEC" == "0" ]]; then
    set +e
    /usr/bin/time -f 'real_sec=%e\nuser_sec=%U\nsys_sec=%S\nmax_rss_kb=%M' -o "$DP1_TIME" \
      "$DP1_BIN" 1 "$DP1_CFG" "$DP1_LOG_CFG" > "$DP1_STDOUT" 2>&1
    rc=$?
    set -e
  else
    set +e
    /usr/bin/time -f 'real_sec=%e\nuser_sec=%U\nsys_sec=%S\nmax_rss_kb=%M' -o "$DP1_TIME" \
      timeout "${TIMEOUT_SEC}s" "$DP1_BIN" 1 "$DP1_CFG" "$DP1_LOG_CFG" > "$DP1_STDOUT" 2>&1
    rc=$?
    set -e
  fi

  echo "$rc" > "$DP1_RC"
  if [[ "$rc" -ne 0 ]]; then
    stage_fail "$RUN_ROOT" "$TEST_ID" "$STAGE_ID" "amanita" "Amanita exited with code $rc"
  fi
  RC="$rc"
}

amanita_wait_dp2_drain() {
  DP2_OUT_DIR=$(python3 - "$DP2_CFG" <<'PY'
import json
import sys
from pathlib import Path
cfg = json.loads(Path(sys.argv[1]).read_text(encoding='utf-8'))
print(cfg.get('strobe-method-par', {}).get('dp2_out_folder', ''))
PY
)

  if [[ -n "$DP2_OUT_DIR" ]]; then
    local prev_cnt=-1
    local stable_hits=0
    : > "$DP2_DRAIN_LOG"
    for _ in $(seq 1 "$DP2_DRAIN_TIMEOUT_SEC"); do
      local cur_cnt
      cur_cnt=$(find "$DP2_OUT_DIR" -maxdepth 1 -type f -name '*.json' 2>/dev/null | wc -l)
      echo "dp2_json_count=$cur_cnt" >> "$DP2_DRAIN_LOG"
      if [[ "$cur_cnt" -eq "$prev_cnt" ]]; then
        stable_hits=$((stable_hits+1))
      else
        stable_hits=0
      fi
      prev_cnt="$cur_cnt"
      if [[ "$stable_hits" -ge 2 ]]; then
        break
      fi
      sleep 1
    done
  fi
}

amanita_cleanup_and_verify_port() {
  amanita_cleanup_dp2
  DP2_PID=""

  for _ in $(seq 1 5); do
    if ss -ltn | awk '{print $4}' | grep -Eq ":${DP2_PORT}$"; then
      sleep 1
    else
      break
    fi
  done

  if ss -ltn | awk '{print $4}' | grep -Eq ":${DP2_PORT}$"; then
    stage_fail "$RUN_ROOT" "$TEST_ID" "$STAGE_ID" "amanita" "cleanup failed: dp2 port ${DP2_PORT} is still in use"
  fi
}

amanita_validate_markers() {
  if [[ -n "$EXPECTED_MARKER" ]]; then
    grep -q "$EXPECTED_MARKER" "$DP1_STDOUT" || stage_fail "$RUN_ROOT" "$TEST_ID" "$STAGE_ID" "amanita" "Missing expected marker: $EXPECTED_MARKER"
  fi

  if [[ -n "$FORBID_MARKER" ]] && grep -q "$FORBID_MARKER" "$DP1_STDOUT"; then
    stage_fail "$RUN_ROOT" "$TEST_ID" "$STAGE_ID" "amanita" "Found forbidden marker: $FORBID_MARKER"
  fi
}

amanita_validate_artifacts() {
  JSON_CNT=$(find "$AMN_DP1_ROOT/data_bin" -maxdepth 1 -type f -name '*.json' 2>/dev/null | wc -l)
  BLOB_CNT=$(find "$AMN_DP1_ROOT/data_bin" -maxdepth 1 -type f -name '*.blob' 2>/dev/null | wc -l)
  PNG_CNT=$(find "${AMN_DP1_ROOT}frame_input" -maxdepth 1 -type f -name '*.png' 2>/dev/null | wc -l)

  [[ "$JSON_CNT" -gt 0 ]] || stage_fail "$RUN_ROOT" "$TEST_ID" "$STAGE_ID" "amanita" "No DP1 json artifacts produced"
  [[ "$BLOB_CNT" -gt 0 ]] || stage_fail "$RUN_ROOT" "$TEST_ID" "$STAGE_ID" "amanita" "No DP1 blob artifacts produced"
  [[ "$PNG_CNT" -gt 0 ]] || stage_fail "$RUN_ROOT" "$TEST_ID" "$STAGE_ID" "amanita" "No DP1 frame_input png artifacts produced"
}

amanita_write_checks() {
  cat > "$DP1_CHECKS" <<EOF
scenario=$STAGE_ID
test_id=$TEST_ID
rc=$RC
dp2_port=$DP2_PORT
dp2_drain_timeout_sec=$DP2_DRAIN_TIMEOUT_SEC
json_count=$JSON_CNT
blob_count=$BLOB_CNT
png_count=$PNG_CNT
out_dir=$OUT_DIR
timeout_sec=$TIMEOUT_SEC
EOF
}
