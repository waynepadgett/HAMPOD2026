#!/bin/bash
# HAMPOD Piper TTS Installer
# Downloads and installs Piper TTS and the voice model for HAMPOD firmware.
set -e

PIPER_VERSION="2023.11.14-2"
PIPER_URL="https://github.com/rhasspy/piper/releases/download/${PIPER_VERSION}/piper_linux_aarch64.tar.gz"
MODEL_URL="https://huggingface.co/rhasspy/piper-voices/resolve/main/en/en_US/lessac/low/en_US-lessac-low.onnx"
MODEL_JSON_URL="https://huggingface.co/rhasspy/piper-voices/resolve/main/en/en_US/lessac/low/en_US-lessac-low.onnx.json"

INSTALL_DIR="${HOME}/piper"
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
FIRMWARE_DIR="$(dirname "$(dirname "$SCRIPT_DIR")")/Firmware"

echo "=== HAMPOD Piper TTS Installer ==="
echo "Piper Version: ${PIPER_VERSION}"
echo "Install Dir: ${INSTALL_DIR}"
echo "Model Dir: ${FIRMWARE_DIR}/models/"
echo ""

# Check architecture
ARCH=$(uname -m)
if [ "$ARCH" != "aarch64" ]; then
    echo "WARNING: This script is intended for Raspberry Pi (aarch64)."
    echo "Detected architecture: $ARCH"
    read -p "Continue anyway? [y/N] " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        exit 1
    fi
fi

# 1. Download and extract Piper
echo "[1/4] Downloading Piper..."
if [ -d "${INSTALL_DIR}" ]; then
    echo "  Existing installation found at ${INSTALL_DIR}"
    read -p "  Overwrite? [y/N] " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        echo "  Skipping Piper download."
    else
        rm -rf "${INSTALL_DIR}"
        wget -q --show-progress -O /tmp/piper.tar.gz "${PIPER_URL}"
        echo "[2/4] Extracting to ${INSTALL_DIR}..."
        mkdir -p "${INSTALL_DIR}"
        tar -xzf /tmp/piper.tar.gz -C "${INSTALL_DIR}" --strip-components=1
        rm /tmp/piper.tar.gz
    fi
else
    wget -q --show-progress -O /tmp/piper.tar.gz "${PIPER_URL}"
    echo "[2/4] Extracting to ${INSTALL_DIR}..."
    mkdir -p "${INSTALL_DIR}"
    tar -xzf /tmp/piper.tar.gz -C "${INSTALL_DIR}" --strip-components=1
    rm /tmp/piper.tar.gz
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
mkdir -p "${FIRMWARE_DIR}/models"

if [ -f "${FIRMWARE_DIR}/models/en_US-lessac-low.onnx" ]; then
    echo "  Model already exists, skipping download."
else
    wget -q --show-progress -O "${FIRMWARE_DIR}/models/en_US-lessac-low.onnx" "${MODEL_URL}"
    wget -q --show-progress -O "${FIRMWARE_DIR}/models/en_US-lessac-low.onnx.json" "${MODEL_JSON_URL}"
fi

# 4. Verify
echo ""
echo "=== Verifying Installation ==="
echo -n "Piper version: "
piper --version || echo "(version check failed, but piper may still work)"

echo ""
echo "=== Installation Complete ==="
echo "Piper: ${INSTALL_DIR}/piper"
echo "Symlink: /usr/local/bin/piper"
echo "Model: ${FIRMWARE_DIR}/models/en_US-lessac-low.onnx"
echo ""
echo "You can now build the firmware with Piper TTS:"
echo "    cd ${FIRMWARE_DIR} && make"
