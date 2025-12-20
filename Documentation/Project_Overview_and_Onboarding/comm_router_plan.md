# Comm Router Implementation Plan

> **Branch**: `feature/comm-router`  
> **Goal**: Fix packet type conflicts between keypad and speech threads by implementing a dedicated router thread that reads all Firmware responses and dispatches them to the correct handler.

---

## Overview

### The Problem
Currently, `keypad.c` and `speech.c` both read from `Firmware_o`. When one thread reads a packet intended for the other, responses get misrouted.

### The Solution
Implement a **router thread** in `comm.c` that:
1. Reads ALL packets from `Firmware_o`
2. Dispatches packets to type-specific queues (KEYPAD, AUDIO, CONFIG)
3. Modules wait on their queue instead of reading the pipe directly

```
Firmware_o  →  [Router Thread]  →  KEYPAD queue → keypad.c reads
             (reads all packets)    AUDIO queue  → speech.c reads
                                   CONFIG queue → comm.c handles
```

---

## Design Decisions

### Timeout Values
- **Keypad response timeout**: 5 seconds (plenty for simple request/response)
- **Audio response timeout**: 30 seconds (speech can take 10+ seconds for long phrases)
- **Router health check**: Every 1 second, verify router thread is alive

### Error Recovery
To avoid hangs and crashes, implement the following safeguards:

1. **Router thread failure detection**: Main code periodically checks if router is alive; if dead, log error and attempt restart
2. **Queue overflow protection**: If a queue is full, drop oldest packet and log warning (prevents memory exhaustion)
3. **Timeout on all blocking waits**: Never wait forever; return `HAMPOD_TIMEOUT` after limit
4. **Graceful degradation**: If router fails to start, fall back to direct pipe reading (current workaround mode)

### Why imitation_software Single-Threaded I/O is Valid
The `imitation_software` tests **Firmware functionality**, not Software2 architecture. Since Firmware:
- Handles packet routing internally (keypad↔audio)
- Uses the same code paths regardless of client threading
- Processes packets identically whether from single or multi-threaded client

...the imitation_software remains a valid Firmware regression test. The router thread is a **Software2 concern only** - it solves a problem in how Software2 consumes packets, not how Firmware produces them.

---

## Branch Workflow

```bash
# Create branch before any changes
git checkout -b feature/comm-router

# ... implement steps ...

# Run regression tests
# If all pass, merge to main
git checkout main
git merge feature/comm-router
git push
```

---

## Implementation Steps

### Step 1: Create Branch and Stub Router Types [COMPILE CHECK]

**Goal**: Define queue structures and function signatures without implementing them.

**Files to modify:**
- `Software2/include/comm.h` - Add queue types and new function declarations

**Changes:**
```c
// Add to comm.h

// Response queue for each packet type
#define COMM_RESPONSE_QUEUE_SIZE 16

// Register a callback to receive packets of a specific type
// Instead of callbacks, we'll use blocking queue reads
int comm_wait_keypad_response(CommPacket* packet, int timeout_ms);
int comm_wait_audio_response(CommPacket* packet, int timeout_ms);

// Start/stop the router thread
int comm_start_router(void);
void comm_stop_router(void);
```

**Verification**: `make phase0_test` compiles (functions not called yet)

---

### Step 2: Implement Response Queues [COMPILE CHECK]

**Goal**: Create thread-safe circular buffer queues for response packets.

**Files to modify:**
- `Software2/src/comm.c` - Add response queue implementation

**Changes:**
- Add `ResponseQueue` struct with mutex and condition variable
- Add `queue_init()`, `queue_push()`, `queue_pop_timeout()` for response queues
- Create 3 queues: `keypad_queue`, `audio_queue`, `config_queue`

**Verification**: `make phase0_test` compiles

---

### Step 3: Implement Router Thread [COMPILE CHECK]

**Goal**: Create the router thread that reads from Firmware_o and dispatches.

**Files to modify:**
- `Software2/src/comm.c`

**Changes:**
```c
static void* router_thread_func(void* arg) {
    while (router_running) {
        CommPacket packet;
        int result = comm_read_packet(&packet);
        
        if (result != HAMPOD_OK) {
            if (!router_running) break;  // Expected shutdown
            LOG_ERROR("Router: Pipe read failed, retrying...");
            usleep(100000);  // 100ms before retry
            continue;
        }
        
        switch (packet.type) {
            case PACKET_KEYPAD:
                if (queue_push(&keypad_queue, &packet) != HAMPOD_OK) {
                    LOG_ERROR("Router: Keypad queue full, dropping packet");
                }
                break;
            case PACKET_AUDIO:
                if (queue_push(&audio_queue, &packet) != HAMPOD_OK) {
                    LOG_ERROR("Router: Audio queue full, dropping packet");
                }
                break;
            case PACKET_CONFIG:
                if (queue_push(&config_queue, &packet) != HAMPOD_OK) {
                    LOG_ERROR("Router: Config queue full, dropping packet");
                }
                break;
            default:
                LOG_ERROR("Router: Unknown packet type %d", packet.type);
                break;
        }
    }
    LOG_INFO("Router thread exiting");
    return NULL;
}
```

- Implement `comm_start_router()` to spawn thread
- Implement `comm_stop_router()` to join thread

**Verification**: `make phase0_test` compiles

---

### Step 4: Implement Blocking Queue Readers [COMPILE CHECK]

**Goal**: Add functions that wait for specific packet types.

**Files to modify:**
- `Software2/src/comm.c`

**Changes:**
- Implement `comm_wait_keypad_response(packet, timeout_ms)` - pops from keypad_queue (5000ms timeout)
- Implement `comm_wait_audio_response(packet, timeout_ms)` - pops from audio_queue (30000ms timeout)
- Use `pthread_cond_timedwait` for timeout support
- Return `HAMPOD_TIMEOUT` if timeout expires (caller can retry or handle gracefully)

**Verification**: `make phase0_test` compiles

---

### Step 5: Unit Test for Router Queue [NEW TEST]

**Goal**: Create a simple unit test for the response queue logic.

**Files to create:**
- `Software2/tests/test_comm_queue.c`

**Test logic:**
1. Create a mock that pushes packets to queues
2. Verify queue ordering (FIFO)
3. Verify timeout behavior
4. Verify thread safety with concurrent push/pop

**Verification**: 
```bash
cd Software2 && make tests
./bin/test_comm_queue  # Should print PASS/FAIL
```

---

### Step 6: Update comm_read_keypad() to Use Router [KEY CHANGE]

**Goal**: Change `comm_read_keypad()` to use the new queue instead of reading pipe directly.

**Files to modify:**
- `Software2/src/comm.c`

**Changes:**
```c
int comm_read_keypad(char* key_out) {
    // Send request (unchanged)
    CommPacket request = { .type = PACKET_KEYPAD, ... };
    comm_send_packet(&request);
    
    // Wait for response from queue (NEW)
    CommPacket response;
    if (comm_wait_keypad_response(&response, 5000) != HAMPOD_OK) {
        return HAMPOD_TIMEOUT;
    }
    
    *key_out = (char)response.data[0];
    return HAMPOD_OK;
}
```

**Verification**: `make phase0_test` compiles

---

### Step 7: Update speech.c to Use Router [KEY CHANGE]

**Goal**: Change speech thread to wait for audio responses via queue.

**Files to modify:**
- `Software2/src/speech.c`

**Changes:**
- Change `comm_send_audio()` call to `comm_send_audio_sync()` which uses router
- Or add internal call to `comm_wait_audio_response()`

**Verification**: `make phase0_test` compiles

---

### Step 8: Update comm_init() and comm_close() [KEY CHANGE]

**Goal**: Start router thread on init, stop on close.

**Files to modify:**
- `Software2/src/comm.c`

**Changes:**
```c
int comm_init(void) {
    // ... existing pipe open logic ...
    
    // Initialize response queues
    response_queue_init(&keypad_queue);
    response_queue_init(&audio_queue);
    response_queue_init(&config_queue);
    
    // Start router thread
    return comm_start_router();
}

void comm_close(void) {
    // Stop router thread first
    comm_stop_router();
    
    // ... existing pipe close logic ...
    
    // Destroy queues
    response_queue_destroy(&keypad_queue);
    response_queue_destroy(&audio_queue);
    response_queue_destroy(&config_queue);
}
```

**Verification**: `make phase0_test` compiles and runs

---

### Step 9: Test Phase 0.9 Integration [REGRESSION TEST]

**Goal**: Verify the integration test works with the new architecture.

**Verification:**
```bash
# On Raspberry Pi
cd ~/HAMPOD2026
git fetch origin
git checkout feature/comm-router
git pull

# Build Software2
cd Software2 && make clean && make phase0_test

# Run integration test
cd ~/HAMPOD2026/Firmware && sudo ./firmware.elf &
sleep 3
cd ~/HAMPOD2026/Software2 && sudo ./bin/phase0_test
# Press keys, verify speech
# Ctrl+C to exit
sudo killall firmware.elf
```

**Expected behavior:**
- Startup announcement plays
- Key presses trigger "You pressed X" speech
- Key holds trigger "You held X" speech
- No packet type mismatch errors

---

### Step 10: Run Imitation Software Regression Test [REGRESSION TEST]

**Goal**: Verify the Firmware + imitation_software still works.

> [!NOTE]
> The imitation_software uses single-threaded send/receive pattern. This is **still a valid Firmware test** because:
> - Firmware handles the same packet routing internally regardless of client threading
> - The router thread is a Software2-only architecture concern
> - imitation_software tests Firmware functionality, not Software2 design

**Verification:**
```bash
# On Raspberry Pi
cd ~/HAMPOD2026
./Documentation/scripts/Regression_Imitation_Software.sh
```

**Expected result:** PASS

---

### Step 11: Run HAL Integration Regression Test [REGRESSION TEST]

**Goal**: Verify the HAL integration test still works.

> [!NOTE]
> The HAL integration test (`test_hal_integration`) doesn't use Software2 at all - it tests HAL directly. Should be unaffected.

**Verification:**
```bash
# On Raspberry Pi
cd ~/HAMPOD2026
./Documentation/scripts/Regression_HAL_Integration.sh
```

**Expected result:** PASS

---

### Step 12: Merge to Main [BRANCH MERGE]

**Goal**: If all tests pass, merge the feature branch back to main.

**Verification:**
```bash
# On development machine
git checkout main
git pull
git merge feature/comm-router
git push

# Sync Pi
ssh waynesr@HAMPOD.local "cd ~/HAMPOD2026 && git checkout main && git pull"
```

---

## Summary of Files Changed

| File | Change Type | Description |
|------|-------------|-------------|
| `Software2/include/comm.h` | MODIFY | Add queue types and router functions |
| `Software2/src/comm.c` | MODIFY | Add router thread, queues, wait functions |
| `Software2/src/speech.c` | MODIFY | Use audio response queue |
| `Software2/tests/test_comm_queue.c` | NEW | Unit test for queue logic |

---

## Rollback Plan

If issues are discovered after merge:
```bash
git revert HEAD  # Revert the merge commit
git push
```

Or reset to previous commit:
```bash
git log --oneline  # Find last good commit
git reset --hard <commit-hash>
git push --force-with-lease
```

---

## Checklist

- [x] Step 1: Create branch, add type stubs ✅ (2025-12-20)
- [x] Step 2: Implement response queues ✅ (2025-12-20)
- [x] Step 3: Implement router thread ✅ (2025-12-20)
- [x] Step 4: Implement blocking queue readers ✅ (2025-12-20)
- [ ] Step 5: Create unit test for queue
- [x] Step 6: Update comm_read_keypad() ✅ (2025-12-20)
- [x] Step 7: Update speech.c ✅ (2025-12-20)
- [x] Step 8: Update comm_init()/comm_close() ✅ (2025-12-20)
- [x] Step 9: Test Phase 0.9 integration ✅ (2025-12-20)
- [x] Step 10: Run Imitation Software regression ✅ (2025-12-20)
- [ ] Step 11: Run HAL Integration regression
- [ ] Step 12: Merge to main
