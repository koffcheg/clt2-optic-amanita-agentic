#!/usr/bin/env bash

# summary_lib.sh - Generate Summary.md for test run
# The output format and metrics are aligned with canonical AmanitaResources run summaries.

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
source "$SCRIPT_DIR/lib/stage_common.sh"

summary_init_defaults() {
  RUN_ROOT=""
  TEST_ID=""
  STAGE_ID=""
  STAGE_IDS=()
  DATASET_NAME=""
  TEST_DESCRIPTION=""
  TEST_DATE=""
  SUMMARY_FILE=""
}

summary_parse_args() {
  while [[ $# -gt 0 ]]; do
    case "$1" in
      --run-root) RUN_ROOT="$2"; shift 2 ;;
      --test-id) TEST_ID="$2"; shift 2 ;;
      --stage-id) STAGE_ID="$2"; shift 2 ;;
      --dataset) DATASET_NAME="$2"; shift 2 ;;
      --description) TEST_DESCRIPTION="$2"; shift 2 ;;
      --test-date) TEST_DATE="$2"; shift 2 ;;
      -h|--help) summary_usage; exit 0 ;;
      *) echo "Unknown argument: $1" >&2; summary_usage; exit 2 ;;
    esac
  done
}

summary_usage() {
  cat <<'EOF'
Usage:
  generate_summary.sh \
    --run-root <path> \
    --test-id <id> \
    [--stage-id <id>] \
    [--dataset <name>] \
    [--description <text>] \
    [--test-date "YYYY-MM-DD HH:MM:SS"]

Generates <TestId>_Summary.md with:
- canonical metadata section
- DP1/DP2 minimal metrics
- Comparator aggregate and per-report metrics
EOF
}

summary_validate_args() {
  assert_non_empty "$RUN_ROOT" "run root"
  assert_non_empty "$TEST_ID" "test id"
}

summary_extract_timing() {
  local timing_file="$1"
  local key="$2"

  if [[ -f "$timing_file" ]]; then
    local v
    v=$(grep "^${key}=" "$timing_file" 2>/dev/null | head -n 1 | cut -d= -f2)
    if [[ -n "$v" ]]; then
      echo "$v"
      return 0
    fi
    echo "N/A"
  else
    echo "N/A"
  fi
}

summary_detect_stage_ids() {
  STAGE_IDS=()

  if [[ -n "$STAGE_ID" ]]; then
    STAGE_IDS+=("$STAGE_ID")
    return 0
  fi

  local ama_dir="$RUN_ROOT/Logs/Amanita"
  local cmp_dir="$RUN_ROOT/Logs/Comparator"
  local ama_stages=""
  local cmp_stages=""

  if [[ -d "$ama_dir" ]]; then
    ama_stages=$(find "$ama_dir" -mindepth 1 -maxdepth 1 -type d -printf '%f\n' | sort)
  fi
  if [[ -d "$cmp_dir" ]]; then
    cmp_stages=$(find "$cmp_dir" -mindepth 1 -maxdepth 1 -type d -printf '%f\n' | sort)
  fi

  local combined
  combined=$(printf "%s\n%s\n" "$ama_stages" "$cmp_stages" | sed '/^$/d' | sort -u)
  while IFS= read -r st; do
    [[ -n "$st" ]] || continue
    # Keep only stages that have both Amanita and Comparator logs.
    if [[ -d "$ama_dir/$st" && -d "$cmp_dir/$st" ]]; then
      STAGE_IDS+=("$st")
    fi
  done <<< "$combined"

  [[ ${#STAGE_IDS[@]} -gt 0 ]] || stage_fail "$RUN_ROOT" "$TEST_ID" "summary" "summary" "Cannot auto-detect stage ids; pass --stage-id"
  STAGE_ID="${STAGE_IDS[0]}"
}

summary_extract_run_out_dirs_for_stage() {
  local stage_id="$1"
  local dp1_cfg="$RUN_ROOT/Configs/Amanita/DP1/$stage_id/config_datapro1.json"
  local dp2_cfg="$RUN_ROOT/Configs/Amanita/DP2/$stage_id/config_datapro2.json"

  local dp1_out="$RUN_ROOT/RunResults/Amanita/DP1"
  local dp2_out="$RUN_ROOT/RunResults/Amanita/DP2"

  if [[ -f "$dp1_cfg" ]]; then
    local v
    v=$(python3 - "$dp1_cfg" <<'PY'
import json
import sys
p = sys.argv[1]
try:
    d = json.load(open(p, 'r', encoding='utf-8'))
    print(d.get('config', {}).get('test', {}).get('out_folder', ''))
except Exception:
    print('')
PY
)
    [[ -n "$v" ]] && dp1_out="$v"
  fi

  if [[ -f "$dp2_cfg" ]]; then
    local v
    v=$(python3 - "$dp2_cfg" <<'PY'
import json
import sys
p = sys.argv[1]
try:
    d = json.load(open(p, 'r', encoding='utf-8'))
    print(d.get('strobe-method-par', {}).get('dp2_out_folder', ''))
except Exception:
    print('')
PY
)
    [[ -n "$v" ]] && dp2_out="$v"
  fi

  echo "$dp1_out|$dp2_out"
}

summary_collect_dp_totals() {
  local dp1_root="$1"
  local dp2_root="$2"

  python3 - "$dp1_root" "$dp2_root" <<'PY'
import glob
import json
import sys

dp1_root = sys.argv[1]
dp2_root = sys.argv[2]

dp1_total = 0
for p in sorted(glob.glob(f"{dp1_root}/data_bin/*.json")):
    try:
        with open(p, "r", encoding="utf-8") as fh:
            d = json.load(fh)
        dp1_total += int(d.get("Measurements frame", {}).get("Size", 0) or 0)
    except Exception:
        pass

dp2_total = 0
for p in sorted(glob.glob(f"{dp2_root}/*.json")):
    try:
        with open(p, "r", encoding="utf-8") as fh:
            d = json.load(fh)
        dp2_total += len(d.get("trajectories", []) or [])
    except Exception:
        pass

print(f"dp1_total={dp1_total}")
print(f"dp2_total={dp2_total}")
PY
}

summary_generate_single_stage() {
  local stage_id="$1"

  # Collect paths
  local amanita_log_dir="$RUN_ROOT/Logs/Amanita/$stage_id"
  local comp_log_dir="$RUN_ROOT/Logs/Comparator/$stage_id"
  local comp_stage_dir="$RUN_ROOT/RunResults/Comparator/$stage_id"

  local dp_dirs
  dp_dirs=$(summary_extract_run_out_dirs_for_stage "$stage_id")
  local amanita_dp1_root amanita_dp2_root
  IFS='|' read -r amanita_dp1_root amanita_dp2_root <<< "$dp_dirs"

  local dp1_time
  dp1_time=$(summary_extract_timing "$amanita_log_dir/time.txt" "real_sec")

  local comp_time
  comp_time=$(summary_extract_timing "$comp_log_dir/time.txt" "real_sec")

  local dp2_time
  dp2_time=$(summary_calculate_dp2_time "$amanita_log_dir/dp2_stdout.log")

  local metrics_blob
  metrics_blob=$(summary_collect_dp_totals "$amanita_dp1_root" "$amanita_dp2_root")

  local dp1_total="0"
  local dp2_total="0"
  while IFS= read -r line; do
    case "$line" in
      dp1_total=*) dp1_total="${line#dp1_total=}" ;;
      dp2_total=*) dp2_total="${line#dp2_total=}" ;;
    esac
  done <<< "$metrics_blob"

  local comp_metrics
  comp_metrics=$(summary_collect_comparator_metrics "$comp_stage_dir/report")

  local comp_report_files="0"
  local comp_frames_total="0"
  local comp_mean_overlap="N/A"
  local comp_mean_rms="N/A"
  local comp_mean_fp="N/A"
  local comp_mean_fn="N/A"
  local dataset_guess=""
  local report_lines=()

  while IFS= read -r line; do
    case "$line" in
      report_files=*) comp_report_files="${line#report_files=}" ;;
      frames_total=*) comp_frames_total="${line#frames_total=}" ;;
      mean_overlap=*) comp_mean_overlap="${line#mean_overlap=}" ;;
      mean_rms=*) comp_mean_rms="${line#mean_rms=}" ;;
      mean_fp=*) comp_mean_fp="${line#mean_fp=}" ;;
      mean_fn=*) comp_mean_fn="${line#mean_fn=}" ;;
      dataset_guess=*) dataset_guess="${line#dataset_guess=}" ;;
      report_line=*) report_lines+=("${line#report_line=}") ;;
    esac
  done <<< "$comp_metrics"

  if [[ -z "$DATASET_NAME" ]]; then
    DATASET_NAME="$dataset_guess"
  fi
  if [[ -z "$DATASET_NAME" ]]; then
    DATASET_NAME="unknown"
  fi

  if [[ -z "$TEST_DESCRIPTION" ]]; then
    TEST_DESCRIPTION="Auto-generated summary for stage $stage_id."
  fi

  if [[ -z "$TEST_DATE" ]]; then
    TEST_DATE=$(date "+%Y-%m-%d %H:%M:%S")
  fi

  # Generate canonical single-stage Summary.md
  cat > "$SUMMARY_FILE" <<EOF
# Test Summary

- test_id: $TEST_ID
- test_date: $TEST_DATE
- dataset: $DATASET_NAME
- test_description: $TEST_DESCRIPTION

## DP1/DP2 minimal metrics
- dp1_total_objects_all_frames: $dp1_total
- dp1_execution_time_sec: $dp1_time
- dp2_total_objects_all_frames: $dp2_total
- dp2_execution_time_sec: $dp2_time

## Comparator report
- comparator_execution_time_sec: $comp_time
- comparator_report_files: $comp_report_files
- comparator_frames_compared_total: $comp_frames_total
- comparator_mean_overlap_pct: $comp_mean_overlap
- comparator_mean_rms_deviation_area: $comp_mean_rms
- comparator_mean_false_positives_pct: $comp_mean_fp
- comparator_mean_false_negatives_pct: $comp_mean_fn
- comparator_reports:
EOF

  local r file_name frames rep_ov rep_rms
  for r in "${report_lines[@]}"; do
    IFS='|' read -r file_name frames rep_ov rep_rms <<< "$r"
    echo "  - $file_name: frames=$frames, mean_overlap_pct=$rep_ov, mean_rms=$rep_rms" >> "$SUMMARY_FILE"
  done

  cat >> "$SUMMARY_FILE" <<EOF

## Result summary
- PASS: Amanita and Comparator stages completed successfully.
EOF
}

summary_generate_multi_stage() {
  local stages_count="${#STAGE_IDS[@]}"
  local run_date
  run_date=$(date "+%Y-%m-%d %H:%M:%S")

  [[ -n "$TEST_DATE" ]] || TEST_DATE="$run_date"
  [[ -n "$DATASET_NAME" ]] || DATASET_NAME="multi-run"
  [[ -n "$TEST_DESCRIPTION" ]] || TEST_DESCRIPTION="Auto-generated aggregated summary for multi-run scenario."

  local total_dp1=0
  local total_dp2=0
  local total_comp_reports=0
  local total_frames=0

  local weighted_overlap_sum="0.0"
  local weighted_rms_sum="0.0"
  local weighted_fp_sum="0.0"
  local weighted_fn_sum="0.0"

  local stage_blocks=()

  local stage_id
  for stage_id in "${STAGE_IDS[@]}"; do
    local amanita_log_dir="$RUN_ROOT/Logs/Amanita/$stage_id"
    local comp_log_dir="$RUN_ROOT/Logs/Comparator/$stage_id"
    local comp_stage_dir="$RUN_ROOT/RunResults/Comparator/$stage_id"

    local dp_dirs
    dp_dirs=$(summary_extract_run_out_dirs_for_stage "$stage_id")
    local dp1_root dp2_root
    IFS='|' read -r dp1_root dp2_root <<< "$dp_dirs"

    local dp_totals
    dp_totals=$(summary_collect_dp_totals "$dp1_root" "$dp2_root")
    local dp1_total="0"
    local dp2_total="0"
    while IFS= read -r line; do
      case "$line" in
        dp1_total=*) dp1_total="${line#dp1_total=}" ;;
        dp2_total=*) dp2_total="${line#dp2_total=}" ;;
      esac
    done <<< "$dp_totals"

    local dp1_time
    dp1_time=$(summary_extract_timing "$amanita_log_dir/time.txt" "real_sec")
    local dp2_time
    dp2_time=$(summary_calculate_dp2_time "$amanita_log_dir/dp2_stdout.log")
    local comp_time
    comp_time=$(summary_extract_timing "$comp_log_dir/time.txt" "real_sec")

    local comp_metrics
    comp_metrics=$(summary_collect_comparator_metrics "$comp_stage_dir/report")
    local comp_report_files="0"
    local comp_frames_total="0"
    local comp_mean_overlap="N/A"
    local comp_mean_rms="N/A"
    local comp_mean_fp="N/A"
    local comp_mean_fn="N/A"
    local report_lines=()
    local dataset_guess=""

    while IFS= read -r line; do
      case "$line" in
        report_files=*) comp_report_files="${line#report_files=}" ;;
        frames_total=*) comp_frames_total="${line#frames_total=}" ;;
        mean_overlap=*) comp_mean_overlap="${line#mean_overlap=}" ;;
        mean_rms=*) comp_mean_rms="${line#mean_rms=}" ;;
        mean_fp=*) comp_mean_fp="${line#mean_fp=}" ;;
        mean_fn=*) comp_mean_fn="${line#mean_fn=}" ;;
        dataset_guess=*) dataset_guess="${line#dataset_guess=}" ;;
        report_line=*) report_lines+=("${line#report_line=}") ;;
      esac
    done <<< "$comp_metrics"

    if [[ "$DATASET_NAME" == "multi-run" && -n "$dataset_guess" ]]; then
      DATASET_NAME="$dataset_guess (multi-run)"
    fi

    total_dp1=$((total_dp1 + dp1_total))
    total_dp2=$((total_dp2 + dp2_total))
    total_comp_reports=$((total_comp_reports + comp_report_files))
    total_frames=$((total_frames + comp_frames_total))

    if [[ "$comp_mean_overlap" != "N/A" && "$comp_frames_total" -gt 0 ]]; then
      weighted_overlap_sum=$(python3 - <<PY
print(float("$weighted_overlap_sum") + float("$comp_mean_overlap") * int("$comp_frames_total"))
PY
)
    fi
    if [[ "$comp_mean_rms" != "N/A" && "$comp_frames_total" -gt 0 ]]; then
      weighted_rms_sum=$(python3 - <<PY
print(float("$weighted_rms_sum") + float("$comp_mean_rms") * int("$comp_frames_total"))
PY
)
    fi
    if [[ "$comp_mean_fp" != "N/A" && "$comp_frames_total" -gt 0 ]]; then
      weighted_fp_sum=$(python3 - <<PY
print(float("$weighted_fp_sum") + float("$comp_mean_fp") * int("$comp_frames_total"))
PY
)
    fi
    if [[ "$comp_mean_fn" != "N/A" && "$comp_frames_total" -gt 0 ]]; then
      weighted_fn_sum=$(python3 - <<PY
print(float("$weighted_fn_sum") + float("$comp_mean_fn") * int("$comp_frames_total"))
PY
)
    fi

    local block
    block="## Stage $stage_id
- dp1_total_objects_all_frames: $dp1_total
- dp1_execution_time_sec: $dp1_time
- dp2_total_objects_all_frames: $dp2_total
- dp2_execution_time_sec: $dp2_time
- comparator_execution_time_sec: $comp_time
- comparator_report_files: $comp_report_files
- comparator_frames_compared_total: $comp_frames_total
- comparator_mean_overlap_pct: $comp_mean_overlap
- comparator_mean_rms_deviation_area: $comp_mean_rms
- comparator_mean_false_positives_pct: $comp_mean_fp
- comparator_mean_false_negatives_pct: $comp_mean_fn
- comparator_reports:"
    local rr
    for rr in "${report_lines[@]}"; do
      local file_name frames rep_ov rep_rms
      IFS='|' read -r file_name frames rep_ov rep_rms <<< "$rr"
      block+="
  - $file_name: frames=$frames, mean_overlap_pct=$rep_ov, mean_rms=$rep_rms"
    done
    stage_blocks+=("$block")
  done

  local agg_overlap="N/A"
  local agg_rms="N/A"
  local agg_fp="N/A"
  local agg_fn="N/A"
  if [[ "$total_frames" -gt 0 ]]; then
    agg_overlap=$(python3 - <<PY
print(f"{float('$weighted_overlap_sum') / int('$total_frames'):.2f}")
PY
)
    agg_rms=$(python3 - <<PY
print(f"{float('$weighted_rms_sum') / int('$total_frames'):.2f}")
PY
)
    agg_fp=$(python3 - <<PY
print(f"{float('$weighted_fp_sum') / int('$total_frames'):.2f}")
PY
)
    agg_fn=$(python3 - <<PY
print(f"{float('$weighted_fn_sum') / int('$total_frames'):.2f}")
PY
)
  fi

  cat > "$SUMMARY_FILE" <<EOF
# Підсумок multi-run тесту

- test_id: $TEST_ID
- test_date: $TEST_DATE
- dataset: $DATASET_NAME
- test_description: $TEST_DESCRIPTION
- stages_count: $stages_count

## Загальний підсумок multi-run
- dp1_total_objects_all_stages: $total_dp1
- dp2_total_objects_all_stages: $total_dp2
- comparator_report_files_total: $total_comp_reports
- comparator_frames_compared_total: $total_frames
- comparator_mean_overlap_pct_weighted: $agg_overlap
- comparator_mean_rms_deviation_area_weighted: $agg_rms
- comparator_mean_false_positives_pct_weighted: $agg_fp
- comparator_mean_false_negatives_pct_weighted: $agg_fn
EOF

  local b
  for b in "${stage_blocks[@]}"; do
    printf "\n%s\n" "$b" >> "$SUMMARY_FILE"
  done

  cat >> "$SUMMARY_FILE" <<EOF

## Result summary
- PASS: multi-run stages aggregated into one summary report.
EOF
}

summary_calculate_dp2_time() {
  local dp2_stdout="$1"
  if [[ ! -f "$dp2_stdout" ]]; then
    echo "N/A"
    return 0
  fi

  python3 - "$dp2_stdout" <<'PY'
import datetime
import re
import sys

path = sys.argv[1]
ts_re = re.compile(r"^\[(\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2},\d{3})\]")
first = None
last = None

with open(path, "r", encoding="utf-8", errors="ignore") as f:
    for line in f:
        m = ts_re.match(line)
        if not m:
            continue
        cur = datetime.datetime.strptime(m.group(1), "%Y-%m-%d %H:%M:%S,%f")
        if first is None:
            first = cur
        last = cur

if first is None or last is None:
    print("N/A")
else:
    print(f"{(last - first).total_seconds():.3f}")
PY
}

summary_collect_comparator_metrics() {
  local report_dir="$1"
  python3 - "$report_dir" <<'PY'
import json
import pathlib
import statistics
import sys

report_dir = pathlib.Path(sys.argv[1])
files = sorted(report_dir.glob("*.json")) if report_dir.exists() else []

def f2(v):
    return f"{v:.2f}"

def trim(v):
    s = f"{v:.2f}"
    if "." in s:
        s = s.rstrip("0").rstrip(".")
    return s

overlap_all = []
rms_all = []
fp_all = []
fn_all = []
frames_total = 0
report_lines = []
dataset_guess = ""

for p in files:
    with open(p, "r", encoding="utf-8") as fh:
        data = json.load(fh)

    if not dataset_guess:
        exp = str(data.get("expected_data_folder", ""))
        marker = "/datasets/raw/"
        if marker in exp:
            suffix = exp.split(marker, 1)[1].strip("/")
            parts = suffix.split("/")
            if len(parts) >= 2:
                dataset_guess = f"{parts[0]}/{parts[1]}"
            elif parts:
                dataset_guess = parts[0]

    frame_comparisons = data.get("frame_comparisons", []) or []
    frames_total += len(frame_comparisons)

    rep_ov = []
    rep_rms = []
    for fc in frame_comparisons:
        ov = fc.get("overlap_percentage")
        rms = fc.get("rms_deviation_area")
        fp = fc.get("false_positives")
        fn = fc.get("false_negatives")

        if ov is not None:
            val = float(ov)
            overlap_all.append(val)
            rep_ov.append(val)
        if rms is not None:
            val = float(rms)
            rms_all.append(val)
            rep_rms.append(val)
        if fp is not None:
            fp_all.append(float(fp))
        if fn is not None:
            fn_all.append(float(fn))

    mean_rep_ov = trim(statistics.fmean(rep_ov)) if rep_ov else "N/A"
    mean_rep_rms = trim(statistics.fmean(rep_rms)) if rep_rms else "N/A"
    report_lines.append(f"{p.name}|{len(frame_comparisons)}|{mean_rep_ov}|{mean_rep_rms}")

mean_ov = f2(statistics.fmean(overlap_all)) if overlap_all else "N/A"
mean_rms = f2(statistics.fmean(rms_all)) if rms_all else "N/A"
mean_fp = f2(statistics.fmean(fp_all)) if fp_all else "N/A"
mean_fn = f2(statistics.fmean(fn_all)) if fn_all else "N/A"

print(f"report_files={len(files)}")
print(f"frames_total={frames_total}")
print(f"mean_overlap={mean_ov}")
print(f"mean_rms={mean_rms}")
print(f"mean_fp={mean_fp}")
print(f"mean_fn={mean_fn}")
print(f"dataset_guess={dataset_guess}")
for line in report_lines:
    print(f"report_line={line}")
PY
}

summary_generate() {
  SUMMARY_FILE="$RUN_ROOT/${TEST_ID}_Summary.md"

  summary_detect_stage_ids
  if [[ ${#STAGE_IDS[@]} -eq 1 ]]; then
    summary_generate_single_stage "${STAGE_IDS[0]}"
  else
    summary_generate_multi_stage
  fi

  echo "Summary generated: $SUMMARY_FILE"
}

summary_validate_summary() {
  # Verify that summary file was created and has required fields
  [[ -f "$SUMMARY_FILE" ]] || { echo "Summary file not created: $SUMMARY_FILE" >&2; return 1; }

  # Common required fields
  local common_required=("test_id:" "test_date:" "dataset:" "## Result summary")
  local field
  for field in "${common_required[@]}"; do
    grep -q "$field" "$SUMMARY_FILE" || {
      echo "Missing required field in summary: $field" >&2
      return 1
    }
  done

  # Single-stage or multi-run required markers
  if grep -q "# Підсумок multi-run тесту" "$SUMMARY_FILE"; then
    local multi_required=(
      "stages_count:"
      "## Загальний підсумок multi-run"
      "dp1_total_objects_all_stages:"
      "dp2_total_objects_all_stages:"
      "comparator_frames_compared_total:"
      "PASS: multi-run stages aggregated into one summary report."
    )
    for field in "${multi_required[@]}"; do
      grep -q "$field" "$SUMMARY_FILE" || {
        echo "Missing multi-run field in summary: $field" >&2
        return 1
      }
    done
  else
    local single_required=(
      "dp1_total_objects_all_frames:"
      "dp2_total_objects_all_frames:"
      "comparator_mean_overlap_pct:"
      "comparator_reports:"
      "PASS: Amanita and Comparator stages completed successfully."
    )
    for field in "${single_required[@]}"; do
      grep -q "$field" "$SUMMARY_FILE" || {
        echo "Missing single-stage field in summary: $field" >&2
        return 1
      }
    done
  fi
  
  return 0
}
