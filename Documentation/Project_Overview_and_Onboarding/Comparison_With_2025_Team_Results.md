# Comparison with 2025 Team Results

## Overview

This document describes how the original 2025 team Software layer was intended to work, what features were implemented, and how the proposed "fresh start" approach differs.

---

## Original Architecture (2025 Team)

### Block Diagram

```
┌────────────────────────────────────────────────────────────────────────────┐
│                              SOFTWARE LAYER                                 │
├────────────────────────────────────────────────────────────────────────────┤
│                                                                            │
│  ┌──────────────┐    ┌─────────────────┐    ┌─────────────────────────┐   │
│  │  Startup.c   │───►│ StateMachine.c  │───►│     ModeRouting.c       │   │
│  │              │    │                 │    │  (dynamic .so loading)  │   │
│  └──────────────┘    └────────┬────────┘    └───────────┬─────────────┘   │
│                               │                         │                  │
│                               ▼                         ▼                  │
│                    ┌────────────────────────────────────────────────────┐  │
│                    │               Modes (libModes.so)                  │  │
│                    │  ┌───────────┐ ┌──────────┐ ┌───────────────────┐  │  │
│                    │  │NormalMode │ │FrequenSy │ │ ConfigMode        │  │  │
│                    │  │(1016 lines│ │Mode      │ │ MonitoringMode    │  │  │
│                    │  │ 51KB!)    │ │          │ │ DummyDTMFMode     │  │  │
│                    │  └───────────┘ └──────────┘ └───────────────────┘  │  │
│                    └────────────────────────────────────────────────────┘  │
│                                         │                                  │
│  ┌───────────────────────────────┐      │      ┌──────────────────────┐   │
│  │ FirmwareCommunication.c       │◄─────┴─────►│ HamlibWrappedFuncs   │   │
│  │ - sendSpeakerOutput()         │             │ - get_level()        │   │
│  │ - firmwarePlayAudio()         │             │ - set_freq()         │   │
│  │ - Audio hashmap caching       │             │ - get_current_vfo()  │   │
│  └───────────────┬───────────────┘             └──────────────────────┘   │
│                  │                                                        │
└──────────────────┼────────────────────────────────────────────────────────┘
                   │ Named Pipes
                   ▼
┌──────────────────────────────────────────────────────────────────────────┐
│                           FIRMWARE LAYER                                  │
│  Firmware_i / Firmware_o pipes                                           │
│  Keypad_i / Keypad_o pipes                                               │
│  Speaker_i / Speaker_o pipes                                             │
└──────────────────────────────────────────────────────────────────────────┘
```

---

## Features Implemented (2025 Team)

### Bootup Flow (`StateMachine.c`)
- **Radio Selection Wizard**: Multi-step flow to select company → model → serial port
- **Save File Loading**: Load settings from numbered save files (1-9)
- **Multi-Radio Support**: Link multiple radios in one session
- **Company/Model Files**: Read from `StartupFiles/Company_List.txt`, `*_Model.txt`, `*_Hamlib.txt`

### Mode System (`ModeRouting.c`)
- **Dynamic Mode Loading**: Uses `dlopen()` to load modes from `libModes.so` at runtime
- **HashMap Storage**: Modes stored in HashMap for O(1) lookup by name
- **Programmable Hotkeys**: Keys C/D can be bound to any mode

### Implemented Modes

| Mode | File | Lines | Status |
|------|------|-------|--------|
| Normal | `NormalMode.c` | 1016 | Extensive Hamlib queries |
| Frequency | `FrequensyMode.c` | 97 | Basic frequency entry |
| Config | `ConfigMode.c` | 177 | Settings configuration |
| Monitoring | `MonitoringMode.c` | 245 | Radio monitoring |
| DummyDTMF | `DummyDTMFMode.c` | 83 | Placeholder DTMF mode |

### Normal Mode Features (`NormalMode.c` - 1016 lines!)
The massive Normal Mode implements extensive Hamlib queries:
- VFO A/B/C frequency queries
- Get/Set: Mode, PTT, RIT/XIT, VOX, AGC, Squelch
- Get/Set: Noise Reduction, Noise Blanker, Compression
- Get/Set: IF Shift, AF/RF levels
- Shift key modifiers for alternate functions
- Hold key modifiers for set operations

### Firmware Communication (`FirmwareCommunication.c`)
- **Pipe-Based IPC**: Communicates with Firmware via named pipes
- **Audio Caching**: HashMap of pre-generated audio files
- **Dynamic TTS**: Falls back to Festival if no cached audio
- **Save/Play Mode**: Generates and caches new audio files
- **Dictionary Substitution**: Replaces words before speaking

---

## Key Architectural Decisions (2025 Team)

### 1. Dynamic Mode Loading via Shared Library
```c
// ModeRouting.c - loads modes at runtime
Mode* dynamicalyLoadInModeByName(char* functionName) {
    lib_handle = dlopen("./libModes.so", RTLD_LAZY | RTLD_GLOBAL);
    modeCreateFunction = dlsym(lib_handle, functionName);
    return modeCreateFunction();
}
```
**Pros:** Hot-swappable modes, modular design
**Cons:** Complex build, dlopen/dlsym errors hard to debug

### 2. Header Files Include Their Own .c Files
```c
// Firmware/audio_firmware.h
#ifndef SHAREDLIB
#include "audio_firmware.c"
#endif
```
**Intended:** Single compilation unit per component
**Result:** Compilation nightmares, redefinition errors

### 3. Global State in Mode Files
```c
// NormalMode.c
int retcode; 
bool enteringValue = false;
char inputValue[100] = ""; 
HamlibSetFunction currentInputFunction;
```
Extensive global state makes testing difficult.

---

## Fresh Start Differences

| Aspect | 2025 Team | Fresh Start |
|--------|-----------|-------------|
| **Mode Loading** | Dynamic `dlopen()` from `.so` | Static linking, compile-time |
| **Code Size** | NormalMode.c = 1016 lines | Modular, ~100-200 lines/mode |
| **Build System** | Complex shared lib + headers including .c | Simple Makefile, standard includes |
| **Radio Polling** | Not implemented | Dedicated polling thread |
| **Key Beeps** | Not implemented in Software | ALSA dmix or Firmware-based |
| **Speech Backend** | Festival only | Abstracted (Festival/Piper) |
| **Radio Selection** | Multi-step wizard | Simple config.h / Config Mode |
| **State Management** | Global variables | Per-mode structs |
| **Auto-Announce** | Partially in firmware | Polling thread in Software |

---

## What Worked Well (Keep)

1. **Pipe Protocol**: The Firmware ↔ Software pipe communication works
2. **Hamlib Integration**: `HamlibWrappedFunctions/*` provides good abstractions
3. **Audio Caching Concept**: Pre-generating common phrases is smart
4. **Mode Concept**: Clear separation between operational modes
5. **KeyPress Struct**: Captures key, shift, hold in one object

---

## What Caused Problems (Change)

1. **#include "file.c" Pattern**: Causes redefinition errors
2. **1000+ Line Switch Statements**: Unmaintainable (`NormalMode.c`)
3. **Dynamic Loading**: Complex, fragile, hard to debug
4. **Global State**: Makes unit testing nearly impossible
5. **Missing Polling**: No automatic announcement of radio changes
6. **Complex Bootup Wizard**: Overkill for single-radio use

---

## Migration Path

For features that overlap (Frequency Mode, Normal Mode queries):

1. **Extract Hamlib wrappers** from `HamlibWrappedFunctions/` - they're usable
2. **Rewrite mode logic** with per-mode state structs
3. **Use simple switch cases** but in smaller, focused functions
4. **Add polling thread** before implementing Normal Mode
5. **Test each mode independently** before integrating

---

## Summary

The 2025 team built an ambitious system with dynamic mode loading, multi-radio support, and extensive Hamlib integration. However, unconventional C patterns (headers including .c files) and monolithic mode implementations made it fragile and hard to maintain.

The fresh start preserves the good ideas (pipe protocol, Hamlib wrappers, modes) while simplifying the build system and using standard C practices. The main additions are a polling thread for radio change detection and audio mixing for key beeps.
