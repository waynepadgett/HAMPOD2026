#!/bin/bash
# =============================================================================
# Fix Beep Audio Format
# =============================================================================
# This script converts the beep WAV files from 32-bit to 16-bit format.
#
# The HAL Audio expects:
#   - Sample Rate: 16000 Hz
#   - Channels: 1 (mono)
#   - Bit Depth: 16-bit
#
# Piper TTS may generate 32-bit WAV files which cause format mismatch errors.
# This script uses sox to convert them to the correct format.
#
# Prerequisites:
#   - sox must be installed: sudo apt install sox
#
# Usage:
#   ./fix_beep_format.sh
#
# =============================================================================

set -e

# Configuration
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
PREGEN_DIR="$REPO_ROOT/Firmware/pregen_audio"

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

echo "=============================================="
echo "  HAMPOD Beep Audio Format Fix"
echo "=============================================="
echo ""

# Check if sox is installed
if ! command -v sox &> /dev/null; then
    echo -e "${RED}ERROR: sox is not installed${NC}"
    echo ""
    echo "Install sox with:"
    echo "  sudo apt install sox"
    echo ""
    exit 1
fi
echo -e "${GREEN}✓${NC} sox is installed"

# Check if pregen_audio directory exists
if [ ! -d "$PREGEN_DIR" ]; then
    echo -e "${RED}ERROR: pregen_audio directory not found at $PREGEN_DIR${NC}"
    exit 1
fi
echo -e "${GREEN}✓${NC} Found pregen_audio directory"
echo ""

# List of beep files to fix
BEEP_FILES=(
    "beep_keypress.wav"
    "beep_hold.wav"
    "beep_error.wav"
)

# Check current format of each file
echo "Checking current format of beep files:"
echo ""
for beep in "${BEEP_FILES[@]}"; do
    filepath="$PREGEN_DIR/$beep"
    if [ -f "$filepath" ]; then
        info=$(sox --info "$filepath" 2>/dev/null | grep -E "Precision|Sample Rate|Channels" || echo "unknown")
        echo "  $beep:"
        echo "$info" | sed 's/^/    /'
    else
        echo -e "  ${YELLOW}$beep: NOT FOUND${NC}"
    fi
done
echo ""

# Convert each beep file
echo "Converting beep files to 16-bit format:"
echo ""

for beep in "${BEEP_FILES[@]}"; do
    filepath="$PREGEN_DIR/$beep"
    
    if [ ! -f "$filepath" ]; then
        echo -e "  ${YELLOW}SKIP${NC} $beep (file not found)"
        continue
    fi
    
    # Check if already 16-bit
    current_bits=$(sox --info "$filepath" 2>/dev/null | grep "Precision" | grep -oE "[0-9]+" || echo "unknown")
    
    if [ "$current_bits" = "16" ]; then
        echo -e "  ${GREEN}OK${NC}   $beep (already 16-bit)"
        continue
    fi
    
    # Create backup
    cp "$filepath" "${filepath}.bak"
    
    # Convert to 16-bit, 16kHz, mono
    # sox input output rate 16000 channels 1 -b 16
    sox "$filepath.bak" -r 16000 -c 1 -b 16 "$filepath"
    
    if [ $? -eq 0 ]; then
        echo -e "  ${GREEN}FIXED${NC} $beep (converted from ${current_bits}-bit to 16-bit)"
        rm "${filepath}.bak"
    else
        echo -e "  ${RED}ERROR${NC} $beep (conversion failed, backup preserved)"
        mv "${filepath}.bak" "$filepath"
    fi
done

echo ""
echo "=============================================="
echo "  Verification"
echo "=============================================="
echo ""

# Verify the new format
all_good=true
for beep in "${BEEP_FILES[@]}"; do
    filepath="$PREGEN_DIR/$beep"
    if [ -f "$filepath" ]; then
        bits=$(sox --info "$filepath" 2>/dev/null | grep "Precision" | grep -oE "[0-9]+" || echo "unknown")
        rate=$(sox --info "$filepath" 2>/dev/null | grep "Sample Rate" | grep -oE "[0-9]+" || echo "unknown")
        channels=$(sox --info "$filepath" 2>/dev/null | grep "Channels" | grep -oE "[0-9]+" || echo "unknown")
        
        if [ "$bits" = "16" ] && [ "$rate" = "16000" ] && [ "$channels" = "1" ]; then
            echo -e "  ${GREEN}✓${NC} $beep: ${rate}Hz, ${channels}ch, ${bits}-bit"
        else
            echo -e "  ${RED}✗${NC} $beep: ${rate}Hz, ${channels}ch, ${bits}-bit (expected 16000Hz, 1ch, 16-bit)"
            all_good=false
        fi
    fi
done

echo ""
if [ "$all_good" = true ]; then
    echo -e "${GREEN}All beep files are now in the correct format!${NC}"
    echo ""
    echo "To commit these changes:"
    echo "  cd $REPO_ROOT"
    echo "  git add Firmware/pregen_audio/beep_*.wav"
    echo "  git commit -m 'audio: Fix beep WAV files to 16-bit format'"
else
    echo -e "${YELLOW}Some files may need manual attention.${NC}"
fi
echo ""
