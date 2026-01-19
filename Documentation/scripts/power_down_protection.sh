#!/bin/bash
# ============================================================================
# HAMPOD Power-Down Protection Script
# ============================================================================
# This script manages the overlay filesystem for SD card protection.
# When enabled, the SD card is read-only and all changes go to RAM.
#
# Usage: ./power_down_protection.sh [--enable|--disable|--status]
#
# Options:
#   --enable   Enable overlay filesystem (requires reboot)
#   --disable  Disable overlay filesystem (requires reboot)
#   --status   Show current protection status
#
# ============================================================================

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
BOLD='\033[1m'
NC='\033[0m' # No Color

# Configuration
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
BASHRC_MARKER="# HAMPOD Protection Mode Indicator"

# ============================================================================
# Helper Functions
# ============================================================================

print_header() {
    echo ""
    echo -e "${CYAN}╔══════════════════════════════════════════════════════════════╗${NC}"
    echo -e "${CYAN}║${NC}           HAMPOD Power-Down Protection                       ${CYAN}║${NC}"
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
    echo "  --enable   Enable SD card protection via overlay filesystem"
    echo "  --disable  Disable SD card protection (normal read-write mode)"
    echo "  --status   Show current protection status"
    echo ""
    echo "When protection is enabled:"
    echo "  - SD card is mounted read-only"
    echo "  - All changes go to RAM and are lost on reboot"
    echo "  - Protects against SD card corruption on unexpected power loss"
    echo ""
}

check_root() {
    if [ "$EUID" -ne 0 ]; then
        print_error "This script must be run with sudo"
        echo "  Try: sudo $0 $1"
        exit 1
    fi
}

check_raspi_config() {
    if ! command -v raspi-config &>/dev/null; then
        print_error "raspi-config not found"
        print_info "This script requires Raspberry Pi OS"
        exit 1
    fi
}

# Check if overlay is currently active (running in overlay mode)
is_overlay_active() {
    grep -q " / overlay " /proc/mounts 2>/dev/null
}

# Check if overlay is configured for boot
# raspi-config nonint get_overlay_now outputs: 0 = enabled, 1 = not enabled
is_overlay_configured() {
    local result
    result=$(raspi-config nonint get_overlay_now 2>/dev/null)
    [ "$result" = "0" ]
}

# ============================================================================
# Prompt Indicator Setup
# ============================================================================

setup_prompt_indicator() {
    local user_home="$1"
    local bashrc="${user_home}/.bashrc"
    
    # Check if already configured
    if grep -q "$BASHRC_MARKER" "$bashrc" 2>/dev/null; then
        print_info "Prompt indicator already configured"
        return 0
    fi
    
    # Append prompt indicator to .bashrc
    cat >> "$bashrc" << 'EOF'

# HAMPOD Protection Mode Indicator (color-coded)
hampod_fs_mode() {
    if grep -q " / overlay " /proc/mounts 2>/dev/null; then
        echo -e "\033[0;31m[RO]\033[0m"  # Red - protected, changes lost on reboot
    else
        echo -e "\033[0;32m[RW]\033[0m"  # Green - normal, changes persist
    fi
}
PS1='$(hampod_fs_mode) \u@\h:\w \$ '
EOF
    
    print_success "Added [RO]/[RW] prompt indicator to $bashrc"
}

# Install the persistent write helper
install_persist_helper() {
    local helper_path="/usr/local/bin/hampod_persist_write"
    
    cat > "$helper_path" << 'HELPER_EOF'
#!/bin/bash
# ============================================================================
# HAMPOD Persistent Write Helper
# ============================================================================
# Writes a file persistently, works in overlay or normal mode.
# Usage: hampod_persist_write <source_file> <destination_path>
#
# When overlay is active, this temporarily remounts the lower filesystem
# to write the file, then remounts it read-only.
# ============================================================================

SOURCE="$1"
DEST="$2"

if [ -z "$SOURCE" ] || [ -z "$DEST" ]; then
    echo "Usage: hampod_persist_write <source_file> <destination_path>"
    exit 1
fi

if [ ! -f "$SOURCE" ]; then
    echo "Error: Source file not found: $SOURCE"
    exit 1
fi

if grep -q " / overlay " /proc/mounts 2>/dev/null; then
    # Overlay is active - need to write to lower filesystem
    echo "Overlay mode detected - writing to persistent storage..."
    
    # Get the lower directory from overlay mount
    OVERLAY_OPTS=$(mount | grep "overlay on / " | sed 's/.*(\(.*\))/\1/')
    LOWER_DIR=$(echo "$OVERLAY_OPTS" | tr ',' '\n' | grep "^lowerdir=" | sed 's/lowerdir=//' | cut -d: -f1)
    
    if [ -z "$LOWER_DIR" ]; then
        echo "Error: Could not determine lower filesystem path"
        exit 1
    fi
    
    # Temporarily remount lower filesystem as writable
    mount -o remount,rw "$LOWER_DIR" 2>/dev/null || mount -o remount,rw / 2>/dev/null || {
        echo "Error: Could not remount filesystem as writable"
        exit 1
    }
    
    # Copy the file
    cp "$SOURCE" "${LOWER_DIR}${DEST}" 2>/dev/null || cp "$SOURCE" "$DEST"
    RESULT=$?
    
    # Remount as read-only
    mount -o remount,ro "$LOWER_DIR" 2>/dev/null || mount -o remount,ro / 2>/dev/null || true
    
    if [ $RESULT -eq 0 ]; then
        echo "File written persistently to $DEST"
    else
        echo "Error: Failed to write file"
        exit 1
    fi
else
    # Normal mode - just copy
    cp "$SOURCE" "$DEST"
    if [ $? -eq 0 ]; then
        echo "File written to $DEST"
    else
        echo "Error: Failed to write file"
        exit 1
    fi
fi
HELPER_EOF
    
    chmod +x "$helper_path"
    print_success "Installed persistent write helper: $helper_path"
}

# ============================================================================
# Status Function
# ============================================================================

show_status() {
    print_header
    echo -e "${CYAN}Current Protection Status:${NC}"
    echo ""
    
    if is_overlay_active; then
        echo -e "  ${RED}${BOLD}[RO] PROTECTED MODE${NC}"
        echo ""
        print_success "Overlay filesystem is ACTIVE"
        print_success "SD card is READ-ONLY"
        print_warning "All changes will be LOST on reboot"
        echo ""
        print_info "To disable protection: sudo $0 --disable"
    else
        echo -e "  ${GREEN}${BOLD}[RW] NORMAL MODE${NC}"
        echo ""
        print_success "Normal read-write filesystem"
        print_info "Changes persist across reboots"
        echo ""
        
        # Check if configured for next boot
        if is_overlay_configured; then
            print_warning "Overlay is configured - will be ACTIVE after reboot"
        else
            print_info "To enable protection: sudo $0 --enable"
        fi
    fi
    
    echo ""
    
    # Show mount info
    echo -e "${CYAN}Root Filesystem Mount:${NC}"
    mount | grep " / " | head -1
    echo ""
}

# ============================================================================
# Enable Function
# ============================================================================

enable_protection() {
    check_root "--enable"
    check_raspi_config
    print_header
    
    if is_overlay_active; then
        print_warning "Overlay filesystem is already ACTIVE"
        print_info "SD card protection is already enabled"
        exit 0
    fi
    
    echo -e "${CYAN}Enabling SD Card Protection...${NC}"
    echo ""
    
    # Install the persistent write helper
    install_persist_helper
    
    # Setup prompt indicator for all users with home directories
    for user_home in /home/*; do
        if [ -d "$user_home" ]; then
            setup_prompt_indicator "$user_home"
        fi
    done
    
    # Also for root
    setup_prompt_indicator "/root"
    
    # Enable overlay filesystem
    print_info "Configuring overlay filesystem..."
    raspi-config nonint enable_overlayfs
    print_success "Overlay filesystem enabled"
    
    # Enable boot partition protection
    print_info "Configuring boot partition protection..."
    raspi-config nonint enable_bootro
    print_success "Boot partition protection enabled"
    
    echo ""
    echo -e "${YELLOW}╔══════════════════════════════════════════════════════════════╗${NC}"
    echo -e "${YELLOW}║${NC}                    ${BOLD}REBOOT REQUIRED${NC}                          ${YELLOW}║${NC}"
    echo -e "${YELLOW}╠══════════════════════════════════════════════════════════════╣${NC}"
    echo -e "${YELLOW}║${NC} SD card protection will be active after reboot.             ${YELLOW}║${NC}"
    echo -e "${YELLOW}║${NC}                                                              ${YELLOW}║${NC}"
    echo -e "${YELLOW}║${NC} After reboot:                                                ${YELLOW}║${NC}"
    echo -e "${YELLOW}║${NC}   • Your prompt will show ${RED}[RO]${NC} (read-only)                  ${YELLOW}║${NC}"
    echo -e "${YELLOW}║${NC}   • All file changes will be lost on next reboot            ${YELLOW}║${NC}"
    echo -e "${YELLOW}║${NC}   • SD card is protected from power-loss corruption         ${YELLOW}║${NC}"
    echo -e "${YELLOW}║${NC}                                                              ${YELLOW}║${NC}"
    echo -e "${YELLOW}║${NC} To save config changes permanently, use:                     ${YELLOW}║${NC}"
    echo -e "${YELLOW}║${NC}   hampod_persist_write <source> <destination>                ${YELLOW}║${NC}"
    echo -e "${YELLOW}╚══════════════════════════════════════════════════════════════╝${NC}"
    echo ""
    
    read -p "Reboot now? [y/N] " response
    if [[ "$response" =~ ^[Yy]$ ]]; then
        echo "Rebooting..."
        reboot
    else
        print_info "Please reboot manually to activate protection"
    fi
}

# ============================================================================
# Disable Function
# ============================================================================

disable_protection() {
    check_root "--disable"
    check_raspi_config
    print_header
    
    # Check if overlay is configured (might not be active yet if hasn't rebooted)
    if ! is_overlay_active && ! is_overlay_configured; then
        print_warning "Overlay filesystem is not configured"
        print_info "SD card protection is already disabled"
        exit 0
    fi
    
    echo -e "${CYAN}Disabling SD Card Protection...${NC}"
    echo ""
    
    # Disable boot partition protection first
    print_info "Disabling boot partition protection..."
    raspi-config nonint disable_bootro
    print_success "Boot partition protection disabled"
    
    # Disable overlay filesystem
    print_info "Disabling overlay filesystem..."
    raspi-config nonint disable_overlayfs
    print_success "Overlay filesystem disabled"
    
    echo ""
    echo -e "${YELLOW}╔══════════════════════════════════════════════════════════════╗${NC}"
    echo -e "${YELLOW}║${NC}                    ${BOLD}REBOOT REQUIRED${NC}                          ${YELLOW}║${NC}"
    echo -e "${YELLOW}╠══════════════════════════════════════════════════════════════╣${NC}"
    echo -e "${YELLOW}║${NC} Normal read-write mode will be active after reboot.         ${YELLOW}║${NC}"
    echo -e "${YELLOW}║${NC}                                                              ${YELLOW}║${NC}"
    echo -e "${YELLOW}║${NC} After reboot:                                                ${YELLOW}║${NC}"
    echo -e "${YELLOW}║${NC}   • Your prompt will show ${GREEN}[RW]${NC} (read-write)                 ${YELLOW}║${NC}"
    echo -e "${YELLOW}║${NC}   • All file changes will persist normally                  ${YELLOW}║${NC}"
    echo -e "${YELLOW}║${NC}   • You can install packages and update the system          ${YELLOW}║${NC}"
    echo -e "${YELLOW}╚══════════════════════════════════════════════════════════════╝${NC}"
    echo ""
    
    read -p "Reboot now? [y/N] " response
    if [[ "$response" =~ ^[Yy]$ ]]; then
        echo "Rebooting..."
        reboot
    else
        print_info "Please reboot manually to disable protection"
    fi
}

# ============================================================================
# Main
# ============================================================================

case "${1:-}" in
    --enable|-e)
        enable_protection
        ;;
    --disable|-d)
        disable_protection
        ;;
    --status|-s|"")
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
