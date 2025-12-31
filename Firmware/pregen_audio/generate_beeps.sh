#!/bin/bash
# generate_beeps.sh - Generate beep audio files for HAMPOD
#
# This script generates the beep audio files required for audio feedback.
# Run this script on the Raspberry Pi after installing sox:
#   sudo apt install sox
#
# Usage:
#   cd ~/HAMPOD2026/Firmware/pregen_audio
#   chmod +x generate_beeps.sh
#   ./generate_beeps.sh

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
OUTPUT_DIR="$SCRIPT_DIR"

echo "=== HAMPOD Beep Generator ==="
echo "Output directory: $OUTPUT_DIR"
echo ""

# Check for sox
if ! command -v sox &> /dev/null; then
    echo "ERROR: sox is not installed."
    echo "Install it with: sudo apt install sox"
    exit 1
fi

# Key press beep: 50ms, 1000Hz sine wave (medium pitch)
echo "Generating beep_keypress.wav (50ms, 1000Hz)..."
sox -n "$OUTPUT_DIR/beep_keypress.wav" synth 0.05 sine 1000 vol 0.5

# Hold indicator beep: 50ms, 700Hz sine wave (lower pitch)
echo "Generating beep_hold.wav (50ms, 700Hz)..."
sox -n "$OUTPUT_DIR/beep_hold.wav" synth 0.05 sine 700 vol 0.5

# Error beep: 100ms, 400Hz sine wave (even lower pitch)
echo "Generating beep_error.wav (100ms, 400Hz)..."
sox -n "$OUTPUT_DIR/beep_error.wav" synth 0.1 sine 400 vol 0.5

echo ""
echo "=== Generated files ==="
ls -la "$OUTPUT_DIR"/beep_*.wav

echo ""
echo "=== Test playback ==="
echo "Test each beep with:"
echo "  aplay $OUTPUT_DIR/beep_keypress.wav"
echo "  aplay $OUTPUT_DIR/beep_hold.wav"
echo "  aplay $OUTPUT_DIR/beep_error.wav"
echo ""
echo "Done!"
