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
BOLD='\033[1m'
NC='\033[0m' # No Color

# Configuration
REPO_URL="https://github.com/waynepadgett/HAMPOD2026.git"
HAMPOD_DIR="$HOME/HAMPOD2026"
TOTAL_STEPS=7

# Track current step
CURRENT_STEP=0

# ============================================================================
# Helper Functions
# ============================================================================

print_header() {
    echo ""
    echo -e "${CYAN}╔══════════════════════════════════════════════════════════════╗${NC}"
    echo -e "${CYAN}║${NC}${BOLD}                 HAMPOD Installation Script                   ${NC}${CYAN}║${NC}"
    echo -e "${CYAN}╠══════════════════════════════════════════════════════════════╣${NC}"
    echo -e "${CYAN}║${NC} This script will set up your Raspberry Pi for HAMPOD.        ${CYAN}║${NC}"
    echo -e "${CYAN}║${NC} Estimated time: 5-15 minutes                                 ${CYAN}║${NC}"
    echo -e "${CYAN}╚══════════════════════════════════════════════════════════════╝${NC}"
    echo ""
}

print_step() {
    CURRENT_STEP=$((CURRENT_STEP + 1))
    echo -e "${BLUE}[${CURRENT_STEP}/${TOTAL_STEPS}]${NC} $1"
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

# Set up error handler
trap 'handle_error' ERR

# ============================================================================
# Main Installation Steps
# ============================================================================

main() {
    print_header
    
    # Pre-flight checks
    echo -e "${BOLD}Pre-flight checks...${NC}"
    check_disk_space
    check_internet
    echo ""
    
    # -------------------------------------------------------------------------
    # Step 1: Update System
    # -------------------------------------------------------------------------
    print_step "Updating system packages..."
    sudo apt update -y > /dev/null 2>&1
    print_success "Package lists updated"
    
    sudo apt upgrade -y > /dev/null 2>&1
    print_success "System upgraded"
    
    # -------------------------------------------------------------------------
    # Step 2: Install Dependencies
    # -------------------------------------------------------------------------
    print_step "Installing dependencies (git, gcc, ALSA, Hamlib)..."
    sudo apt install -y git make gcc libasound2-dev libhamlib-dev wget > /dev/null 2>&1
    print_success "All dependencies installed"
    
    # -------------------------------------------------------------------------
    # Step 3: Clone Repository
    # -------------------------------------------------------------------------
    print_step "Setting up HAMPOD repository..."
    
    if [ -d "$HAMPOD_DIR" ]; then
        print_warning "Repository already exists at $HAMPOD_DIR"
        print_info "Pulling latest changes..."
        cd "$HAMPOD_DIR"
        git pull origin main > /dev/null 2>&1 || git pull origin master > /dev/null 2>&1 || true
        print_success "Repository updated"
    else
        print_info "Cloning from $REPO_URL..."
        cd "$HOME"
        git clone "$REPO_URL" > /dev/null 2>&1
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
        print_info "Skipping installation (use --force to reinstall)"
    else
        print_info "Running Piper installer..."
        cd "$HAMPOD_DIR/Documentation/scripts"
        ./install_piper.sh --force
        print_success "Piper TTS installed"
    fi
    
    # -------------------------------------------------------------------------
    # Step 5: Build Firmware
    # -------------------------------------------------------------------------
    print_step "Building Firmware..."
    cd "$HAMPOD_DIR/Firmware"
    
    # Clean first to avoid architecture mismatch
    make clean > /dev/null 2>&1 || true
    print_info "Cleaned old build artifacts"
    
    make > /dev/null 2>&1
    
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
    
    make clean > /dev/null 2>&1 || true
    make > /dev/null 2>&1
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
    # Success!
    # -------------------------------------------------------------------------
    print_success_banner
}

# Run main function
main "$@"
