#!/usr/bin/env bash

stage_fail() {
  local run_root="$1"
  local test_id="$2"
  local stage_id="$3"
  local step_name="$4"
  local msg="$5"

  echo "ERROR: $msg" >&2
  if [[ -n "$run_root" ]]; then
    mkdir -p "$run_root"
    {
      echo "test_id=${test_id:-unknown}"
      echo "stage=${stage_id:-unknown}"
      echo "step=${step_name:-unknown}"
      echo "reason=$msg"
      echo "timestamp=$(date +%Y-%m-%dT%H:%M:%S)"
    } > "$run_root/FAILED.txt"
  fi
  exit 1
}

copy_if_needed() {
  local src="$1"
  local dst="$2"
  local src_abs dst_abs

  src_abs=$(readlink -f "$src")
  dst_abs=$(readlink -f "$dst" 2>/dev/null || true)
  if [[ -n "$dst_abs" && "$src_abs" == "$dst_abs" ]]; then
    return 0
  fi
  cp "$src" "$dst"
}

assert_executable() {
  local p="$1"
  local label="$2"
  [[ -x "$p" ]] || { echo "$label is not executable: $p" >&2; exit 2; }
}

assert_file() {
  local p="$1"
  local label="$2"
  [[ -f "$p" ]] || { echo "$label missing: $p" >&2; exit 2; }
}

assert_non_empty() {
  local v="$1"
  local label="$2"
  [[ -n "$v" ]] || { echo "$label is required" >&2; exit 2; }
}