#!/bin/bash
# ============================================================================
# HAMPOD Auto-Start on Power-Up Script
# ============================================================================
# This script manages the systemd service for auto-starting HAMPOD on boot.
#
# Usage: ./hampod_on_powerup.sh [--enable|--disable|--status]
#
# Options:
#   --enable   Install and enable the systemd service (default)
#   --disable  Disable and remove the systemd service
#   --status   Show current auto-start status
#
# ============================================================================

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# Configuration
SERVICE_NAME="hampod"
SERVICE_FILE="/etc/systemd/system/${SERVICE_NAME}.service"
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
HAMPOD_USER="${SUDO_USER:-$(whoami)}"
HAMPOD_HOME="/home/${HAMPOD_USER}"

# ============================================================================
# Helper Functions
# ============================================================================

print_header() {
    echo ""
    echo -e "${CYAN}╔══════════════════════════════════════════════════════════════╗${NC}"
    echo -e "${CYAN}║${NC}           HAMPOD Auto-Start Configuration                    ${CYAN}║${NC}"
    echo -e "${CYAN}╚══════════════════════════════════════════════════════════════╝${NC}"
    echo ""
}

print_success() {
    echo -e "  ${GREEN}✓${NC} $1"
}

print_warning() {
    echo -e "  ${YELLOW}⚠${NC} $1"
}

print_error() {
    echo -e "  ${RED}✗${NC} $1"
}

print_info() {
    echo -e "  ${CYAN}→${NC} $1"
}

show_help() {
    echo "Usage: $0 [--enable|--disable|--status]"
    echo ""
    echo "Options:"
    echo "  --enable   Install and enable HAMPOD auto-start on boot (default)"
    echo "  --disable  Disable and remove HAMPOD auto-start"
    echo "  --status   Show current auto-start status"
    echo ""
}

check_root() {
    if [ "$EUID" -ne 0 ]; then
        print_error "This script must be run with sudo"
        echo "  Try: sudo $0 $1"
        exit 1
    fi
}

# ============================================================================
# Status Function
# ============================================================================

show_status() {
    print_header
    echo -e "${CYAN}Current Status:${NC}"
    echo ""
    
    if [ -f "$SERVICE_FILE" ]; then
        print_success "Service file exists: $SERVICE_FILE"
        
        if systemctl is-enabled "$SERVICE_NAME" &>/dev/null; then
            print_success "Auto-start is ENABLED"
        else
            print_warning "Service exists but is DISABLED"
        fi
        
        if systemctl is-active "$SERVICE_NAME" &>/dev/null; then
            print_success "HAMPOD is currently RUNNING"
        else
            print_info "HAMPOD is not currently running"
        fi
        
        echo ""
        echo -e "${CYAN}Service Details:${NC}"
        systemctl status "$SERVICE_NAME" --no-pager 2>/dev/null || echo "  (service not active)"
    else
        print_info "Auto-start is NOT CONFIGURED"
        print_info "Run '$0 --enable' to enable auto-start"
    fi
    echo ""
}

# ============================================================================
# Enable Function
# ============================================================================

enable_autostart() {
    check_root "--enable"
    print_header
    echo -e "${CYAN}Enabling HAMPOD Auto-Start...${NC}"
    echo ""
    
    # Check that required binaries exist, offer to build if missing
    FIRMWARE_BIN="$REPO_ROOT/Firmware/firmware.elf"
    SOFTWARE_BIN="$REPO_ROOT/Software2/bin/hampod"
    NEEDS_BUILD=false
    
    if [ ! -f "$FIRMWARE_BIN" ]; then
        print_warning "Firmware binary not found: firmware.elf"
        NEEDS_BUILD=true
    else
        print_success "Firmware binary found: firmware.elf"
    fi
    
    if [ ! -f "$SOFTWARE_BIN" ]; then
        print_warning "Software binary not found: hampod"
        NEEDS_BUILD=true
    else
        print_success "Software binary found: hampod"
    fi
    
    if [ "$NEEDS_BUILD" = true ]; then
        echo ""
        read -p "Build missing binaries now? [Y/n] " response
        if [[ ! "$response" =~ ^[Nn]$ ]]; then
            echo ""
            print_info "Building Firmware..."
            cd "$REPO_ROOT/Firmware"
            make clean > /dev/null 2>&1 || true
            if make; then
                print_success "Firmware built successfully"
            else
                print_error "Firmware build failed"
                exit 1
            fi
            
            echo ""
            print_info "Building Software2..."
            cd "$REPO_ROOT/Software2"
            make clean > /dev/null 2>&1 || true
            if make; then
                print_success "Software2 built successfully"
            else
                print_error "Software2 build failed"
                exit 1
            fi
            echo ""
        else
            print_error "Cannot enable auto-start without binaries"
            print_info "Build manually and re-run this script"
            exit 1
        fi
    fi
    
    # Create the service wrapper script
    WRAPPER_SCRIPT="$SCRIPT_DIR/run_hampod_service.sh"
    
    if [ ! -f "$WRAPPER_SCRIPT" ]; then
        print_error "Wrapper script not found: $WRAPPER_SCRIPT"
        print_info "Please ensure run_hampod_service.sh exists in the scripts directory"
        exit 1
    fi
    
    # Make wrapper executable
    chmod +x "$WRAPPER_SCRIPT"
    print_success "Made wrapper script executable"
    
    # Create systemd service file
    cat > "$SERVICE_FILE" << EOF
[Unit]
Description=HAMPOD Ham Radio Accessibility System
After=multi-user.target sound.target
Wants=sound.target

[Service]
Type=forking
User=root
WorkingDirectory=${REPO_ROOT}/Firmware
ExecStartPre=/bin/sleep 3
ExecStart=${WRAPPER_SCRIPT}
ExecStop=/bin/bash -c 'killall -9 firmware.elf hampod 2>/dev/null || true'
Restart=on-failure
RestartSec=5
StandardOutput=journal
StandardError=journal

[Install]
WantedBy=multi-user.target
EOF
    
    print_success "Created service file: $SERVICE_FILE"
    
    # Reload systemd
    systemctl daemon-reload
    print_success "Reloaded systemd configuration"
    
    # Enable service
    systemctl enable "$SERVICE_NAME"
    print_success "Enabled $SERVICE_NAME service"
    
    echo ""
    echo -e "${GREEN}╔══════════════════════════════════════════════════════════════╗${NC}"
    echo -e "${GREEN}║${NC}              Auto-Start Configuration Complete              ${GREEN}║${NC}"
    echo -e "${GREEN}╠══════════════════════════════════════════════════════════════╣${NC}"
    echo -e "${GREEN}║${NC} HAMPOD will now start automatically on boot.                ${GREEN}║${NC}"
    echo -e "${GREEN}║${NC}                                                              ${GREEN}║${NC}"
    echo -e "${GREEN}║${NC} To start immediately without rebooting:                      ${GREEN}║${NC}"
    echo -e "${GREEN}║${NC}   sudo systemctl start hampod                                ${GREEN}║${NC}"
    echo -e "${GREEN}║${NC}                                                              ${GREEN}║${NC}"
    echo -e "${GREEN}║${NC} To check status:                                             ${GREEN}║${NC}"
    echo -e "${GREEN}║${NC}   sudo systemctl status hampod                               ${GREEN}║${NC}"
    echo -e "${GREEN}║${NC}                                                              ${GREEN}║${NC}"
    echo -e "${GREEN}║${NC} To view logs:                                                ${GREEN}║${NC}"
    echo -e "${GREEN}║${NC}   sudo journalctl -u hampod -f                               ${GREEN}║${NC}"
    echo -e "${GREEN}╚══════════════════════════════════════════════════════════════╝${NC}"
    echo ""
}

# ============================================================================
# Disable Function
# ============================================================================

disable_autostart() {
    check_root "--disable"
    print_header
    echo -e "${CYAN}Disabling HAMPOD Auto-Start...${NC}"
    echo ""
    
    if [ ! -f "$SERVICE_FILE" ]; then
        print_warning "Service is not currently configured"
        print_info "Nothing to disable"
        exit 0
    fi
    
    # Stop service if running
    if systemctl is-active "$SERVICE_NAME" &>/dev/null; then
        systemctl stop "$SERVICE_NAME"
        print_success "Stopped $SERVICE_NAME service"
    fi
    
    # Disable service
    if systemctl is-enabled "$SERVICE_NAME" &>/dev/null; then
        systemctl disable "$SERVICE_NAME"
        print_success "Disabled $SERVICE_NAME service"
    fi
    
    # Remove service file
    rm -f "$SERVICE_FILE"
    print_success "Removed service file"
    
    # Reload systemd
    systemctl daemon-reload
    print_success "Reloaded systemd configuration"
    
    echo ""
    echo -e "${GREEN}Auto-start has been disabled.${NC}"
    echo -e "${CYAN}HAMPOD will no longer start automatically on boot.${NC}"
    echo ""
}

# ============================================================================
# Main
# ============================================================================

case "${1:-}" in
    --enable|-e|"")
        enable_autostart
        ;;
    --disable|-d)
        disable_autostart
        ;;
    --status|-s)
        show_status
        ;;
    --help|-h)
        show_help
        ;;
    *)
        print_error "Unknown option: $1"
        show_help
        exit 1
        ;;
esac
