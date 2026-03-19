#!/bin/bash
# =============================================================================
# HAMPOD CLI Setup script
# =============================================================================
# A lightweight utility to quickly create the global `hampod` symlink 
# without needing to run the full `install_hampod.sh` installer.

set -e

SCRIPT_DIR="$( cd -P "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
CLI_SCRIPT="$SCRIPT_DIR/hampod_cli.sh"

echo "Setting up global 'hampod' command..."

if [ ! -f "$CLI_SCRIPT" ]; then
    echo "ERROR: CLI script not found at $CLI_SCRIPT"
    exit 3
fi

# Make it executable just in case
chmod +x "$CLI_SCRIPT" 2>/dev/null || true

# Add the main hampod CLI to PATH so it can be run globally
if [ -L "/usr/local/bin/hampod" ] || [ -f "/usr/local/bin/hampod" ]; then
    sudo rm -f "/usr/local/bin/hampod"
fi

if ! sudo ln -s "$CLI_SCRIPT" "/usr/local/bin/hampod"; then
    echo "ERROR: Failed to create symlink at /usr/local/bin/hampod"
    exit 3
fi

echo "Successfully linked $CLI_SCRIPT to /usr/local/bin/hampod"

# Verify /usr/local/bin is in PATH
if ! echo "$PATH" | tr ':' '\n' | grep -qx '/usr/local/bin'; then
    echo ""
    echo "WARNING: /usr/local/bin is not in your \$PATH."
    echo "The 'hampod' command may not work until you add it."
    echo "Add this line to your ~/.bashrc or ~/.profile:"
    echo "  export PATH=\"/usr/local/bin:\$PATH\""
else
    echo "You can now run 'hampod help' from any directory."
fi
