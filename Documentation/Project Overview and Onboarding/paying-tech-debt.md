# Paying Technical Debt: HAMPOD Rebuild Plan

**Created:** December 4, 2025  
**Status:** 🔴 Not Started  
**Estimated Time:** 1-2 days  
**Risk Level:** Medium (reversible with Git)

---

## Executive Summary

The December 3rd bug-fixing sprint produced working code, but the *process* left the codebase in a fragile state. This plan preserves the valuable problem-solving knowledge while cleaning up the implementation.

### The Strategy:
1. **Save** the messy state to a branch (don't lose Wayne's work)
2. **Revert** main to the stable Nov 29 state
3. **Cherry-pick** the good solutions, applying them cleanly
4. **Set up CI/CD** so this never happens again

---

## Pre-Flight Checklist

Complete these before starting:

- [ ] **Notify team** — Wayne and any collaborators know this is happening
- [ ] **No one is working** — Ensure no active development on main
- [ ] **Local repo is clean** — `git status` shows no uncommitted changes
- [ ] **Backup exists** — Clone exists elsewhere or pushed to remote
- [ ] **Read through Dec 3rd commits** — Understand what was solved
- [ ] **Time blocked** — 2-3 hours of uninterrupted time available

---

## Phase 1: Preserve the Messy State

**Goal:** Save everything so nothing is lost

### Step 1.1: Create Archive Branch
```bash
# From main branch
git checkout main
git pull origin main

# Create branch to preserve Dec 3rd work
git checkout -b archive/dec3-bug-fixes

# Push to remote for safekeeping
git push origin archive/dec3-bug-fixes
```

- [x] Archive branch created
- [x] Archive branch pushed to GitHub
- [x] Verified branch appears on GitHub

### Step 1.2: Document What We're Preserving

Before reverting, extract the valuable knowledge:

---

#### 📡 Pipe Communication Solution

**Location:** `Software/FirmwareCommunication.c` and `Software/FirmwareCommunication.h`

**Key Discovery:** Pipes must use **relative paths** from the Software directory:
```c
#define INPUT_PIPE "../Firmware/Firmware_i"
#define OUTPUT_PIPE "../Firmware/Firmware_o"
```

**Implementation Pattern:**
```c
// In FirmwareCommunication.c (lines 17-38)
int firmware_input_fd = -1;
int firmware_output_fd = -1;

void firmwareCommunicationStartup(){
    // ... mutex init ...
    
    printf("Software: Connecting to Firmware pipes\n");
    firmware_input_fd = open(INPUT_PIPE, O_WRONLY);
    if(firmware_input_fd == -1) {
        perror("Failed to open Firmware_i");
    }
    firmware_output_fd = open(OUTPUT_PIPE, O_RDONLY);
    if(firmware_output_fd == -1) {
        perror("Failed to open Firmware_o");
    }
    printf("Software: Connected to Firmware pipes\n");
}
```

**Threading Fix:** A wrapper function was needed for `pthread_create`:
```c
// Wrapper for firmwarePlayAudio to match pthread_create signature
void* firmwarePlayAudioWrapper(void* text) {
    firmwarePlayAudio(text);
    return NULL;
}
```

**Forward Declaration Required:**
```c
// At top of FirmwareCommunication.c
char* applyDictionary(char* s);
```

- [x] Pipe implementation documented
- [x] Relative path fix documented (`../Firmware/`)
- [x] Threading approach documented

---

#### 📁 Header Include Fixes

**Files needing `stdlib.h`:**
| File | Status |
|------|--------|
| `Software/ModeRouting.c` | ✅ Documented |
| `Software/FirmwareCommunication.c` | ✅ Documented |
| `Software/IDQueue.c` | ✅ Documented |
| `Software/ThreadQueue.c` | ✅ Documented |
| `Software/GeneralFunctions.c` | ✅ Documented |

**Files needing `string.h`:**
| File | Status |
|------|--------|
| `Software/ModeRouting.c` | ✅ Documented |
| `Software/FirmwareCommunication.c` | ✅ Documented |

**Files needing `stdbool.h`:**
| File | Status |
|------|--------|
| `Software/IDQueue.h` | ✅ Documented |

**Files needing their own header include:**
| File | Include Needed | Status |
|------|----------------|--------|
| `Software/KeyWatching.c` | `#include "KeyWatching.h"` | ✅ |
| `Software/StateMachine.c` | `#include "StateMachine.h"` | ✅ |
| `Software/Radio.c` | `#include "Radio.h"` | ✅ |
| `Firmware/hampod_firm_packet.c` | `#include "hampod_firm_packet.h"` | ✅ |
| `Firmware/keypad_firmware.c` | `#include "keypad_firmware.h"` | ✅ |

- [x] All stdlib.h files listed
- [x] All string.h files listed
- [x] All stdbool.h files listed

---

#### 🛡️ SHAREDLIB Guard Pattern

**Purpose:** Prevents double-inclusion when headers are used by both Firmware and Software layers.

**Pattern to apply at END of header files:**
```c
// At end of .h file, before final #endif
#ifndef SHAREDLIB
#include "FileName.c"  // Include the .c file
#endif
```

**Files requiring this pattern:**
| Header File | Status |
|-------------|--------|
| `Software/ThreadQueue.h` | ✅ Has guard at line 53-55 |
| `Software/IDQueue.h` | ✅ Has guard at line 24-26 |
| `Software/FirmwareCommunication.h` | ✅ Has guard at line 83-85 |

**Makefile CFLAGS must include:**
```makefile
CFLAGS = -Wall -DSHAREDLIB
```

- [x] SHAREDLIB guard pattern documented

---

#### 🔧 Makefile Changes

**Firmware Makefile Key Changes:**
```makefile
# Compiler flags - MUST include -DSHAREDLIB
CFLAGS = -Wall -DSHAREDLIB
LDFLAGS = -lpthread -lasound

# HAL sources and objects
HAL_SRCS = hal/hal_keypad_usb.c hal/hal_audio_usb.c
HAL_OBJS = $(HAL_SRCS:.c=.o)

# Main target includes ALL object files
all: firmware.elf hampod_firm_packet.o audio_firmware.o keypad_firmware.o

# Link command must include HAL objects
firmware.elf: firmware.o hampod_firm_packet.o audio_firmware.o keypad_firmware.o $(HAL_OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

# Individual .o files for Software layer linkage
hampod_firm_packet.o: hampod_firm_packet.c hampod_firm_packet.h
	$(CC) $(CFLAGS) -c hampod_firm_packet.c -o hampod_firm_packet.o

# HAL pattern rule
hal/%.o: hal/%.c hal/hal_keypad.h hal/hal_audio.h
	$(CC) $(CFLAGS) -c $< -o $@
```

**Software Makefile Key Changes:**
```makefile
# Shared library approach
LIB_NAME := libModes.so

# Build shared library with -DSHAREDLIB -fPIC -shared
$(LIB_NAME): $(LIB_SRCS) $(HEADERS)
	$(CC) $(CFLAGS) -rdynamic -DSHAREDLIB -fPIC -shared -o $(LIB_NAME) $(LIB_SRCS) $(LDFLAGS)

# Link Firmware object files into Software.elf
$(EXEC_NAME): $(EXEC_SRC) $(LIB_NAME)
	$(CC) $(CFLAGS) -rdynamic -DSHAREDLIB -o $(EXEC_NAME) $(EXEC_SRC) \
		-I. -L. -l:$(LIB_NAME) -ldl $(LDFLAGS) -Wl,-rpath,'$$ORIGIN' \
		../Firmware/hampod_firm_packet.o \
		../Firmware/audio_firmware.o \
		../Firmware/keypad_firmware.o \
		../Firmware/hal/hal_audio_usb.o \
		../Firmware/hal/hal_keypad_usb.o
```

**Build Order (Critical):**
1. Build Firmware FIRST (`cd Firmware && make`)
2. Build Software SECOND (`cd Software && make`)

- [x] Shared library approach documented
- [x] Object file linking documented
- [x] Build order documented

---

#### 🖥️ Windows Deploy Script

**File:** `Documentation/scripts/remote_install.ps1`

**Usage:**
```powershell
.\remote_install.ps1 "<commit_message>"
```

**What it does:**
1. `git add .`
2. `git commit -m "<message>"`
3. `git push`
4. SSH to Pi: `cd ~/HAMPOD2026 && git pull`
5. SSH to Pi: `cd Firmware && make`
6. SSH to Pi: `cd ../Software && make`

**Key line (build order):**
```powershell
ssh waynesr@HAMPOD.local "cd ~/HAMPOD2026 && git pull && cd Firmware && make && cd ../Software && make"
```

- [x] Windows deploy script documented

---

#### 📋 Files to Reference from Archive

| File | What to Extract | Status |
|------|-----------------|--------|
| `Software/FirmwareCommunication.c` | Pipe implementation, forward declaration, wrapper function | ✅ Documented |
| `Software/FirmwareCommunication.h` | Pipe path defines, SHAREDLIB guard | ✅ Documented |
| `Software/makefile` | Shared lib approach, Firmware object linking | ✅ Documented |
| `Firmware/makefile` | HAL objects, SHAREDLIB flag, object rules | ✅ Documented |
| `Software/ThreadQueue.h` | SHAREDLIB guard pattern | ✅ Documented |
| `Software/IDQueue.h` | SHAREDLIB guard pattern | ✅ Documented |
| `Documentation/scripts/remote_install.ps1` | Windows deploy script | ✅ Documented |

---

### ✅ Step 1.2 Complete

All valuable knowledge from December 3rd has been extracted and documented above.

---

## Phase 2: Revert Main to Nov 29

**Goal:** Return to last known stable state

### Step 2.1: Identify the Target Commit

The last stable commit before Dec 3rd was:
```
6eaa25d  2025-11-29  Fix: ConfigParams.c compilation errors
```

Verify with:
```bash
git log --oneline --since="2025-11-29" --until="2025-11-30"
```

- [ ] Confirmed target commit hash: `______________`

### Step 2.2: Create a Clean Branch

```bash
# Return to main
git checkout main

# Create a new branch for the clean rebuild
git checkout -b rebuild/clean-dec3

# Reset to Nov 29 state
git reset --hard 6eaa25d

# Verify we're at the right spot
git log --oneline -5
```

- [ ] On `rebuild/clean-dec3` branch
- [ ] Reset to Nov 29 commit
- [ ] `git log` shows expected commits

### Step 2.3: Verify the Nov 29 State Builds

```bash
# Test Firmware build
cd Firmware
make clean
make

# Test Software build (expect some failures - that's why Dec 3 happened!)
cd ../Software
make clean
make
```

- [ ] Firmware builds: ✅ / ❌
- [ ] Software builds: ✅ / ❌ (expected to fail)
- [ ] Documented which errors appear: ________________________

---

## Phase 3: Cherry-Pick Solutions (Clean)

**Goal:** Apply Dec 3rd fixes properly, one at a time

### Approach:
Instead of cherry-picking whole commits (which were messy), we'll manually apply the *solutions* in a clean way.

### Step 3.1: Fix Missing Header Includes

Apply these systematically across all files:

**Files needing `stdlib.h`:**
- [ ] `Software/ModeRouting.c`
- [ ] `Software/FirmwareCommunication.c`
- [ ] `Software/IDQueue.c`
- [ ] `Software/ThreadQueue.c`
- [ ] `Software/GeneralFunctions.c`

**Files needing `string.h`:**
- [ ] `Software/ModeRouting.c`
- [ ] `Software/FirmwareCommunication.c`

**Files needing `stdbool.h`:**
- [ ] `Software/IDQueue.h`

**Files needing their own header:**
- [ ] `Software/KeyWatching.c` → `#include "KeyWatching.h"`
- [ ] `Software/StateMachine.c` → `#include "StateMachine.h"`
- [ ] `Software/Radio.c` → `#include "Radio.h"`
- [ ] `Firmware/hampod_firm_packet.c` → `#include "hampod_firm_packet.h"`
- [ ] `Firmware/keypad_firmware.c` → `#include "keypad_firmware.h"`

**Commit after all includes are fixed:**
```bash
git add -A
git commit -m "Fix: Add missing standard library includes to all source files

Added stdlib.h, string.h, stdbool.h where needed.
Added self-includes for header files.
Fixes standalone compilation issues."
```

- [ ] All includes added
- [ ] Single clean commit made

### Step 3.2: Add SHAREDLIB Guards

Add to headers that are included by both Firmware and Software:

**Pattern:**
```c
#ifndef SHAREDLIB
#define SHAREDLIB
// ... header content ...
#endif
```

**Files:**
- [ ] `Software/ThreadQueue.h`
- [ ] `Software/IDQueue.h`

```bash
git add -A
git commit -m "Fix: Add SHAREDLIB guards to prevent double inclusion

Prevents linker errors when headers are used by both Firmware and Software."
```

- [ ] Guards added
- [ ] Commit made

### Step 3.3: Fix Makefiles

**Firmware Makefile:**
- [ ] Add HAL object files to link command
- [ ] Add SHAREDLIB flag to CFLAGS
- [ ] Add object file compilation rules

**Software Makefile:**
- [ ] Implement shared library approach OR
- [ ] Fix object file linking

```bash
git add -A
git commit -m "Build: Fix Firmware and Software makefiles for Raspberry Pi

- Link HAL objects correctly
- Add SHAREDLIB compile flag
- Fix build order dependencies"
```

- [ ] Makefiles fixed
- [ ] Builds successfully
- [ ] Commit made

### Step 3.4: Implement Pipe Communication

Reference the archive branch for the solution:

```bash
# View the working implementation
git show archive/dec3-bug-fixes:Software/FirmwareCommunication.c
```

- [ ] Pipe paths use relative paths (`../Firmware/`)
- [ ] Forward declarations added
- [ ] Implementation tested

```bash
git add -A
git commit -m "Feature: Implement pipe communication between Firmware and Software

- Uses relative paths for portability
- Tested on Raspberry Pi"
```

- [ ] Pipe communication working
- [ ] Commit made

### Step 3.5: Add Windows Deploy Script

```bash
# Copy from archive
git checkout archive/dec3-bug-fixes -- Documentation/scripts/remote_install.ps1

git add -A
git commit -m "Feature: Add Windows deployment script (remote_install.ps1)

Builds Firmware first, then Software, then deploys to Pi."
```

- [ ] Script copied
- [ ] Commit made

### Step 3.6: Verify Everything Builds

```bash
cd Firmware && make clean && make
cd ../Software && make clean && make
```

- [ ] Firmware builds: ✅
- [ ] Software builds: ✅
- [ ] No warnings (or documented acceptable warnings)

---

## Phase 4: Set Up CI/CD

**Goal:** Never have another Dec 3rd situation

### Step 4.1: Choose CI Platform

- [ ] **GitHub Actions** (free for public repos, recommended)
- [ ] GitLab CI
- [ ] Other: _______________

### Step 4.2: Create GitHub Actions Workflow

Create file: `.github/workflows/build.yml`

```yaml
name: Build Check

on:
  push:
    branches: [ main, develop ]
  pull_request:
    branches: [ main ]

jobs:
  build-firmware:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
      
      - name: Install dependencies
        run: |
          sudo apt-get update
          sudo apt-get install -y build-essential libasound2-dev
      
      - name: Build Firmware
        run: |
          cd Firmware
          make clean
          make
          
  build-software:
    runs-on: ubuntu-latest
    needs: build-firmware
    steps:
      - uses: actions/checkout@v4
      
      - name: Install dependencies
        run: |
          sudo apt-get update
          sudo apt-get install -y build-essential libhamlib-dev libasound2-dev festival-dev
      
      - name: Build Software
        run: |
          cd Software
          make clean
          make
```

- [ ] Workflow file created
- [ ] Pushed to GitHub
- [ ] First run triggered
- [ ] Build passes: ✅ / ❌

### Step 4.3: Add Build Status Badge

Add to `README.md`:

```markdown
![Build Status](https://github.com/waynepadgett/HAMPOD2026/actions/workflows/build.yml/badge.svg)
```

- [ ] Badge added to README
- [ ] Badge displays correctly

### Step 4.4: Branch Protection Rules

On GitHub:
1. Go to Settings → Branches
2. Add rule for `main`
3. Enable:
   - [ ] Require status checks to pass before merging
   - [ ] Require branches to be up to date before merging
   - [ ] Select "build-firmware" and "build-software" as required checks

- [ ] Branch protection enabled

---

## Phase 5: Merge and Cleanup

### Step 5.1: Final Verification

```bash
# On rebuild/clean-dec3 branch
git log --oneline -10  # Review commits
make -C Firmware clean && make -C Firmware
make -C Software clean && make -C Software
```

- [ ] All commits look clean
- [ ] Build succeeds
- [ ] Tests pass (if any)

### Step 5.2: Merge to Main

```bash
git checkout main
git merge rebuild/clean-dec3 --no-ff -m "Rebuild: Clean implementation of Dec 3rd fixes

Replaces the messy Dec 3rd sprint with clean, documented commits.
Original work preserved in archive/dec3-bug-fixes branch.

Changes:
- Added missing standard library includes
- Added SHAREDLIB guards
- Fixed Makefiles for Pi build
- Implemented pipe communication
- Added Windows deploy script
- Added CI/CD with GitHub Actions"
```

- [ ] Merge completed
- [ ] Pushed to origin

### Step 5.3: Verify CI Passes

- [ ] GitHub Actions shows green checkmark
- [ ] Build status badge is green

### Step 5.4: Optional Cleanup

After 30 days of stability:

```bash
# Delete the messy archive (optional - it's good to keep for reference)
# git push origin --delete archive/dec3-bug-fixes
```

- [ ] Decided: Keep / Delete archive branch

---

## Post-Rebuild Checklist

- [ ] All team members pulled latest main
- [ ] Raspberry Pi deployment tested
- [ ] Documentation updated
- [ ] This checklist marked complete

---

## Rollback Plan

If anything goes wrong:

```bash
# Return main to the pre-rebuild state
git checkout main
git reset --hard archive/dec3-bug-fixes
git push --force origin main
```

⚠️ **Warning:** Force push will overwrite remote history. Use only if necessary.

---

## Notes Section

*Use this space to document decisions and issues during the rebuild:*

### Decisions Made:
- 

### Issues Encountered:
- 

### Files That Needed Extra Attention:
- 

### Time Log:
| Phase | Start Time | End Time | Duration |
|-------|------------|----------|----------|
| Phase 1 | | | |
| Phase 2 | | | |
| Phase 3 | | | |
| Phase 4 | | | |
| Phase 5 | | | |

---

## Sign-Off

| Role | Name | Date | Signature |
|------|------|------|-----------|
| Developer | | | |
| Reviewer | | | |

---

*Document Version: 1.0*  
*Last Updated: December 4, 2025*
