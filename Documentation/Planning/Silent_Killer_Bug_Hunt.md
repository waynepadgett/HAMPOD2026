> **Status:** 🔴 Not Started
> **Last Updated:** 2026-06-21

# Silent Killer Bug Hunt

## The Problem

HAMPOD crashes and restarts. No error message, no warning. Just... reboot. Happens occasionally, not every time.

## What We Know

- **Symptom:** Full restart (not just a crash — the system comes back up)
- **Frequency:** Intermittent (not every session)
- **Visibility:** Silent (no logged error before restart)
- **Speed:** Crashes in milliseconds (logging won't survive)

## Gameplan

### Step 1: Check What Survived the Crash

The crash is too fast to log. But the *system* might have logged something. After every restart, run these immediately:

```bash
# Check kernel log for segfaults, panics, watchdog resets
dmesg | tail -50

# Check system log
cat /var/log/syslog | tail -50

# Check for core dumps
ls -la /tmp/core* /var/crash/* 2>/dev/null
```

Write down what you find each time. Patterns emerge.

### Step 2: Enable Core Dumps

If the app segfaults, a core dump gives you a snapshot of exactly what crashed and why.

```bash
# Enable core dumps for current session
ulimit -c unlimited

# Run HAMPOD normally
./Documentation/scripts/run_hampod.sh

# If it crashes, a core file appears. Analyze with:
gdb ./Software2/bin/hampod core
(gdb) bt    # backtrace — shows exactly where it crashed
```

### Step 3: Add a Signal Handler

Catch the crash *just before* it dies. Add this to `main.c`:

```c
#include <signal.h>
#include <stdio.h>

void crash_handler(int sig) {
    FILE *f = fopen("/tmp/hampod_crash.log", "a");
    fprintf(f, "CRASH: signal %d at %s\n", sig, __func__);
    fclose(f);
    _exit(1);  // exit so core dump is generated
}

// In main(), before anything else:
signal(SIGSEGV, crash_handler);
signal(SIGBUS, crash_handler);
signal(SIGABRT, crash_handler);
```

Now when it crashes, `/tmp/hampod_crash.log` tells you which signal killed it.

### Step 4: Check the Usual Suspects

Intermittent crashes on embedded systems are usually:

| Suspect | How to Check |
|---------|--------------|
| **Segfault** | `dmesg` will show "segfault at ..." — this is the most common |
| **NULL pointer** | Core dump backtrace will show the exact line |
| **Stack overflow** | Check thread stack sizes in `pthread_create` calls |
| **Watchdog reset** | `dmesg` shows "watchdog reset" — firmware is killing the app |
| **Memory exhaustion** | `monitor_mem.sh` — watch for climbing RSS before crash |

### Step 5: Reproduce It

The hardest part. Try to make it crash on command:

- Stress test: rapid key presses, fast mode switching
- Run overnight with `dmesg -w` watching for segfaults
- Add artificial delays to slow things down (timing bugs hide in speed)

## Success Criteria

- [ ] `dmesg` checked after every restart, findings logged
- [ ] Core dumps enabled and analyzed if they appear
- [ ] Signal handler deployed
- [ ] Segfault address or watchdog reset identified
- [ ] Root cause found and fixed
