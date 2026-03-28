#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/lib/amanita_stage_lib.sh"

amanita_init_defaults
amanita_parse_args "$@"
amanita_validate_args
amanita_prepare_layout

trap amanita_cleanup_dp2 EXIT

amanita_preflight
amanita_validate_ports_in_configs
amanita_validate_out_folder
amanita_start_dp2
amanita_wait_dp2_ready
amanita_run_dp1_blocking
amanita_wait_dp2_drain
amanita_cleanup_and_verify_port
amanita_validate_markers
amanita_validate_artifacts
amanita_write_checks

echo "OK amanita stage=$STAGE_ID test_id=$TEST_ID"
