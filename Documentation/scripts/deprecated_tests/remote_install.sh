#!/bin/bash

# Remote Install Script
# Usage: ./remote_install.sh "<commit_message>"

# Check if a commit message was provided
if [ -z "$1" ]; then
  echo "Error: No commit message provided."
  echo "Usage: ./remote_install.sh \"<commit_message>\""
  exit 1
fi

COMMIT_MESSAGE="$1"

echo "========================================"
echo "Step 1: Committing and Pushing local changes..."
echo "========================================"

# Add all changes
git add .

# Commit with the provided message
git commit -m "$COMMIT_MESSAGE"

# Push to the default remote/branch (usually origin main)
git push

# Check if push was successful
if [ $? -ne 0 ]; then
  echo "Error: Git push failed. Aborting."
  exit 1
fi

echo "========================================"
echo "Step 2: Connecting to remote to Pull and Make..."
echo "========================================"

# SSH into the remote machine, pull changes, and run make in the Software directory
ssh waynesr@HAMPOD.local "cd ~/HAMPOD2026/Software && echo 'Pulling changes...' && git pull && echo 'Running make...' && make"

echo "========================================"
echo "Remote install process complete."
echo "========================================"
