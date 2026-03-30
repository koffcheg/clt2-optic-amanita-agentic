#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/lib/comparator_stage_lib.sh"

comparator_init_defaults
comparator_parse_args "$@"
comparator_validate_args
comparator_prepare_layout
comparator_run_blocking
comparator_validate_artifacts
comparator_write_checks

echo "OK comparator stage=$STAGE_ID test_id=$TEST_ID"
