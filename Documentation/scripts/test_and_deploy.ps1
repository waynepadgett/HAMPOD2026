$REMOTE_USER = "waynesr"
$REMOTE_HOST = "HAMPOD.local"
$REMOTE_DIR = "~/HAMPOD2026"
$LOCAL_DIR = Get-Location

Write-Host "Syncing files to $REMOTE_HOST..."

# Create remote directory if it doesn't exist
ssh $REMOTE_USER@$REMOTE_HOST "mkdir -p $REMOTE_DIR"

# Use scp to copy Firmware, Software, and Speech_Comparison directories
# We exclude .git and bulky Documentation to save time, unless needed.
# For now, let's copy the specific source directories.

Write-Host "Copying Firmware..."
scp -r "$LOCAL_DIR/Firmware" "$REMOTE_USER@$REMOTE_HOST`:$REMOTE_DIR/"

Write-Host "Copying Software..."
scp -r "$LOCAL_DIR/Software" "$REMOTE_USER@$REMOTE_HOST`:$REMOTE_DIR/"

Write-Host "Copying Software2..."
scp -r "$LOCAL_DIR/Software2" "$REMOTE_USER@$REMOTE_HOST`:$REMOTE_DIR/"

Write-Host "Copying Speech_Comparison..."
scp -r "$LOCAL_DIR/Speech_Comparison" "$REMOTE_USER@$REMOTE_HOST`:$REMOTE_DIR/"

# Run Make (Test)
Write-Host "Building Firmware..."
ssh $REMOTE_USER@$REMOTE_HOST "cd $REMOTE_DIR/Firmware && make clean && make"

if ($LASTEXITCODE -eq 0) {
    Write-Host "Firmware build successful."
}
else {
    Write-Host "Firmware build FAILED."
    exit 1
}

Write-Host "Building Software..."
ssh $REMOTE_USER@$REMOTE_HOST "cd $REMOTE_DIR/Software && make clean && make"

if ($LASTEXITCODE -eq 0) {
    Write-Host "Software build successful."
}
else {
    Write-Host "Software build FAILED (Legacy - Expected)."
}

Write-Host "Building Software2 (New Phase 0)..."
ssh $REMOTE_USER@$REMOTE_HOST "cd $REMOTE_DIR/Software2 && make clean && make"

if ($LASTEXITCODE -eq 0) {
    Write-Host "Software2 build successful."
}
else {
    Write-Host "Software2 build FAILED."
    exit 1
}

Write-Host "Building Speech Comparison..."
ssh $REMOTE_USER@$REMOTE_HOST "cd $REMOTE_DIR/Speech_Comparison && make clean && make"

if ($LASTEXITCODE -eq 0) {
    Write-Host "Speech Comparison build successful."
}
else {
    Write-Host "Speech Comparison build FAILED."
    exit 1
}

Write-Host "All builds passed!"
