#!/bin/bash
# ============================================================================
# HAMPOD Service Wrapper Script
# ============================================================================
# Non-interactive wrapper for running HAMPOD via systemd.
# This script is called by the hampod.service unit file.
#
# Unlike run_hampod.sh (interactive), this script:
#   - Runs silently without user prompts
#   - Logs to systemd journal
#   - Properly forks for systemd Type=forking
#
# ============================================================================

# Configuration
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
FIRMWARE_DIR="$REPO_ROOT/Firmware"
SOFTWARE2_DIR="$REPO_ROOT/Software2"
LOG_TAG="hampod-service"

# Log to syslog and stdout
log() {
    echo "[$(date '+%Y-%m-%d %H:%M:%S')] $1"
    logger -t "$LOG_TAG" "$1"
}

# ============================================================================
# Cleanup Function
# ============================================================================

cleanup() {
    log "Cleaning up stale processes and pipes..."
    
    # =========================================================================
    # DEFENSIVE: Unblock WiFi in case something (possibly our code) blocked it
    # See: https://github.com/waynepadgett/HAMPOD2026/issues/TBD
    # =========================================================================
    if command -v rfkill &> /dev/null; then
        rfkill unblock wifi 2>/dev/null || true
        log "Ensured WiFi is unblocked (defensive measure)"
    fi
    
    # Kill any existing processes
    killall -9 firmware.elf 2>/dev/null || true
    killall -9 hampod 2>/dev/null || true
    killall -9 piper 2>/dev/null || true
    killall -9 aplay 2>/dev/null || true
    
    # Remove stale pipes
    rm -f "$FIRMWARE_DIR/Firmware_i" "$FIRMWARE_DIR/Firmware_o" 2>/dev/null || true
    rm -f "$FIRMWARE_DIR/Speaker_i" "$FIRMWARE_DIR/Speaker_o" 2>/dev/null || true
    rm -f "$FIRMWARE_DIR/Keypad_i" "$FIRMWARE_DIR/Keypad_o" 2>/dev/null || true
    
    sleep 1
}

# ============================================================================
# Start HAMPOD
# ============================================================================

start_hampod() {
    log "Starting HAMPOD service..."
    
    # Check that executables exist
    if [ ! -f "$FIRMWARE_DIR/firmware.elf" ]; then
        log "ERROR: firmware.elf not found at $FIRMWARE_DIR/firmware.elf"
        log "Please run 'make' in the Firmware directory first"
        exit 1
    fi
    
    if [ ! -f "$SOFTWARE2_DIR/bin/hampod" ]; then
        log "ERROR: hampod not found at $SOFTWARE2_DIR/bin/hampod"
        log "Please run 'make' in the Software2 directory first"
        exit 1
    fi
    
    # Start Firmware in background
    log "Starting Firmware..."
    cd "$FIRMWARE_DIR"
    ./firmware.elf > /tmp/hampod_firmware.log 2>&1 &
    FIRMWARE_PID=$!
    log "Firmware started with PID: $FIRMWARE_PID"
    
    # Wait for Firmware to create pipes
    log "Waiting for Firmware pipes..."
    for i in $(seq 1 30); do
        if [ -p "$FIRMWARE_DIR/Firmware_o" ]; then
            break
        fi
        sleep 0.2
    done
    
    if [ ! -p "$FIRMWARE_DIR/Firmware_o" ]; then
        log "ERROR: Firmware_o pipe not created after 6 seconds"
        log "Check /tmp/hampod_firmware.log for errors"
        kill $FIRMWARE_PID 2>/dev/null || true
        exit 1
    fi
    
    log "Firmware pipes ready"
    
    # Start Software2 (hampod) in background
    log "Starting Software2 (hampod)..."
    cd "$SOFTWARE2_DIR"
    ./bin/hampod > /tmp/hampod_software.log 2>&1 &
    HAMPOD_PID=$!
    log "HAMPOD started with PID: $HAMPOD_PID"
    
    # Brief wait to verify it's running
    sleep 2
    
    if ! kill -0 $HAMPOD_PID 2>/dev/null; then
        log "ERROR: hampod process died immediately"
        log "Check /tmp/hampod_software.log for errors"
        kill $FIRMWARE_PID 2>/dev/null || true
        exit 1
    fi
    
    log "HAMPOD service started successfully"
    log "Firmware PID: $FIRMWARE_PID, HAMPOD PID: $HAMPOD_PID"
    
    # Write PIDs to file for later reference
    echo "$FIRMWARE_PID" > /tmp/hampod_firmware.pid
    echo "$HAMPOD_PID" > /tmp/hampod_software.pid
    
    # Exit successfully (systemd Type=forking expects parent to exit)
    exit 0
}

# ============================================================================
# Main
# ============================================================================

cleanup
start_hampod
