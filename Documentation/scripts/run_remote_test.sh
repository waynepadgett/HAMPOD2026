#!/bin/bash
cd ~/HAMPOD2026/Firmware || exit 1

echo '--- Killing Stale Processes ---'
killall -9 firmware.elf imitation_software 2>/dev/null

echo '--- Cleaning Build ---'
make clean

echo '--- Building ---'
make firmware.elf imitation_software
if [ $? -ne 0 ]; then
    echo 'Build Failed'
    exit 1
fi

echo '--- Cleaning Pipes ---'
rm -f Firmware_i Firmware_o Keypad_i Keypad_o Speaker_i Speaker_o

echo '--- Starting Firmware ---'
# Start firmware in background, redirected to log
nohup ./firmware.elf > firmware.log 2>&1 &
FIRM_PID=$!

echo "FIRM_PID=$FIRM_PID"
echo '--- Waiting for Firmware Init (3s) ---'
sleep 3

# Firmware blocks on opening Firmware_o until a reader connects.
# So we only check if the PIPE file exists.
if [ ! -p Firmware_o ]; then
    echo 'ERROR: Firmware_o pipe not found. Firmware failed to start?'
    echo '--- Firmware Log ---'
    cat firmware.log
    kill $FIRM_PID 2>/dev/null
    exit 1
fi

echo 'FIRMWARE STATUS: Running and waiting for connection (as expected).'

echo '--- Running Imitation Software (Timeout 15s) ---'
timeout 15s ./imitation_software
TEST_EXIT=$?

echo '--- Killing Firmware ---'
kill $FIRM_PID 2>/dev/null

echo '--- Firmware Log ---'
cat firmware.log

if [ $TEST_EXIT -eq 0 ]; then
    echo 'TEST MARKER: SUCCESS'
else
    echo "TEST MARKER: FAILURE (Exit code $TEST_EXIT)"
fi
