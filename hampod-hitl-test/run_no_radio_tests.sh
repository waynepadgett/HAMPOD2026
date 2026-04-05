#!/usr/bin/env bash
# run_no_radio_tests.sh
# Entry point for Job 1: Audio + Keypad HITL tests (no radio required).
# Called by GitHub Actions on the Pi 5 self-hosted runner.
#
# TODO: Replace this stub with actual test invocations as tests are written.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

echo "=== HAMPOD HITL: Audio + Keypad Tests ==="
echo "Runner: $(hostname)"
echo "Branch: ${GITHUB_REF_NAME:-local}"
echo ""

# TODO: add test calls here, e.g.:
#   bash "$SCRIPT_DIR/audio/test_startup_announcement.sh"
#   bash "$SCRIPT_DIR/audio/test_frequency_entry_tts.sh"
#   bash "$SCRIPT_DIR/input/test_keypad_simulation.sh"

echo "No tests implemented yet — stub passes."
exit 0
