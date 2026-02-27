#!/bin/bash
# HAMPOD Piper TTS Installer
# Downloads and installs Piper TTS and the voice model for HAMPOD firmware.
#
# Usage: ./install_piper.sh [--force]
#   --force  Overwrite existing installation without prompting
#   Do NOT run with sudo — the script will call sudo for the symlink step.
#
set -e

# Refuse to run as root — prevents piper installing to /root/
if [ "$(id -u)" -eq 0 ]; then
    echo "ERROR: Do not run this script with sudo or as root."
    echo ""
    echo "Usage: ./install_piper.sh [--force]"
    echo ""
    echo "The script will call sudo internally for steps that need it."
    exit 1
fi

# Parse arguments
FORCE=false
for arg in "$@"; do
    case $arg in
        --force|-f)
            FORCE=true
            shift
            ;;
    esac
done

PIPER_VERSION="2023.11.14-2"
PIPER_URL="https://github.com/rhasspy/piper/releases/download/${PIPER_VERSION}/piper_linux_aarch64.tar.gz"
MODEL_URL="https://huggingface.co/rhasspy/piper-voices/resolve/main/en/en_US/lessac/low/en_US-lessac-low.onnx"
MODEL_JSON_URL="https://huggingface.co/rhasspy/piper-voices/resolve/main/en/en_US/lessac/low/en_US-lessac-low.onnx.json"

INSTALL_DIR="${HOME}/piper"

# Detect FIRMWARE_DIR - try multiple methods
SCRIPT_DIR="$(cd "$(dirname "$0")" 2>/dev/null && pwd)" || SCRIPT_DIR="$PWD"

# Method 1: If run from Documentation/scripts, go up two levels
if [ -d "$SCRIPT_DIR/../../Firmware" ]; then
    FIRMWARE_DIR="$(cd "$SCRIPT_DIR/../../Firmware" && pwd)"
# Method 2: If in HAMPOD2026 directory
elif [ -d "$HOME/HAMPOD2026/Firmware" ]; then
    FIRMWARE_DIR="$HOME/HAMPOD2026/Firmware"
# Method 3: Current directory has Firmware
elif [ -d "./Firmware" ]; then
    FIRMWARE_DIR="$(pwd)/Firmware"
else
    echo "ERROR: Cannot find Firmware directory"
    echo "Run this script from the HAMPOD2026 repository root or Documentation/scripts/"
    exit 1
fi

MODEL_DIR="${FIRMWARE_DIR}/models"

echo "=== HAMPOD Piper TTS Installer ==="
echo "Piper Version: ${PIPER_VERSION}"
echo "Install Dir: ${INSTALL_DIR}"
echo "Model Dir: ${MODEL_DIR}/"
echo "Force mode: ${FORCE}"
echo ""

# Check architecture
ARCH=$(uname -m)
if [ "$ARCH" != "aarch64" ]; then
    echo "WARNING: This script is intended for Raspberry Pi (aarch64)."
    echo "Detected architecture: $ARCH"
    if [ "$FORCE" = false ]; then
        read -p "Continue anyway? [y/N] " -n 1 -r
        echo
        if [[ ! $REPLY =~ ^[Yy]$ ]]; then
            exit 1
        fi
    else
        echo "  Continuing anyway (--force)"
    fi
fi

# 1. Download and extract Piper
echo "[1/4] Downloading Piper..."
if [ -d "${INSTALL_DIR}" ]; then
    echo "  Existing installation found at ${INSTALL_DIR}"
    if [ "$FORCE" = true ]; then
        echo "  Overwriting (--force)"
        rm -rf "${INSTALL_DIR}"
    else
        read -p "  Overwrite? [y/N] " -n 1 -r
        echo
        if [[ ! $REPLY =~ ^[Yy]$ ]]; then
            echo "  Skipping Piper download."
        else
            rm -rf "${INSTALL_DIR}"
        fi
    fi
fi

# Download if not present
if [ ! -d "${INSTALL_DIR}" ]; then
    wget -q --show-progress -O /tmp/piper.tar.gz "${PIPER_URL}"
    echo "[2/4] Extracting to ${INSTALL_DIR}..."
    mkdir -p "${INSTALL_DIR}"
    tar -xzf /tmp/piper.tar.gz -C "${INSTALL_DIR}" --strip-components=1
    rm /tmp/piper.tar.gz
else
    echo "[2/4] Piper already installed, skipping extraction."
fi

# 2. Create symlink
echo "[3/4] Creating symlink in /usr/local/bin..."
if [ -L "/usr/local/bin/piper" ] || [ -f "/usr/local/bin/piper" ]; then
    echo "  Symlink already exists, updating..."
    sudo rm -f /usr/local/bin/piper
fi
sudo ln -s "${INSTALL_DIR}/piper" /usr/local/bin/piper
echo "  Created: /usr/local/bin/piper -> ${INSTALL_DIR}/piper"

# 3. Download voice model
echo "[4/4] Downloading voice model..."
mkdir -p "${MODEL_DIR}"

if [ -f "${MODEL_DIR}/en_US-lessac-low.onnx" ] && [ "$FORCE" = false ]; then
    echo "  Model already exists, skipping download."
else
    echo "  Downloading to ${MODEL_DIR}/en_US-lessac-low.onnx"
    wget -q --show-progress -O "${MODEL_DIR}/en_US-lessac-low.onnx" "${MODEL_URL}"
    wget -q --show-progress -O "${MODEL_DIR}/en_US-lessac-low.onnx.json" "${MODEL_JSON_URL}"
fi

# 4. Verify installation
echo ""
echo "=== Verifying Installation ==="

if [ -f "${INSTALL_DIR}/piper" ]; then
    echo "✓ Piper binary: ${INSTALL_DIR}/piper"
else
    echo "✗ ERROR: Piper binary not found!"
    exit 1
fi

if [ -f "${MODEL_DIR}/en_US-lessac-low.onnx" ]; then
    MODEL_SIZE=$(stat -c %s "${MODEL_DIR}/en_US-lessac-low.onnx" 2>/dev/null || stat -f %z "${MODEL_DIR}/en_US-lessac-low.onnx")
    echo "✓ Voice model: ${MODEL_DIR}/en_US-lessac-low.onnx (${MODEL_SIZE} bytes)"
else
    echo "✗ ERROR: Voice model not found!"
    exit 1
fi

echo ""
echo "=== Installation Complete ==="
echo "Piper: ${INSTALL_DIR}/piper"
echo "Symlink: /usr/local/bin/piper"
echo "Model: ${MODEL_DIR}/en_US-lessac-low.onnx"
echo ""
echo "You can now build the firmware with Piper TTS:"
echo "    cd ${FIRMWARE_DIR} && make"

