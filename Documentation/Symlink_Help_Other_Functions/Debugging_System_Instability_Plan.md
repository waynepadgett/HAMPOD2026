# Tracking Down System Instability & Crashes

When a C program causes a Raspberry Pi OS to become so unstable that it requires a reflash, the most likely culprits are:
1. **Memory Leaks (OOM)**: The program consumes all RAM, forcing the Pi to aggressively swap to the SD card, completely locking up the system and potentially corrupting the SD card.
2. **Overheating**: Pinning the Pi to performance mode without a heatsink can cause thermal throttling and, in extreme cases, hard crashes.
3. **Under-voltage**: Spikes in CPU usage can draw more current than the power supply can provide, browning out the Pi.
4. **Buffer Overflows**: A severe memory corruption out of bounds might cause segmentation faults, though this usually just kills the app, not the whole OS, *unless* it's writing to a critical system file descriptor.
5. **Storage Exhaustion/SD Corruption**: If the program is writing an unrestricted amount of data to the disk (e.g., an infinite error loop), it can fill the OS partition instantly, bricking the OS until reflashed.

Here is a phased approach to tracking down this "stinky" bug without having to stare at the screen for hours.

---

## Phase 1: Hardware & System Triage (The "Quick Checks")

Before diving into the C code, let's rule out physical limitations.

### 1. Check for Throttling & Power Issues
The Raspberry Pi hardware keeps a persistent record if it has *ever* throttled or experienced low voltage since the last boot. 
Run this command over SSH while the program is running (or after it has run for a bit):
```bash
vcgencmd get_throttled
```
* **`0x0`**: Everything is perfect.
* **`0x50000`** or similar: It has experienced under-voltage.
* **`0x80000`** or similar: It has experienced thermal throttling.

### 2. Monitor Temperature
To check if it's baking itself to death:
```bash
watch -n 1 vcgencmd measure_temp
```
*If it exceeds 80°C, it will throttle. If it exceeds 85°C, it can become unstable. If there is no heatsink and performance mode is on, this is highly likely.*

### 3. Check System Logs for the "OOM Killer"
If it was a memory leak, the Linux kernel will try to assassinate the program before the OS dies.
```bash
dmesg -T | grep -i -E "oom|memory|kill|segfault"
```
If you see the `OOM Killer` invoked, you absolutely have a memory leak.

---

## Phase 2: Memory Safety Tooling (The "Smart Developer" Approach)

You do not need to manually read 50,000 lines of AI-generated C code. We can use compiler tools to find memory leaks and buffer overflows automatically.

### 1. AddressSanitizer (ASan) - *Highly Recommended*
ASan is a compiler feature that slows your program down slightly but perfectly tracks every memory allocation, out-of-bounds array access, and memory leak. It will instantly crash the program and print the exact file and line number where the bad memory action happened.

**How to use:**
Modify your `Makefile` or compilation command to include these flags:
```Makefile
CFLAGS += -fsanitize=address -g
LDFLAGS += -fsanitize=address
```
Recompile the project, run it, and use the device. When the program does something illegal with memory, it will immediately halt and print a highly detailed error report to the terminal.

### 2. Valgrind
If ASan doesn't catch it, Valgrind is the heavy-duty option. It runs the program on a virtual CPU and monitors every byte. It *will* slow down the program by 10x-20x, so TTS/Audio might lag, but it is excellent for finding leaks.
```bash
sudo apt install valgrind
valgrind --leak-check=full --show-leak-kinds=all ./hampod_executable
```
Let it run, press some buttons, and then press `Ctrl+C` to stop it. Valgrind will print a summary of all leaked memory and the functions that allocated it.

---

## Phase 3: Application Logging & Profiling

If memory and hardware are fine, the application might be doing something logical but fatal (like an infinite `while` loop that locks the CPU at 100%).

### 1. Implement File Logging (Carefully)
Since you already have a TODO for logging, start writing logs to a file. 
**CRITICAL:** Write your logs to `/tmp/hampod.log` (which is RAM, not the SD card). If your program enters an endless loop and prints a billion lines of errors to the SD card, you will completely fill the SD card and brick the OS (requiring a reflash).
* Log level: INFO, WARN, ERROR.
* Log all major state machine transitions and button presses.

### 2. Run with `htop`
Open two SSH windows:
1. One running `./hampod`
2. One running `htop`
Watch the `RES` (Resident Memory) and `CPU%` columns for your program.
* If `RES` slowly but constantly goes up every time a button is pushed... you have a memory leak.
* If `CPU%` suddenly spikes to 100% and stays there... you have an infinite loop or thread deadlock.

### 3. Check for Console Spam (Confirmed Bug)
**We recently found a bug where Hamlib was aggressively logging `rig_get_freq` calls to the terminal (>100 lines per second).** 
If the `-hampod` running script was ever redirected to save output to a file (e.g. `./run_hampod.sh > output.log`), it would fill up the Raspberry Pi's SD card entirely in a matter of minutes, rendering the OS unresponsive and requiring a reflash.
* **The Fix**: We added `rig_set_debug(RIG_DEBUG_NONE);` to `radio.c` to silence this spam. Let's monitor if the crashes stop after this fix.

---

## Phase 4: Isolation Testing

To avoid sitting there for 30 minutes pushing buttons:

1. **The Idle Test:** Start the program and literally do not touch it for 30 minutes. If it crashes, the bug is in a background thread (e.g., polling the keypad, checking battery, or a sleep loop).
2. **The Spam Test:** Rapid-fire press buttons. Interrupt the TTS while it's talking. If it crashes here, the bug is in thread synchronization, interrupting an audio stream, or a buffer overflow related to queueing too many actions.

## Summary Checklist for Dad/Rob
- [ ] SSH in and run `watch -n 1 vcgencmd measure_temp` while it runs.
- [ ] Run `vcgencmd get_throttled` after 10 mins.
- [ ] Check `dmesg` for OOM-killer logs from previous crashes.
- [ ] Recompile with `-fsanitize=address -g` (AddressSanitizer) and run it normally.
- [ ] Watch the program in `htop` over SSH. Does memory strictly increase over time?
