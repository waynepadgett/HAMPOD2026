#!/bin/bash
# Regenerate pre-generated audio files using Piper TTS
# This replaces the old Festival-generated files with Piper-generated ones

PIPER_PATH="/home/waynesr/piper"
MODEL_PATH="/home/waynesr/HAMPOD2026/Firmware/models/en_US-lessac-low.onnx"
OUTPUT_DIR="/home/waynesr/HAMPOD2026/Firmware/pregen_audio"

cd "$(dirname "$0")"

# Check Piper exists
if [ ! -f "$PIPER_PATH/piper" ]; then
    echo "ERROR: Piper not found at $PIPER_PATH/piper"
    exit 1
fi

echo "Regenerating audio files with Piper TTS..."
echo "Model: $MODEL_PATH"
echo "Output: $OUTPUT_DIR"
echo

# Digits
for i in 0 1 2 3 4 5 6 7 8 9; do
    echo "Generating: $i"
    echo "$i" | "$PIPER_PATH/piper" --model "$MODEL_PATH" --output_file "$OUTPUT_DIR/$i.wav" 2>/dev/null
done

# Letters
for letter in A B C D; do
    echo "Generating: $letter"
    echo "$letter" | "$PIPER_PATH/piper" --model "$MODEL_PATH" --output_file "$OUTPUT_DIR/$letter.wav" 2>/dev/null
done

# Special keys
echo "Generating: POINT"
echo "point" | "$PIPER_PATH/piper" --model "$MODEL_PATH" --output_file "$OUTPUT_DIR/POINT.wav" 2>/dev/null

echo "Generating: POUND"
echo "pound" | "$PIPER_PATH/piper" --model "$MODEL_PATH" --output_file "$OUTPUT_DIR/POUND.wav" 2>/dev/null

# NOTE: DTMF files are NOT regenerated - they are actual DTMF tone audio files,
# not speech. The DTMF*.wav files should be left as-is.

echo
echo "Done! Audio files regenerated with Piper TTS."
echo "Note: DTMF tone files were not modified (they are actual tones, not speech)."
