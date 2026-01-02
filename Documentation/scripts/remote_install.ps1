# =============================================================================
# DEPRECATED - DO NOT USE FOR GUIDANCE
# =============================================================================
# This script contains outdated SSH connection information.
# The correct SSH target is: hampod@hampod.local (not waynesr@HAMPOD.local)
# Use Regression_Phase_One_Manual_Radio_Test.sh for manual testing instead.
# =============================================================================

# Remote Install Script for Windows (PowerShell)
# Usage: .\remote_install.ps1 "<commit_message>"

param(
    [Parameter(Mandatory = $true)]
    [string]$CommitMessage
)

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Step 1: Committing and Pushing local changes..." -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan

# Add all changes
git add .

# Commit with the provided message
git commit -m $CommitMessage

# Push to the default remote/branch (usually origin main)
git push

# Check if push was successful
if ($LASTEXITCODE -ne 0) {
    Write-Host "Error: Git push failed. Aborting." -ForegroundColor Red
    exit 1
}

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Step 2: Building Firmware and Software on remote Pi..." -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan

# SSH into the remote machine, pull changes, build Firmware first then Software
ssh waynesr@HAMPOD.local "cd ~/HAMPOD2026 && echo 'Pulling changes...' && git pull && echo 'Building Firmware...' && cd Firmware && make && echo 'Building Software...' && cd ../Software && make"

if ($LASTEXITCODE -ne 0) {
    Write-Host "Error: Remote build failed." -ForegroundColor Red
    exit 1
}

Write-Host "========================================" -ForegroundColor Green
Write-Host "Remote install process complete." -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Green
