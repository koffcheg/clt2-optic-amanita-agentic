#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/lib/summary_lib.sh"

summary_init_defaults
summary_parse_args "$@"
summary_validate_args
summary_generate
summary_validate_summary

echo "OK summary test_id=$TEST_ID"
