#!/bin/bash
# ============================================================================
# HAMPOD Raspberry Pi Installation Script
# ============================================================================
# This script automates the complete setup of a Raspberry Pi for HAMPOD.
# Run this after SSH'ing into your Pi for the first time.
#
# Usage: ./install_hampod.sh
#
# Prerequisites:
#   - Raspberry Pi with Debian Trixie installed
#   - SSH access enabled
#   - Internet connection
#
# What this script does:
#   1. Updates system packages
#   2. Installs dependencies (git, gcc, ALSA, Hamlib)
#   3. Clones the HAMPOD repository
#   4. Installs Piper TTS
#   5. Builds the Firmware
#   6. Builds HAL Integration Tests
#   7. Adds user to audio group
#
# After completion, run the first regression test to verify your setup.
# ============================================================================

set -e  # Exit on any error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
MAGENTA='\033[0;35m'
BOLD='\033[1m'
DIM='\033[2m'
NC='\033[0m' # No Color

# Configuration
REPO_URL="https://github.com/waynepadgett/HAMPOD2026.git"
HAMPOD_DIR="$HOME/HAMPOD2026"
TOTAL_STEPS=10

# Track current step
CURRENT_STEP=0

# Spinner characters
SPINNER_CHARS='⠋⠙⠹⠸⠼⠴⠦⠧⠇⠏'

# PID of background spinner process
SPINNER_PID=""

# ============================================================================
# Helper Functions
# ============================================================================

print_header() {
    echo ""
    echo -e "${CYAN}╔══════════════════════════════════════════════════════════════╗${NC}"
    echo -e "${CYAN}║${NC}${BOLD}                 HAMPOD Installation Script                   ${NC}${CYAN}║${NC}"
    echo -e "${CYAN}╠══════════════════════════════════════════════════════════════╣${NC}"
    echo -e "${CYAN}║${NC} This script will set up your Raspberry Pi for HAMPOD.        ${CYAN}║${NC}"
    echo -e "${CYAN}║${NC} Estimated time: 5-15 minutes (depends on internet speed)     ${CYAN}║${NC}"
    echo -e "${CYAN}╚══════════════════════════════════════════════════════════════╝${NC}"
    echo ""
}

# Progress bar: draws [████████░░░░░░░░░░░░] 40%
draw_progress_bar() {
    local current=$1
    local total=$2
    local width=30
    local percent=$((current * 100 / total))
    local filled=$((current * width / total))
    local empty=$((width - filled))
    
    # Build the bar
    local bar=""
    for ((i=0; i<filled; i++)); do bar+="█"; done
    for ((i=0; i<empty; i++)); do bar+="░"; done
    
    echo -e "${DIM}[${NC}${GREEN}${bar:0:$filled}${NC}${DIM}${bar:$filled}${NC}${DIM}]${NC} ${BOLD}${percent}%${NC}"
}

# Start a spinner in the background
start_spinner() {
    local message="$1"
    
    # Don't start if already running
    if [ -n "$SPINNER_PID" ] && kill -0 "$SPINNER_PID" 2>/dev/null; then
        return
    fi
    
    (
        local i=0
        local len=${#SPINNER_CHARS}
        while true; do
            local char="${SPINNER_CHARS:$i:1}"
            printf "\r      ${MAGENTA}%s${NC} %s" "$char" "$message"
            i=$(( (i + 1) % len ))
            sleep 0.1
        done
    ) &
    SPINNER_PID=$!
    disown $SPINNER_PID 2>/dev/null || true
}

# Stop the spinner
stop_spinner() {
    if [ -n "$SPINNER_PID" ]; then
        kill $SPINNER_PID 2>/dev/null || true
        wait $SPINNER_PID 2>/dev/null || true
        SPINNER_PID=""
        printf "\r\033[K"  # Clear the line
    fi
}

# Run a command with a spinner
run_with_spinner() {
    local message="$1"
    shift
    
    start_spinner "$message"
    
    # Run the command
    if "$@" > /dev/null 2>&1; then
        stop_spinner
        return 0
    else
        stop_spinner
        return 1
    fi
}

print_step() {
    CURRENT_STEP=$((CURRENT_STEP + 1))
    echo ""
    echo -e "$(draw_progress_bar $CURRENT_STEP $TOTAL_STEPS)"
    echo -e "${BLUE}[${CURRENT_STEP}/${TOTAL_STEPS}]${NC} ${BOLD}$1${NC}"
}

print_success() {
    echo -e "      ${GREEN}✓${NC} $1"
}

print_warning() {
    echo -e "      ${YELLOW}⚠${NC} $1"
}

print_error() {
    echo -e "      ${RED}✗${NC} $1"
}

print_info() {
    echo -e "      ${CYAN}→${NC} $1"
}

check_disk_space() {
    # Need at least 2GB free
    AVAILABLE_KB=$(df "$HOME" | awk 'NR==2 {print $4}')
    REQUIRED_KB=2097152  # 2GB in KB
    
    if [ "$AVAILABLE_KB" -lt "$REQUIRED_KB" ]; then
        print_error "Insufficient disk space. Need at least 2GB free."
        print_info "Available: $((AVAILABLE_KB / 1024)) MB"
        exit 1
    fi
    print_success "Disk space: $((AVAILABLE_KB / 1024)) MB available"
}

check_internet() {
    if ! ping -c 1 github.com &> /dev/null; then
        print_error "No internet connection. Please check your network."
        exit 1
    fi
    print_success "Internet connection verified"
}

# Test internet speed by downloading a small file
test_internet_speed() {
    print_info "Testing download speed..."
    
    # Download a small file and measure time (GitHub's robots.txt is tiny and reliable)
    local test_url="https://raw.githubusercontent.com/waynepadgett/HAMPOD2026/main/README.md"
    local start_time=$(date +%s.%N)
    
    # Download to /dev/null and capture size
    local result=$(curl -s -w "%{size_download} %{time_total}" -o /dev/null "$test_url" 2>/dev/null)
    local size=$(echo "$result" | awk '{print $1}')
    local time=$(echo "$result" | awk '{print $2}')
    
    if [ -n "$size" ] && [ -n "$time" ] && [ "$time" != "0" ] && [ "$time" != "0.000000" ]; then
        # Calculate speed in KB/s
        local speed=$(echo "scale=0; $size / $time / 1024" | bc 2>/dev/null || echo "0")
        
        if [ "$speed" -gt 0 ]; then
            if [ "$speed" -gt 1024 ]; then
                local speed_mb=$(echo "scale=1; $speed / 1024" | bc 2>/dev/null || echo "$speed")
                print_success "Download speed: ~${speed_mb} MB/s"
            else
                print_success "Download speed: ~${speed} KB/s"
            fi
            
            # Estimate based on speed
            if [ "$speed" -lt 100 ]; then
                print_warning "Slow connection detected - installation may take 15-30 minutes"
            elif [ "$speed" -lt 500 ]; then
                print_info "Moderate speed - installation should take 10-15 minutes"
            else
                print_info "Good speed - installation should take 5-10 minutes"
            fi
        else
            print_info "Could not measure speed (will proceed anyway)"
        fi
    else
        print_info "Could not measure speed (will proceed anyway)"
    fi
}

print_success_banner() {
    echo ""
    echo -e "${GREEN}╔══════════════════════════════════════════════════════════════╗${NC}"
    echo -e "${GREEN}║${NC}${BOLD}                   🎉 SETUP SUCCESSFUL! 🎉                    ${NC}${GREEN}║${NC}"
    echo -e "${GREEN}╠══════════════════════════════════════════════════════════════╣${NC}"
    echo -e "${GREEN}║${NC} Your Raspberry Pi is now configured for HAMPOD development. ${GREEN}║${NC}"
    echo -e "${GREEN}║${NC}                                                              ${GREEN}║${NC}"
    echo -e "${GREEN}║${NC}${BOLD} NEXT STEP:${NC} Run the first regression test to verify setup:  ${GREEN}║${NC}"
    echo -e "${GREEN}║${NC}                                                              ${GREEN}║${NC}"
    echo -e "${GREEN}║${NC}   ${CYAN}cd ~/HAMPOD2026/Documentation/scripts${NC}                     ${GREEN}║${NC}"
    echo -e "${GREEN}║${NC}   ${CYAN}./Regression_Phase0_Integration.sh${NC}                        ${GREEN}║${NC}"
    echo -e "${GREEN}║${NC}                                                              ${GREEN}║${NC}"
    echo -e "${GREEN}║${NC}${BOLD} WHAT TO EXPECT:${NC}                                             ${GREEN}║${NC}"
    echo -e "${GREEN}║${NC}   • System announces \"System Ready\"                          ${GREEN}║${NC}"
    echo -e "${GREEN}║${NC}   • Pressing keys announces \"You pressed [Key Name]\"         ${GREEN}║${NC}"
    echo -e "${GREEN}║${NC}   • Holding keys announces \"You held [Key Name]\"             ${GREEN}║${NC}"
    echo -e "${GREEN}║${NC}   • Press Ctrl+C to exit                                     ${GREEN}║${NC}"
    echo -e "${GREEN}║${NC}                                                              ${GREEN}║${NC}"
    echo -e "${GREEN}║${NC} If you only hear clicks (no speech), see troubleshooting in ${GREEN}║${NC}"
    echo -e "${GREEN}║${NC} Documentation/Project_Overview_and_Onboarding/              ${GREEN}║${NC}"
    echo -e "${GREEN}║${NC} RPi_Setup_Guide.md                                          ${GREEN}║${NC}"
    echo -e "${GREEN}╚══════════════════════════════════════════════════════════════╝${NC}"
    echo ""
}

handle_error() {
    stop_spinner  # Make sure spinner is stopped
    echo ""
    echo -e "${RED}╔══════════════════════════════════════════════════════════════╗${NC}"
    echo -e "${RED}║${NC}${BOLD}                     ❌ INSTALLATION FAILED                    ${NC}${RED}║${NC}"
    echo -e "${RED}╠══════════════════════════════════════════════════════════════╣${NC}"
    echo -e "${RED}║${NC} Step ${CURRENT_STEP}/${TOTAL_STEPS} failed. See error message above.                   ${RED}║${NC}"
    echo -e "${RED}║${NC}                                                              ${RED}║${NC}"
    echo -e "${RED}║${NC} Try these steps:                                             ${RED}║${NC}"
    echo -e "${RED}║${NC}   1. Check internet: ping -c 3 google.com                    ${RED}║${NC}"
    echo -e "${RED}║${NC}   2. Re-run this script: ./install_hampod.sh                 ${RED}║${NC}"
    echo -e "${RED}║${NC}   3. See RPi_Setup_Guide.md for manual instructions          ${RED}║${NC}"
    echo -e "${RED}╚══════════════════════════════════════════════════════════════╝${NC}"
    echo ""
    exit 1
}

# Clean up spinner on exit
cleanup() {
    stop_spinner
}

# Set up handlers
trap 'handle_error' ERR
trap 'cleanup' EXIT

# ============================================================================
# Main Installation Steps
# ============================================================================

main() {
    print_header
    
    # Pre-flight checks
    echo -e "${BOLD}Pre-flight checks...${NC}"
    check_disk_space
    check_internet
    test_internet_speed
    echo ""
    
    # -------------------------------------------------------------------------
    # Step 1: Update System
    # -------------------------------------------------------------------------
    print_step "Updating system packages..."
    
    run_with_spinner "Updating package lists..." sudo apt update -y
    print_success "Package lists updated"
    
    run_with_spinner "Upgrading packages (this may take a few minutes)..." sudo env DEBIAN_FRONTEND=noninteractive apt upgrade -y -o Dpkg::Options::="--force-confdef" -o Dpkg::Options::="--force-confold"
    print_success "System upgraded"
    
    # -------------------------------------------------------------------------
    # Step 2: Install Dependencies
    # -------------------------------------------------------------------------
    print_step "Installing dependencies (git, gcc, ALSA, Hamlib)..."
    
    run_with_spinner "Installing build tools and libraries..." sudo env DEBIAN_FRONTEND=noninteractive apt install -y git make gcc libasound2-dev libhamlib-dev wget bc
    print_success "All dependencies installed"
    
    # -------------------------------------------------------------------------
    # Step 3: Clone Repository
    # -------------------------------------------------------------------------
    print_step "Setting up HAMPOD repository..."
    
    if [ -d "$HAMPOD_DIR" ]; then
        print_warning "Repository already exists at $HAMPOD_DIR"
        print_info "Pulling latest changes..."
        cd "$HAMPOD_DIR"
        run_with_spinner "Pulling updates..." git pull origin main || git pull origin master || true
        print_success "Repository updated"
    else
        print_info "Cloning from $REPO_URL..."
        cd "$HOME"
        run_with_spinner "Cloning repository..." git clone "$REPO_URL"
        print_success "Repository cloned to $HAMPOD_DIR"
    fi
    
    # -------------------------------------------------------------------------
    # Step 4: Install Piper TTS
    # -------------------------------------------------------------------------
    print_step "Installing Piper TTS..."
    
    # Make scripts executable
    chmod +x "$HAMPOD_DIR/Documentation/scripts/"*.sh 2>/dev/null || true
    
    # Check if Piper is already installed
    if command -v piper &> /dev/null && [ -f "$HAMPOD_DIR/Firmware/models/en_US-lessac-low.onnx" ]; then
        print_warning "Piper TTS already installed"
        print_info "Skipping installation (use --force in install_piper.sh to reinstall)"
    else
        print_info "Running Piper installer (downloading ~20MB)..."
        cd "$HAMPOD_DIR/Documentation/scripts"
        
        # Run piper installer - it has its own output, don't suppress
        ./install_piper.sh --force
        
        print_success "Piper TTS installed"
    fi
    
    # -------------------------------------------------------------------------
    # Step 5: Build Firmware
    # -------------------------------------------------------------------------
    print_step "Building Firmware..."
    cd "$HAMPOD_DIR/Firmware"
    
    # Clean first to avoid architecture mismatch
    run_with_spinner "Cleaning old build artifacts..." make clean || true
    print_info "Cleaned old build artifacts"
    
    run_with_spinner "Compiling firmware..." make
    
    if [ -f "firmware.elf" ]; then
        print_success "Firmware built successfully (firmware.elf)"
    else
        print_error "Firmware build may have issues - firmware.elf not found"
    fi
    
    # -------------------------------------------------------------------------
    # Step 6: Build HAL Integration Tests
    # -------------------------------------------------------------------------
    print_step "Building HAL Integration Tests..."
    cd "$HAMPOD_DIR/Firmware/hal/tests"
    
    run_with_spinner "Cleaning test artifacts..." make clean || true
    run_with_spinner "Compiling tests..." make
    print_success "HAL Integration Tests built"
    
    # -------------------------------------------------------------------------
    # Step 7: Add User to Audio Group
    # -------------------------------------------------------------------------
    print_step "Configuring audio permissions..."
    
    # Check if user is already in audio group
    if groups | grep -q '\baudio\b'; then
        print_success "User already in audio group"
    else
        sudo usermod -aG audio "$USER"
        print_success "Added $USER to audio group"
        print_warning "Note: Log out and back in for group changes to take full effect"
    fi
    
    # -------------------------------------------------------------------------
    # Step 8: Configure Auto-Start
    # -------------------------------------------------------------------------
    print_step "Configuring HAMPOD auto-start on boot..."
    
    # Make scripts executable
    chmod +x "$HAMPOD_DIR/Documentation/scripts/hampod_on_powerup.sh" 2>/dev/null || true
    chmod +x "$HAMPOD_DIR/Documentation/scripts/run_hampod_service.sh" 2>/dev/null || true
    chmod +x "$HAMPOD_DIR/Documentation/scripts/power_down_protection.sh" 2>/dev/null || true
    
    "$HAMPOD_DIR/Documentation/scripts/hampod_on_powerup.sh" --enable
    print_success "HAMPOD will start automatically on boot"
    
    # -------------------------------------------------------------------------
    # Step 9: SD Card Protection (Optional)
    # -------------------------------------------------------------------------
    print_step "SD Card Protection Setup..."
    
    echo ""
    echo -e "${CYAN}SD Card Protection${NC} helps prevent filesystem corruption"
    echo "if the Raspberry Pi loses power unexpectedly."
    echo ""
    echo "When enabled:"
    echo "  • SD card is read-only (protected)"
    echo "  • All changes go to RAM and are lost on reboot"
    echo "  • Your prompt will show [RO] in red"
    echo ""
    echo "When disabled:"
    echo "  • Normal read-write operation"
    echo "  • Your prompt will show [RW] in green"
    echo ""
    echo -e "${YELLOW}Recommendation:${NC}"
    echo -e "  • ${GREEN}End users${NC} should enable this (answer Y)"
    echo -e "  • ${CYAN}Developers${NC} should skip this (answer N) - you can enable it later"
    echo ""
    
    read -p "Enable SD card protection now? [y/N] " response
    if [[ "$response" =~ ^[Yy]$ ]]; then
        "$HAMPOD_DIR/Documentation/scripts/power_down_protection.sh" --enable
    else
        print_info "SD card protection not enabled"
        print_info "You can enable it later with: ./power_down_protection.sh --enable"
        
        # Still install the prompt indicator for when they do enable it
        "$HAMPOD_DIR/Documentation/scripts/power_down_protection.sh" --status > /dev/null 2>&1 || true
    fi
    
    # -------------------------------------------------------------------------
    # Step 10: Performance Settings
    # -------------------------------------------------------------------------
    print_step "Configuring CPU Performance Mode..."
    
    echo ""
    print_info "Creating systemd service to pin CPU governor to 'performance'..."
    sudo bash -c 'cat > /etc/systemd/system/hampod-performance.service << EOF
[Unit]
Description=Set CPU Governor to Performance
After=multi-user.target

[Service]
Type=oneshot
ExecStart=/bin/sh -c "echo performance | tee /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor"
RemainAfterExit=yes

[Install]
WantedBy=multi-user.target
EOF'
    sudo systemctl daemon-reload
    sudo systemctl enable --now hampod-performance
    print_success "CPU pinned to maximum performance"
    
    # -------------------------------------------------------------------------
    # Success!
    # -------------------------------------------------------------------------
    print_success_banner
}

# Run main function
main "$@"
