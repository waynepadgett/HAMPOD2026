$REMOTE_USER = "waynesr"
$REMOTE_HOST = "HAMPOD.local"
$REMOTE_DIR = "~/HAMPOD2026/Firmware"
$LOCAL_DIR = Get-Location

# 1. Sync Firmware
Write-Host "Syncing Firmware files to $REMOTE_HOST..."
scp -r "$LOCAL_DIR/Firmware" "$REMOTE_USER@$REMOTE_HOST`:~/HAMPOD2026/"

# 2. Sync Test Script
Write-Host "Syncing test script..."
scp "$LOCAL_DIR/Documentation/scripts/run_remote_test.sh" "$REMOTE_USER@$REMOTE_HOST`:~/HAMPOD2026/run_remote_test.sh"

# 3. Run Test
Write-Host "Running remote test..."
# Convert DOS line endings to Unix just in case, chmod, and run
ssh $REMOTE_USER@$REMOTE_HOST "sed -i 's/\r$//' ~/HAMPOD2026/run_remote_test.sh && chmod +x ~/HAMPOD2026/run_remote_test.sh && ~/HAMPOD2026/run_remote_test.sh"
