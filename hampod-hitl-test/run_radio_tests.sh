#!/usr/bin/env bash
# run_radio_tests.sh
# Entry point for Job 2: Radio Control HITL tests.
# Called by GitHub Actions on the Pi 5 self-hosted runner.
#
# Auto-skips (exit 0) if no supported radio is detected via hamlib.
# This allows the job to pass cleanly when the radio is off or unplugged.
#
# TODO: Replace this stub with actual test invocations as tests are written.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

echo "=== HAMPOD HITL: Radio Control Tests ==="
echo "Runner: $(hostname)"
echo "Branch: ${GITHUB_REF_NAME:-local}"
echo ""

# --- Radio detection ---
# TODO: Replace with the actual port and model used by HAMPOD.
# Example hamlib probe (rigctl -m 1 = dummy, use real model for IC-7300 etc.)
# RADIO_PORT="/dev/ttyUSB0"
# if ! rigctl -m 3085 -r "$RADIO_PORT" -C retry=1 get_freq > /dev/null 2>&1; then
#     echo "No radio detected on $RADIO_PORT — skipping radio tests."
#     exit 0
# fi

# Stub: always skip for now until detection logic and tests are implemented.
echo "Radio detection not yet implemented — skipping radio tests."
exit 0

# TODO: add radio test calls here, e.g.:
#   bash "$SCRIPT_DIR/radio/test_frequency_readback.sh"
#   bash "$SCRIPT_DIR/radio/test_band_change.sh"
