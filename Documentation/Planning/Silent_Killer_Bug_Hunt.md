> **Status:** 🔴 In Progress
> **Last Updated:** 2026-06-21

# Silent Killer Bug Hunt

## The Problem

HAMPOD crashes so hard the Raspberry Pi OS reboots. No error message, no warning. Just... reboot. Happens occasionally, not every time.

## What We Know

- **Symptom:** Full OS restart (not just an app crash — the entire Pi reboots)
- **Frequency:** Intermittent (not every session)
- **Visibility:** Silent (no logged error before restart)
- **Speed:** Crashes in milliseconds (logging won't survive)

## Root Cause Analysis (Verified 2026-06-21)

### Primary Suspect: Stack Buffer Overflow in Firmware Pipe Reader

**File:** `Firmware/firmware.c`, `io_buffer_thread()`, lines 351 and 393
**File:** `Firmware/keypad_firmware.c`, `keypad_io_thread()`, lines 154 and 191

Both IO threads have the same pattern:

```c
unsigned char buffer[256];                    // 256-byte stack buffer (line 351/154)
...
bytes_read = read(i_pipe, buffer, size);      // 'size' is unsigned short (0–65535) (line 393/191)
```

**The bug:** `size` is read directly from the named pipe as an `unsigned short` and used as the byte count for `read()` with NO bounds check against the 256-byte buffer. If `size > 256`, this is a stack buffer overflow that corrupts the return address and other stack frames.

**Why it happens:** The pipe protocol between Software2 and Firmware is a raw byte stream with no framing. Software2 sends packets via 4 separate `write()` calls (type, data_len, tag, data). Firmware reads them via 4 separate `read()` calls. Each `read()` only checks `bytes_read <= 0` — it does NOT verify the full expected number of bytes was received. If any `read()` returns a short count (which is legal on Linux pipes), the header fields desynchronize. After desynchronization, `size` is interpreted from garbage data and can be any value up to 65535.

**Why it reboots the OS:** `firmware.elf` runs as root (`sudo ./firmware.elf` in `run_hampod.sh` line 136). A stack buffer overflow in a root process corrupts memory at a privilege level that can trigger kernel panics or watchdog resets when the corrupted thread returns.

### Secondary Suspects

| Suspect | Status | Notes |
|---------|--------|-------|
| **Memory exhaustion (OOM)** | Plausible | Linked-list queue could leak if packets aren't destroyed; `monitor_mem.sh` should be run to check |
| **Overheating** | Plausible | Pi 3B+ overclocked to 1.5GHz (`Overclocking_Plan.md`); check `vcgencmd get_throttled` after crash |
| **Under-voltage** | Plausible | Overclocked Pi draws more current; check `vcgencmd get_throttled` for `0x50000` |
| **Non-async-signal-safe handlers** | Confirmed | `sigsegv_handler` in `firmware.c:465` calls `printf()` and `exit()` — neither is async-signal-safe. This prevents clean crash reporting and can cause secondary corruption |
| **Software2 missing crash signal handlers** | Confirmed | `Software2/src/main.c` only handles SIGINT/SIGTERM, not SIGSEGV/SIGBUS/SIGABRT |

### What Makes This Intermittent

The buffer overflow only triggers if the pipe stream desynchronizes. In normal operation, Software2 writes each field atomically and Firmware reads them in order. Desynchronization requires a timing anomaly — a partial read caused by signal interruption, pipe buffer pressure, or process scheduling jitter. This explains why the crash is intermittent and not reproducible on demand.

## Gameplan

### Step 1: Add Bounds Check to Firmware IO Threads (Critical Fix)

**Files:** `Firmware/firmware.c`, `Firmware/keypad_firmware.c`

In both `io_buffer_thread` and `keypad_io_thread`, add a bounds check BEFORE the data read:

```c
// After reading size from pipe:
bytes_read = read(i_pipe, &size, 2);
if (bytes_read <= 0) {
    break;
}

// ADD THIS BOUNDS CHECK:
if (size > 256) {
    fprintf(stderr, "FATAL: packet size %u exceeds buffer (256) — stream desync?\n", size);
    break;  // or: skip this packet and try to resync
}
```

This prevents the stack buffer overflow regardless of what `size` contains.

### Step 2: Add Read Loops to Prevent Desynchronization

**Files:** `Firmware/firmware.c`, `Firmware/keypad_firmware.c`

Replace each bare `read()` with a loop that retries until all expected bytes are received:

```c
// Helper function (add to firmware.c):
static ssize_t read_exact(int fd, void *buf, size_t count) {
    size_t total = 0;
    while (total < count) {
        ssize_t n = read(fd, (char *)buf + total, count - total);
        if (n <= 0) return n;
        total += n;
    }
    return (ssize_t)total;
}

// Then replace each header read:
bytes_read = read_exact(i_pipe, &packet_type, sizeof(Packet_type));
bytes_read = read_exact(i_pipe, &size, 2);
bytes_read = read_exact(i_pipe, &tag, sizeof(unsigned short));
```

This eliminates the root cause of desynchronization by ensuring each field is fully read before moving to the next.

### Step 3: Make Firmware Signal Handler Async-Signal-Safe

**File:** `Firmware/firmware.c`, lines 465-469

Replace the current handler:

```c
void sigsegv_handler(int signum) {
    // Use only async-signal-safe functions:
    const char msg[] = "\033[0;31mSEGMENTATION FAULT\n";
    write(STDERR_FILENO, msg, sizeof(msg) - 1);
    _exit(1);  // NOT exit() — _exit is async-signal-safe
}
```

Same fix for `sigint_handler` (use `write()` instead of `printf()`, `_exit(0)` instead of `exit(0)`).

### Step 4: Add Crash Signal Handlers to Software2

**File:** `Software2/src/main.c`

Add the crash handler from the original plan, but make it async-signal-safe:

```c
#include <signal.h>
#include <unistd.h>

static void crash_handler(int sig) {
    const char msg[] = "CRASH: received signal\n";
    write(STDERR_FILENO, msg, sizeof(msg) - 1);
    _exit(1);
}

// In main(), add:
signal(SIGSEGV, crash_handler);
signal(SIGBUS, crash_handler);
signal(SIGABRT, crash_handler);
```

### Step 5: Diagnose the Actual Crash (While Deploying Fixes)

After deploying fixes, run these to confirm the root cause:

```bash
# Check if hardware watchdog is active
cat /proc/sys/kernel/watchdog
# If enabled, check dmesg for watchdog resets after a crash

# Check for OOM kills
dmesg -T | grep -i -E "oom|kill|segfault|panic|watchdog"

# Check for throttling/under-voltage
vcgencmd get_throttled

# Monitor memory usage over time
./Documentation/scripts/monitor_mem.sh
```

### Step 6: Stress Test

After fixes are deployed:

- Rapid key presses (10+ per second)
- Fast mode switching (Normal → Frequency → Set → Normal)
- Run overnight with `dmesg -w` watching for segfaults
- Monitor temperature with `watch -n 1 vcgencmd measure_temp`

## Success Criteria

- [ ] Bounds check added to `firmware.c` io_buffer_thread (size > 256 → reject)
- [ ] Bounds check added to `keypad_firmware.c` keypad_io_thread (size > 256 → reject)
- [ ] Read loops added to prevent pipe desynchronization
- [ ] `sigsegv_handler` / `sigint_handler` made async-signal-safe
- [ ] Crash signal handlers added to `Software2/src/main.c`
- [ ] `dmesg` checked after every restart, findings logged
- [ ] Stress test passes without crash
- [ ] Root cause confirmed or ruled out via `dmesg` / `vcgencmd get_throttled`
