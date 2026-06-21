> **Status:** 🔴 In Progress
> **Last Updated:** 2026-06-21
> **Verified:** 2026-06-21 — all line numbers and claims checked against source

# Silent Killer Bug Hunt

## The Problem

HAMPOD crashes so hard the Raspberry Pi OS reboots. No error message, no warning. Just... reboot. Happens occasionally, not every time.

## What We Know

- **Symptom:** Full OS restart (not just an app crash — the entire Pi reboots)
- **Frequency:** Intermittent (not every session)
- **Visibility:** Silent (no app-level error before restart, but kernel `dmesg` log survives)
- **Speed:** Crashes in milliseconds (app logging won't survive, but `dmesg` will)

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

**Why it happens:** The pipe protocol between Software2 and Firmware is a raw byte stream with no framing. Software2 sends packets via 4 separate `write()` calls (type, data_len, tag, data — verified in `comm.c:comm_send_packet()` lines 477-504). Firmware reads them via 4 separate `read()` calls. Each `read()` only checks `bytes_read <= 0` — it does NOT verify the full expected number of bytes was received. If any `read()` returns a short count (which is legal on Linux pipes, especially under signal interruption), the header fields desynchronize. After desynchronization, `size` is interpreted from garbage data and can be any value up to 65535.

**Why it reboots the OS:** `firmware.elf` runs as root (`sudo ./firmware.elf` in `run_hampod.sh` line 136). **Software2 also runs as root** (`sudo ./bin/hampod` in `run_hampod.sh` line 192). A stack buffer overflow in either root process corrupts memory at a privilege level that can trigger kernel panics or watchdog resets when the corrupted thread returns.

### Additional Desync-Prone Read Sites (Verified, Not in Original Plan)

| Site | File:Lines | Risk |
|------|-----------|------|
| `audio_waiter()` thread | `firmware.c:435-442` | Bare `read()` from `audio_out_pipe_fd`, checks `<= 0` but not short counts. No buffer overflow (fixed sizes), but desync garbles audio responses. |
| Main thread keypad reader | `firmware.c:300-303` | Bare `read()` from `keypad_out_pipe_fd` with **NO error checking at all** — not even `<= 0`. If keypad process dies or pipe desyncs, values are undefined. Worse than the IO threads. |

### Secondary Suspects

| Suspect | Status | Notes |
|---------|--------|-------|
| **Memory exhaustion (OOM)** | Plausible | Linked-list queue could leak if packets aren't destroyed; `monitor_mem.sh` should be run to check |
| **Overheating** | Plausible | Pi 3B+ overclocked to 1.5GHz (`Overclocking_Plan.md`); check `vcgencmd get_throttled` after crash |
| **Under-voltage** | Plausible | Overclocked Pi draws more current; check `vcgencmd get_throttled` for `0x50000`. **This is a strong alternative root cause** — under-voltage reboots leave NO segfault in dmesg. |
| **Non-async-signal-safe handlers** | Confirmed | `sigsegv_handler` in `firmware.c:465` calls `printf()` and `exit()` — neither is async-signal-safe. Also `sigint_handler` at `firmware.c:459`. Also `signal_handler` in `Software2/src/main.c:44` uses `printf()`. |
| **Software2 missing crash signal handlers** | Confirmed | `Software2/src/main.c:166-167` only handles SIGINT/SIGTERM, not SIGSEGV/SIGBUS/SIGABRT |

### What Makes This Intermittent

The buffer overflow only triggers if the pipe stream desynchronizes. In normal operation, Software2 writes each field atomically (writes ≤ PIPE_BUF are atomic on Linux) and Firmware reads them in order. Desynchronization requires a timing anomaly — a partial read caused by signal interruption, pipe buffer pressure, or process scheduling jitter. This explains why the crash is intermittent and not reproducible on demand.

## Gameplan

### Step 1: Diagnose the Actual Crash (DO THIS FIRST — Before Any Code Changes)

**Why first:** If the root cause is under-voltage (overclocked Pi 3B+), code fixes won't stop the reboots. Check `dmesg` immediately after the next crash while kernel evidence is fresh. If `dmesg` shows a segfault → software bug (proceed to Step 2). If `dmesg` shows NO segfault but shows throttling/under-voltage → hardware problem (fix power supply or reduce overclock BEFORE touching code).

```bash
# Check if hardware watchdog is active
cat /proc/sys/kernel/watchdog
# If enabled, check dmesg for watchdog resets after a crash

# Check for OOM kills, segfaults, panics, watchdog resets
dmesg -T | grep -i -E "oom|kill|segfault|panic|watchdog"

# Check for throttling/under-voltage (0x50000 = under-voltage)
vcgencmd get_throttled

# Monitor memory usage over time
./Documentation/scripts/monitor_mem.sh
```

**Decision gate:**
- `dmesg` shows segfault in `firmware.elf` → confirmed software bug, proceed to Step 2
- `dmesg` shows no segfault, `vcgencmd get_throttled` shows `0x50000` → hardware problem, fix power/overclock first
- `dmesg` shows OOM kill → memory leak, investigate queue cleanup

**Either way, proceed to Step 2** — the buffer overflow is a real bug that must be fixed regardless of whether it's causing the reboots.

### Step 2: Fix All Firmware Read Sites (Bounds Check + Read Loop)

**Files:** `Firmware/firmware.c`, `Firmware/keypad_firmware.c`

**Applies to ALL four read sites:**
1. `io_buffer_thread()` — `firmware.c:375-397`
2. `keypad_io_thread()` — `keypad_firmware.c:173-195`
3. `audio_waiter()` — `firmware.c:435-442`
4. Main thread keypad reader — `firmware.c:300-303`

#### 2a: Add `read_exact()` helper (handles EINTR)

Add to `firmware.c` (and declare in header or make available to `keypad_firmware.c`):

```c
#include <errno.h>  // Add to includes

static ssize_t read_exact(int fd, void *buf, size_t count) {
    size_t total = 0;
    while (total < count) {
        ssize_t n = read(fd, (char *)buf + total, count - total);
        if (n < 0 && errno == EINTR) continue;  // Retry on signal interruption
        if (n <= 0) return n;                    // EOF or error
        total += n;
    }
    return (ssize_t)total;
}
```

**The `EINTR` check is critical:** the plan adds signal handlers, which means signals will be MORE common. Without this check, a signal arriving during `read()` would cause the IO thread to exit, making firmware stop processing input.

#### 2b: Replace all bare `read()` calls with `read_exact()`

In `io_buffer_thread` and `keypad_io_thread`:

```c
// Replace each bare read() with read_exact():
bytes_read = read_exact(i_pipe, &packet_type, sizeof(Packet_type));
if (bytes_read <= 0) break;

bytes_read = read_exact(i_pipe, &size, sizeof(unsigned short));
if (bytes_read <= 0) break;

// ADD BOUNDS CHECK (use sizeof, not magic number):
if (size > sizeof(buffer)) {
    fprintf(stderr, "FATAL: packet size %u exceeds buffer (%zu) — stream desync?\n",
            size, sizeof(buffer));
    break;
}

bytes_read = read_exact(i_pipe, &tag, sizeof(unsigned short));
if (bytes_read <= 0) break;

bytes_read = read_exact(i_pipe, buffer, size);
if (bytes_read <= 0) break;
```

In `audio_waiter()` (`firmware.c:435-442`) and main thread keypad reader (`firmware.c:300-303`): replace each bare `read()` with `read_exact()`. No bounds check needed (fixed-size reads), but the read loop prevents desync.

### Step 3: Make Firmware Signal Handlers Async-Signal-Safe

**File:** `Firmware/firmware.c`, lines 459-468

```c
void sigint_handler(int signum) {
    const char msg[] = "\033[0;31mTERMINATING FIRMWARE\n";
    write(STDERR_FILENO, msg, sizeof(msg) - 1);
    running = 0;
    _exit(0);  // NOT exit() — _exit is async-signal-safe
}

void sigsegv_handler(int signum) {
    const char msg[] = "\033[0;31mSEGMENTATION FAULT\n";
    write(STDERR_FILENO, msg, sizeof(msg) - 1);
    _exit(1);  // NOT exit() — _exit is async-signal-safe
}
```

### Step 4: Fix Software2 Signal Handling

**File:** `Software2/src/main.c`

**4a: Make existing `signal_handler` async-signal-safe** (line 42-46):

```c
static void signal_handler(int sig) {
    (void)sig;
    const char msg[] = "\nShutting down...\n";
    write(STDERR_FILENO, msg, sizeof(msg) - 1);
    g_running = false;
}
```

**4b: Add crash handlers** (new code, after `signal_handler`):

```c
static void crash_handler(int sig) {
    const char msg[] = "CRASH: received signal\n";
    write(STDERR_FILENO, msg, sizeof(msg) - 1);
    _exit(1);
}

// In main(), after existing signal() calls (line 167):
signal(SIGSEGV, crash_handler);
signal(SIGBUS, crash_handler);
signal(SIGABRT, crash_handler);
```

**Why this matters:** Software2 also runs as root (`run_hampod.sh:192`). A crash in Software2 is also a root-process crash that could trigger watchdog resets.

### Step 5: Stress Test

After fixes are deployed:

- Rapid key presses (10+ per second)
- Fast mode switching (Normal → Frequency → Set → Normal)
- Run overnight with `dmesg -w` watching for segfaults
- Monitor temperature with `watch -n 1 vcgencmd measure_temp`
- Check `vcgencmd get_throttled` periodically for under-voltage

## Success Criteria

- [ ] **Step 1 completed:** `dmesg` checked after crash, root cause categorized (software/hardware/memory)
- [ ] `read_exact()` helper added with EINTR handling
- [ ] Bounds check added to `firmware.c` `io_buffer_thread` (size > sizeof(buffer) → reject)
- [ ] Bounds check added to `keypad_firmware.c` `keypad_io_thread` (size > sizeof(buffer) → reject)
- [ ] `read_exact()` applied to `audio_waiter()` thread (`firmware.c:435-442`)
- [ ] `read_exact()` applied to main thread keypad reader (`firmware.c:300-303`)
- [ ] `sigsegv_handler` / `sigint_handler` made async-signal-safe in firmware
- [ ] Existing `signal_handler` in `Software2/src/main.c` made async-signal-safe
- [ ] Crash signal handlers (SIGSEGV/SIGBUS/SIGABRT) added to `Software2/src/main.c`
- [ ] `dmesg` checked after every restart, findings logged
- [ ] Stress test passes without crash
- [ ] Root cause confirmed or ruled out via `dmesg` / `vcgencmd get_throttled`
