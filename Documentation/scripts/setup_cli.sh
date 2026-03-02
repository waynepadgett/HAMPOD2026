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

# Make it executable just in case
chmod +x "$CLI_SCRIPT" 2>/dev/null || true

# Add the main hampod CLI to PATH so it can be run globally
if [ -L "/usr/local/bin/hampod" ] || [ -f "/usr/local/bin/hampod" ]; then
    sudo rm -f "/usr/local/bin/hampod"
fi
sudo ln -s "$CLI_SCRIPT" "/usr/local/bin/hampod"

echo "Successfully linked $CLI_SCRIPT to /usr/local/bin/hampod"
echo "You can now run 'hampod help' from any directory."
