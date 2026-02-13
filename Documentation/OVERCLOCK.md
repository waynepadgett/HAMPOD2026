# Raspberry Pi 3 B+ Overclock Configuration

## Overview
This document describes the conservative overclock settings applied to the HAMPOD Raspberry Pi 3 B+ to reduce TTS latency for Piper neural network inference.

## Current Settings

Applied to `/boot/firmware/config.txt` (or `/boot/config.txt` on older systems):

```ini
arm_freq=1500          # CPU: 1.5 GHz (7% boost from 1.4 GHz stock)
gpu_freq=500           # GPU: 500 MHz (25% boost from 400 MHz)
over_voltage=2         # Voltage: +0.05V for stability
temp_limit=75          # Thermal throttle at 75°C (vs 70°C default)
```

### Piper TTS Speed Settings

In addition to the hardware overclock, the following Piper TTS parameters are tuned in `hal_tts_piper.c`:

```
length_scale = 0.45    # ~2.2x faster speech (default 1.0 = normal speed)
noise_scale  = 0.0     # Deterministic inference (no random sampling)
noise_scale_w = 0.0    # Deterministic pronunciation
```

> **History**: Initially set to `0.3` (~3.3x speed) but was too fast/unintelligible. Adjusted to `0.45` on 2026-02-12.

## Performance Impact

### Measured Performance (2026-02-11)
**Conditions**: CPU pinned to 1.5 GHz (`performance` governor)
- **Time-to-First-Audio (TTFB)**: **~190-210ms** (Down from **700ms** - >3x faster!)
- **Total Processing Time**: **~315ms** (For short word "two" including playback)
- **Responsiveness**: "Instant" feel for short phrases
- **Note**: Occasional ALSA underruns observed (buffer tuning pending)

### Thermal Stability
- **Idle Temp**: ~45°C
- **Load Temp (TTS Loop)**: **52.6°C** (Passive cooling, well below 75°C throttle)
- **Stability**: Stable with `over_voltage=2`

## Safety & Stability

### Conservative Overclock Profile
This is a **conservative** overclock that should be stable without additional cooling:
- Only 7% CPU boost (1.4 → 1.5 GHz)
- Minimal voltage increase (+0.05V)
- Well within Pi 3 B+ capabilities

### Warranty Impact
- `over_voltage=2` is **within warranty limits** (warranty void starts at `over_voltage=6`)
- No `force_turbo` flag used

### Recommended Cooling
While this overclock should work with passive cooling, for best results:
- **Minimum**: Heatsink on CPU
- **Recommended**: Small 5V fan + heatsink
- **Monitor temps**: Use `vcgencmd measure_temp` to check

## Testing & Verification

### After Reboot, Verify Settings:
```bash
# Check CPU frequency (should show ~1500000000 under load)
vcgencmd measure_clock arm

# Check temperature
vcgencmd measure_temp

# Check for thermal throttling (should return throttled=0x0)
vcgencmd get_throttled

# Run TTS benchmark
cd ~/HAMPOD2026/Firmware/hal/tests
./test_persistent_piper
```

### Stability Testing
```bash
# CPU stress test (run for 5-10 minutes, monitor temps)
stress-ng --cpu 4 --timeout 300s --metrics-brief

# Watch temperature during stress
watch -n 1 vcgencmd measure_temp
```

## Rollback Instructions

If the system becomes unstable:

### Option 1: Restore Backup
```bash
sudo cp /boot/firmware/config.txt.backup /boot/firmware/config.txt
sudo reboot
```

### Option 2: Remove Overclock Settings
```bash
sudo nano /boot/firmware/config.txt
# Delete the HAMPOD Performance Overclock section
sudo reboot
```

### Option 3: Emergency Boot (if Pi won't boot)
1. Remove SD card
2. Mount on another computer
3. Edit `config.txt` in the boot partition
4. Remove overclock lines
5. Reinsert SD card and boot

## Further Optimization Options

If you want even more performance (requires active cooling):

### Aggressive Overclock
```ini
arm_freq=1570          # 1.57 GHz (12% boost)
gpu_freq=525           # 525 MHz GPU
over_voltage=4         # More voltage for stability
sdram_freq=500         # RAM overclock
temp_limit=80          # Allow higher temps with active cooling
```

**⚠️ WARNING**: Aggressive overclock requires:
- Active cooling (fan + heatsink)
- High-quality power supply (5V/3A minimum)
- Thorough stability testing

## Monitoring Commands

```bash
# Real-time CPU frequency
watch -n 0.5 vcgencmd measure_clock arm

# Real-time temperature
watch -n 1 'vcgencmd measure_temp && vcgencmd get_throttled'

# Check if currently throttled
vcgencmd get_throttled
# 0x0 = no throttling (good!)
# 0x50000 = currently throttled (bad - reduce overclock or add cooling)
```

## Branch Information

- **Branch**: `feature/pi3-overclock`
- **Date Applied**: 2026-02-11
- **Backup Location**: `/boot/firmware/config.txt.backup`

## References

- [Raspberry Pi Overclocking Documentation](https://www.raspberrypi.com/documentation/computers/config_txt.html#overclocking)
- [Pi 3 B+ Thermal Management](https://www.raspberrypi.com/documentation/computers/raspberry-pi.html#frequency-management-and-thermal-control)
