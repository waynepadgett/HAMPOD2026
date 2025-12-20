# Step 0.9: Integration Test Plan

> **Parent Plan:** [Fresh Start: Phase Zero Plan](fresh-start-phase-zero-plan.md)  
> **Goal:** Combine all Software2 modules into a working integration test that demonstrates end-to-end functionality.

## Overview

This plan creates an integration test (`main_phase0.c`) that ties together:
- **Comm module** - Pipe communication with Firmware
- **Speech module** - Non-blocking speech queue and TTS
- **Keypad module** - Key press/hold detection with callbacks

**Success Criteria:**
1. App compiles cleanly with no warnings
2. Key presses trigger speech ("You pressed <key>")
3. Key holds trigger different speech ("You held <key>")
4. Speech is queued correctly (no cutoffs)
5. App handles Ctrl+C gracefully

---

## Prerequisites Checklist

Before starting, verify these are working:
- [ ] Firmware builds and runs (`cd Firmware && make && sudo ./firmware.elf`)
- [ ] `test_comm_read` works (reads keypad via Firmware pipes)
- [ ] `test_speech_queue` works (speaks text via Firmware)
- [ ] `test_keypad_events` works (detects key press and hold)

---

## Step-by-Step Implementation

### Step 1: Create Empty main_phase0.c Skeleton

**Goal:** Create a minimal C file that compiles successfully.

**File:** `Software2/src/main_phase0.c`

```c
/**
 * main_phase0.c - Phase Zero Integration Test
 * 
 * Demonstrates all Software2 modules working together:
 * - Comm (Firmware pipe communication)
 * - Speech (non-blocking TTS queue)
 * - Keypad (key press/hold detection)
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include "hampod_core.h"

static volatile int running = 1;

void signal_handler(int sig) {
    if (sig == SIGINT) {
        printf("\nShutting down...\n");
        running = 0;
    }
}

int main() {
    printf("=== HAMPOD2026 Phase Zero Integration Test ===\n");
    signal(SIGINT, signal_handler);
    
    printf("Press Ctrl+C to exit\n");
    
    while (running) {
        usleep(100000);  // 100ms loop
    }
    
    printf("Goodbye!\n");
    return 0;
}
```

**Verification:**
```bash
# Compile manually to verify it builds
cd Software2
gcc -Wall -pthread -I./include -o bin/test_phase0 src/main_phase0.c
./bin/test_phase0
# Press Ctrl+C - should exit cleanly
```

---

### Step 2: Add Makefile Target for Integration Test

**Goal:** Add a build target so we can easily rebuild.

**File:** `Software2/Makefile` (add to existing)

Add after the `test_%` rule:

```makefile
# Phase 0 Integration Test
$(BIN_DIR)/phase0_test: $(SRC_DIR)/main_phase0.c $(filter-out $(OBJ_DIR)/main.o, $(OBJS))
	$(CC) $(CFLAGS) -o $@ $^
```

Also add `phase0` to the `.PHONY` targets.

**Verification:**
```bash
cd Software2
make phase0_test
./bin/phase0_test
# Press Ctrl+C - should exit cleanly
```

---

### Step 3: Add Comm Module Initialization

**Goal:** Initialize and verify connection to Firmware.

**Changes to `main_phase0.c`:**

1. Add include: `#include "comm.h"`
2. After signal setup, add:
   ```c
   printf("Initializing communication...\n");
   if (comm_init() != HAMPOD_OK) {
       LOG_ERROR("Failed to connect to Firmware. Is firmware.elf running?");
       return 1;
   }
   printf("Connected to Firmware.\n");
   
   printf("Waiting for Firmware ready signal...\n");
   if (comm_wait_ready() != HAMPOD_OK) {
       LOG_ERROR("Firmware did not send ready signal");
       comm_close();
       return 1;
   }
   printf("Firmware ready!\n");
   ```
3. Before `return 0`, add:
   ```c
   comm_close();
   ```

**Verification:**
```bash
# Terminal 1: Start Firmware
cd Firmware && sudo ./firmware.elf

# Terminal 2: Run integration test
cd Software2 && make phase0_test && ./bin/phase0_test
# Should see "Connected to Firmware" and "Firmware ready!"
# Press Ctrl+C to exit
```

---

### Step 4: Add Speech Module Initialization

**Goal:** Start the speech system and speak a startup message.

**Changes to `main_phase0.c`:**

1. Add include: `#include "speech.h"`
2. After "Firmware ready!", add:
   ```c
   printf("Initializing speech system...\n");
   if (speech_init() != HAMPOD_OK) {
       LOG_ERROR("Failed to initialize speech system");
       comm_close();
       return 1;
   }
   printf("Speech system ready.\n");
   
   // Announce startup
   speech_say_text("Phase zero integration test ready. Press any key.");
   ```
3. Before `comm_close()`, add:
   ```c
   speech_say_text("Goodbye");
   speech_wait_complete();  // Wait for final speech before shutdown
   speech_shutdown();
   ```

**Verification:**
```bash
# Terminal 1: Start Firmware
cd Firmware && sudo ./firmware.elf

# Terminal 2: Run integration test
cd Software2 && make phase0_test && ./bin/phase0_test
# Should HEAR "Phase zero integration test ready. Press any key."
# Press Ctrl+C
# Should HEAR "Goodbye"
```

---

### Step 5: Add Keypad Callback Function (Stub)

**Goal:** Define the callback function that will handle key events.

**Changes to `main_phase0.c`:**

1. Add include: `#include "keypad.h"`
2. Before `main()`, add the callback function:
   ```c
   /**
    * Handle keypad events.
    * Called by keypad module when a key is pressed or held.
    */
   void on_key_event(const KeyPressEvent* event) {
       if (event->key == '-') {
           return;  // No key pressed, ignore
       }
       
       printf("[Keypad] Key: '%c', Hold: %s\n", 
              event->key, 
              event->isHold ? "YES" : "NO");
       
       // TODO: Speak the key
   }
   ```

**Verification:**
```bash
cd Software2 && make phase0_test
# Should compile without warnings
```

---

### Step 6: Add Keypad Module Initialization

**Goal:** Start the keypad polling thread and register our callback.

**Changes to `main_phase0.c`:**

1. After "Speech system ready.", add:
   ```c
   printf("Initializing keypad system...\n");
   if (keypad_init() != HAMPOD_OK) {
       LOG_ERROR("Failed to initialize keypad system");
       speech_shutdown();
       comm_close();
       return 1;
   }
   keypad_register_callback(on_key_event);
   printf("Keypad system ready.\n");
   ```
2. Before `speech_say_text("Goodbye")`, add:
   ```c
   keypad_shutdown();
   ```

**Verification:**
```bash
# Terminal 1: Start Firmware
cd Firmware && sudo ./firmware.elf

# Terminal 2: Run integration test
cd Software2 && make phase0_test && ./bin/phase0_test
# Press keys on keypad
# Should see console output like: [Keypad] Key: '5', Hold: NO
# Press Ctrl+C to exit
```

---

### Step 7: Implement Key Press Speech

**Goal:** Speak "You pressed <key>" when a key is pressed (not held).

**Changes to `on_key_event()` function:**

Replace the `// TODO: Speak the key` comment with:
```c
    char speak_buffer[64];
    
    if (!event->isHold) {
        // Regular press
        snprintf(speak_buffer, sizeof(speak_buffer), "You pressed %c", event->key);
        speech_say_text(speak_buffer);
    }
```

**Verification:**
```bash
# Run integration test
# Press keys quickly (don't hold)
# Should HEAR "You pressed 1", "You pressed 2", etc.
```

---

### Step 8: Implement Key Hold Speech

**Goal:** Speak "You held <key>" when a key is held.

**Changes to `on_key_event()` function:**

Expand the if/else:
```c
    char speak_buffer[64];
    
    if (event->isHold) {
        // Long press (held)
        snprintf(speak_buffer, sizeof(speak_buffer), "You held %c", event->key);
        speech_say_text(speak_buffer);
    } else {
        // Regular press
        snprintf(speak_buffer, sizeof(speak_buffer), "You pressed %c", event->key);
        speech_say_text(speak_buffer);
    }
```

**Verification:**
```bash
# Run integration test
# Press and HOLD a key for 1+ seconds
# Should HEAR "You pressed 5" then "You held 5"
# (The "pressed" happens on initial press, "held" happens after threshold)
```

---

### Step 9: Final Polish and Cleanup

**Goal:** Add finishing touches for a complete integration test.

**Changes:**

1. Add a startup log of all modules:
   ```c
   printf("\n=== All systems GO! ===\n");
   printf("Press keys on the keypad to hear them spoken.\n");
   printf("Hold a key for 1+ second to hear 'held' message.\n");
   printf("Press Ctrl+C to exit.\n\n");
   ```

2. Add orderly shutdown message:
   ```c
   printf("\nShutdown sequence:\n");
   printf("  - Stopping keypad...\n");
   keypad_shutdown();
   printf("  - Speaking goodbye...\n");
   speech_say_text("Goodbye");
   speech_wait_complete();
   printf("  - Stopping speech...\n");
   speech_shutdown();
   printf("  - Closing communication...\n");
   comm_close();
   printf("  - Done!\n");
   ```

**Verification:**
```bash
# Full test run
# 1. Start Firmware
# 2. Start phase0_test
# 3. Hear "Phase zero integration test ready"
# 4. Press keys - hear "You pressed X"
# 5. Hold keys - hear "You held X"
# 6. Press Ctrl+C
# 7. Hear "Goodbye"
# 8. Clean exit with no errors
```

---

## Final Checklist

- [ ] Step 1: Empty skeleton compiles
- [ ] Step 2: Makefile target works
- [ ] Step 3: Comm init connects to Firmware
- [ ] Step 4: Speech init works, startup announcement plays
- [ ] Step 5: Callback function compiles
- [ ] Step 6: Keypad init works, console shows key events
- [ ] Step 7: Key presses trigger "You pressed X" speech
- [ ] Step 8: Key holds trigger "You held X" speech
- [ ] Step 9: Clean startup/shutdown sequence

---

## Regression Test Script (Future)

After the integration test is complete, create a regression script similar to:
- `Documentation/scripts/Regression_Phase0_Integration.sh`

This script would:
1. Build Software2
2. Start Firmware in background
3. Start phase0_test
4. Verify initialization messages appear
5. (Optionally) Simulate key presses or prompt user
6. Capture results and report PASS/FAIL

---

## Notes

- Each step is designed to be independently compilable
- Run `make phase0_test` after each step to verify no compile errors
- If a step fails, the previous step's code should still compile
- The callback receives both press and hold events; handle them appropriately to avoid speaking both "pressed" AND "held" for the same key interaction (current design speaks "pressed" immediately, then "held" after threshold)
