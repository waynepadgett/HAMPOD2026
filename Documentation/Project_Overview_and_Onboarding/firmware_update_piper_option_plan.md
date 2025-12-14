# Firmware Piper TTS Integration Plan

## Goal
Add compile-time option to use **Piper TTS** (persistent mode, low-quality model) as an alternative to **Festival**. Default to Piper with speed=1.0.

## Background

The `Speech_Comparison/speech_latency_test.c` already implements working Piper persistent mode with these key elements:
- Persistent pipe opened with `popen()` to a `piper | aplay` pipeline
- Text written via `fprintf()` + `fflush()` for zero-latency synthesis
- Speed control via `--length_scale` parameter
- Uses low-quality model with 16kHz sample rate

This plan adapts that proven approach into the main Firmware codebase as a compile-time option.

## User Review Required

> [!IMPORTANT]
> **Piper Installation**: Piper and the voice model will be installed via a script (`install_piper.sh`). The install script downloads a specific version to ensure consistency across all HAMPOD devices.

## Proposed Changes

### Phase 1: Create Piper Installation Script

#### [NEW] Documentation/scripts/install_piper.sh
A bash script that:
1. Downloads a pinned version of Piper (linux-aarch64) from GitHub releases
2. Extracts to `~/piper` or prompts for custom location
3. Creates symlink in `/usr/local/bin/piper` (with sudo)
4. Downloads the `en_US-lessac-low` voice model to `Firmware/models/`
5. Verifies installation by running `piper --version`

```bash
#!/bin/bash
set -e

PIPER_VERSION="2023.11.14-2"
PIPER_URL="https://github.com/rhasspy/piper/releases/download/${PIPER_VERSION}/piper_linux_aarch64.tar.gz"
MODEL_URL="https://huggingface.co/rhasspy/piper-voices/resolve/main/en/en_US/lessac/low/en_US-lessac-low.onnx"
MODEL_JSON_URL="https://huggingface.co/rhasspy/piper-voices/resolve/main/en/en_US/lessac/low/en_US-lessac-low.onnx.json"

INSTALL_DIR="${HOME}/piper"
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
FIRMWARE_DIR="$(dirname "$(dirname "$SCRIPT_DIR")")/Firmware"

echo "=== HAMPOD Piper TTS Installer ==="
echo "Piper Version: ${PIPER_VERSION}"
echo ""

# 1. Download and extract Piper
echo "Downloading Piper..."
wget -q --show-progress -O /tmp/piper.tar.gz "${PIPER_URL}"

echo "Extracting to ${INSTALL_DIR}..."
mkdir -p "${INSTALL_DIR}"
tar -xzf /tmp/piper.tar.gz -C "${INSTALL_DIR}" --strip-components=1
rm /tmp/piper.tar.gz

# 2. Create symlink
echo "Creating symlink in /usr/local/bin..."
sudo ln -sf "${INSTALL_DIR}/piper" /usr/local/bin/piper

# 3. Download voice model
echo "Downloading voice model..."
mkdir -p "${FIRMWARE_DIR}/models"
wget -q --show-progress -O "${FIRMWARE_DIR}/models/en_US-lessac-low.onnx" "${MODEL_URL}"
wget -q --show-progress -O "${FIRMWARE_DIR}/models/en_US-lessac-low.onnx.json" "${MODEL_JSON_URL}"

# 4. Verify
echo ""
echo "Verifying installation..."
piper --version

echo ""
echo "=== Installation Complete ==="
echo "Piper installed to: ${INSTALL_DIR}"
echo "Model installed to: ${FIRMWARE_DIR}/models/"
```

#### [NEW] Firmware/models/.gitignore
Ignore the large `.onnx` files (downloaded by install script):
```
*.onnx
*.onnx.json
```

**Verification:**
- Run `./install_piper.sh` on a fresh Pi
- `which piper` returns `/usr/local/bin/piper`
- `ls Firmware/models/` shows the model files

---

### Phase 2: Create TTS HAL Interface

#### [NEW] Firmware/hal/hal_tts.h
```c
#ifndef HAL_TTS_H
#define HAL_TTS_H

/**
 * Initialize TTS subsystem.
 * For Piper: checks for 'piper' in PATH, then starts the persistent pipeline.
 * @return 0 on success, -1 on failure (prints guidance to stderr)
 */
int hal_tts_init(void);

/**
 * Speak text (blocking for Festival, async for Piper persistent).
 * @param text The text to speak
 * @param output_file Optional output file path. NULL for direct playback.
 * @return 0 on success, -1 on failure
 */
int hal_tts_speak(const char* text, const char* output_file);

/**
 * Cleanup TTS resources.
 */
void hal_tts_cleanup(void);

/**
 * Get TTS implementation name (e.g., "Festival", "Piper")
 */
const char* hal_tts_get_impl_name(void);

#endif
```

**Verification:**
- Compiles with `make` (header only, no implementation yet)

---

### Phase 3: Implement Festival TTS HAL

#### [NEW] Firmware/hal/hal_tts_festival.c
```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hal_tts.h"
#include "hal_audio.h"

int hal_tts_init(void) {
    // Festival needs no persistent state
    // Optionally check for 'text2wave' command
    if (system("which text2wave > /dev/null 2>&1") != 0) {
        fprintf(stderr, "HAL TTS ERROR: Festival 'text2wave' not found.\n");
        fprintf(stderr, "Install with: sudo apt-get install festival festvox-kallpc16k\n");
        return -1;
    }
    return 0;
}

int hal_tts_speak(const char* text, const char* output_file) {
    char command[1024];
    const char* out = output_file ? output_file : "/tmp/hampod_speak.wav";
    snprintf(command, sizeof(command), 
             "echo '%s' | text2wave -o '%s'", text, out);
    int result = system(command);
    if (result != 0) return -1;
    return hal_audio_play_file(out);
}

void hal_tts_cleanup(void) {
    // No cleanup needed
}

const char* hal_tts_get_impl_name(void) {
    return "Festival";
}
```

**Verification:**
- Compile with `make TTS_ENGINE=festival`
- Run firmware → speaks with Festival

---

### Phase 4: Implement Piper TTS HAL (Persistent Mode)

#### [NEW] Firmware/hal/hal_tts_piper.c
- Checks for `piper` in PATH at init
- If missing, prints actionable error message guiding user to run install script
- Checks for model file presence

```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "hal_tts.h"
#include "hal_audio.h"

#ifndef PIPER_MODEL_PATH
#define PIPER_MODEL_PATH "models/en_US-lessac-low.onnx"
#endif

#ifndef PIPER_SPEED
#define PIPER_SPEED "1.0"
#endif

static FILE *persistent_pipe = NULL;

int hal_tts_init(void) {
    // 1. Check if 'piper' is installed
    if (system("which piper > /dev/null 2>&1") != 0) {
        fprintf(stderr, "\n");
        fprintf(stderr, "===================================================\n");
        fprintf(stderr, "ERROR: Piper TTS is not installed!\n");
        fprintf(stderr, "===================================================\n");
        fprintf(stderr, "To install Piper and the voice model, run:\n");
        fprintf(stderr, "\n");
        fprintf(stderr, "    ./Documentation/scripts/install_piper.sh\n");
        fprintf(stderr, "\n");
        fprintf(stderr, "Or build with Festival instead:\n");
        fprintf(stderr, "    make TTS_ENGINE=festival\n");
        fprintf(stderr, "===================================================\n");
        return -1;
    }

    // 2. Check if model file exists
    if (access(PIPER_MODEL_PATH, F_OK) != 0) {
        fprintf(stderr, "\n");
        fprintf(stderr, "===================================================\n");
        fprintf(stderr, "ERROR: Piper voice model not found!\n");
        fprintf(stderr, "Expected: %s\n", PIPER_MODEL_PATH);
        fprintf(stderr, "===================================================\n");
        fprintf(stderr, "To download the model, run:\n");
        fprintf(stderr, "    ./Documentation/scripts/install_piper.sh\n");
        fprintf(stderr, "===================================================\n");
        return -1;
    }

    // 3. Start persistent pipeline
    char command[1024];
    const char* audio_dev = hal_audio_get_device();
    
    snprintf(command, sizeof(command), 
             "piper --model %s --length_scale %s --output_raw | aplay -D %s -r 16000 -f S16_LE -t raw -",
             PIPER_MODEL_PATH, PIPER_SPEED, audio_dev);
    
    persistent_pipe = popen(command, "w");
    if (persistent_pipe == NULL) {
        fprintf(stderr, "HAL TTS: Failed to start Piper pipeline\n");
        return -1;
    }
    
    printf("HAL TTS: Piper initialized (speed=%s)\n", PIPER_SPEED);
    return 0;
}

int hal_tts_speak(const char* text, const char* output_file) {
    (void)output_file; // Ignored in persistent mode
    if (persistent_pipe == NULL) return -1;
    fprintf(persistent_pipe, "%s\n", text);
    fflush(persistent_pipe);
    return 0;
}

void hal_tts_cleanup(void) {
    if (persistent_pipe) {
        pclose(persistent_pipe);
        persistent_pipe = NULL;
    }
}

const char* hal_tts_get_impl_name(void) {
    return "Piper (Persistent)";
}
```

**Verification:**
- Compile with `make` (Piper is default)
- Run without piper installed → Clear error message with install instructions
- Run after `install_piper.sh` → Speech output works

---

### Phase 5: Update Build System

#### [MODIFY] Firmware/makefile

Add compile-time switch, TTS HAL sources, **and a pre-build check for Piper**:

```makefile
# TTS Engine Selection (Default: Piper)
ifndef TTS_ENGINE
TTS_ENGINE = piper
endif

# TTS Speed (Default: 1.0)
ifndef TTS_SPEED
TTS_SPEED = 1.0
endif

# Select TTS HAL source based on engine
ifeq ($(TTS_ENGINE),festival)
TTS_SRC = hal/hal_tts_festival.c
TTS_FLAGS = -DUSE_FESTIVAL
else
TTS_SRC = hal/hal_tts_piper.c
TTS_FLAGS = -DUSE_PIPER -DPIPER_SPEED=\"$(TTS_SPEED)\"
endif

CFLAGS += $(TTS_FLAGS)
HAL_SRCS += $(TTS_SRC)

# Pre-build check for Piper (only when building with Piper)
.PHONY: check-piper
check-piper:
ifeq ($(TTS_ENGINE),piper)
	@which piper > /dev/null 2>&1 || { \
		echo ""; \
		echo "================================================="; \
		echo "WARNING: 'piper' not found in PATH."; \
		echo "The firmware will fail at runtime."; \
		echo "================================================="; \
		echo "To install Piper, run:"; \
		echo "    ./Documentation/scripts/install_piper.sh"; \
		echo ""; \
		echo "Or build with Festival:"; \
		echo "    make TTS_ENGINE=festival"; \
		echo "================================================="; \
		exit 1; \
	}
endif

# Add check-piper as dependency of main target
firmware.elf: check-piper $(OBJS)
	...
```

**Verification:**
- `make` on Pi without Piper → Warning message, build aborts
- `make` after install → Builds successfully
- `make TTS_ENGINE=festival` → Builds without Piper check

---

### Phase 6: Integrate TTS HAL into Audio Firmware

#### [MODIFY] Firmware/audio_firmware.c

Replace direct `system("echo ... | text2wave ...")` calls with `hal_tts_speak()`:

**Changes:**
1. Add `#include "hal/hal_tts.h"`
2. In `audio_process()`, call `hal_tts_init()` after `hal_audio_init()`
3. Replace 'd' mode logic:
   ```c
   // OLD:
   sprintf(buffer, "echo '%s' | text2wave -o output.wav", remaining_string);
   system_result = system(buffer);
   system_result = hal_audio_play_file("output.wav");
   
   // NEW:
   system_result = hal_tts_speak(remaining_string, NULL);
   ```
4. Replace 's' mode logic similarly
5. Add `hal_tts_cleanup()` at end of `audio_process()`

**Verification:**
- Compile with `make` → no errors
- Run `imitation_software` test → audio plays with Piper
- Compile with `make TTS_ENGINE=festival` → audio plays with Festival

---

### Phase 7: Update Documentation

#### [MODIFY] Firmware/BUILD.md

Add section on TTS engine selection and Piper installation:
```markdown
## TTS Engine Selection

The firmware supports two text-to-speech engines:

### Piper (Default) - Recommended
- Uses Piper with persistent pipeline for zero-latency response
- Higher quality voice than Festival
- Requires one-time installation

**Install Piper:**
```bash
./Documentation/scripts/install_piper.sh
```

**Build with Piper (default):**
```bash
make
```

**Build with custom speed (1.0 = normal, 0.8 = faster):**
```bash
make TTS_SPEED=0.8
```

### Festival (Legacy)
- Uses Festival `text2wave` command
- Lower quality but pre-installed on most Linux systems

**Build with Festival:**
```bash
make TTS_ENGINE=festival
```
```

#### [MODIFY] Documentation/Hampod_RPi_change_plan.md
Add Phase 4.6 entry for TTS engine selection.

---

## Verification Plan

### Automated Tests
1. **Compile Test (Piper):** `make clean && make` → exit 0 (after install_piper.sh)
2. **Compile Test (Festival):** `make clean && make TTS_ENGINE=festival` → exit 0
3. **Integration Test:** `./run_remote_test.sh` → TEST MARKER: SUCCESS

### Manual Verification
1. Fresh Pi → `make` fails with install guidance
2. Run `install_piper.sh` → Piper and model downloaded
3. `make` → Builds successfully
4. Run firmware → Speaks with Piper voice
5. `make TTS_ENGINE=festival` → Speaks with Festival voice

---

## Incremental Checkpoints

| Phase | Compiles | Testable | Description |
|-------|----------|----------|-------------|
| 1     | N/A      | `install_piper.sh` works | Install script |
| 2     | ✓        | Header only | TTS HAL interface |
| 3     | ✓        | Festival works | Festival implementation |
| 4     | ✓        | Piper works | Piper implementation |
| 5     | ✓        | Compile-time switch + Piper check | Makefile updates |
| 6     | ✓        | Full integration | Audio firmware integration |
| 7     | N/A      | Docs updated | Documentation |
