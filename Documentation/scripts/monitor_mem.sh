#!/bin/bash
# HAMPOD2026 Memory Monitoring Script
# Logs memory usage of firmware.elf and hampod every 5 minutes (300 seconds)

INTERVAL=300

echo "=========================================================="
echo " HAMPOD Memory Monitor "
echo " Logging every $INTERVAL seconds. Press Ctrl+C to stop."
echo "=========================================================="
echo ""

# Print header
printf "%-10s | %-15s | %-8s | %-8s | %-10s | %-10s\n" "TIME" "PROCESS" "PID" "CPU%" "MEM%" "RES(KB)"
echo "--------------------------------------------------------------------------------"

while true; do
    TIME=$(date "+%H:%M:%S")
    
    echo "--------------------------------------------------------------------------------"
    
    # Get total system memory (in MB, convert to KB approximately for consistency in table)
    SYS_TOTAL=$(free -m | grep Mem | awk '{print $2}')
    SYS_USED=$(free -m | grep Mem | awk '{print $3}')
    SYS_PCT=$(( 100 * SYS_USED / SYS_TOTAL ))
    printf "%-10s | %-15s | %-8s | %-8s | %-10s | %-10s\n" "$TIME" "[SYSTEM TOTAL]" "---" "---" "$SYS_PCT%" "${SYS_USED}MB / ${SYS_TOTAL}MB"
    
    # Get stats for processes, removing the header line
    ps -C firmware.elf,hampod,piper -o comm=,pid=,pcpu=,pmem=,rss= | while read -r P_NAME P_PID P_CPU P_MEM P_RSS; do
        if [ ! -z "$P_NAME" ]; then
            printf "%-10s | %-15s | %-8s | %-8s | %-10s | %-10s\n" "   ^" "$P_NAME" "$P_PID" "$P_CPU%" "$P_MEM%" "$P_RSS"
        fi
    done
    
    echo "================================================================================"
    sleep $INTERVAL
done
