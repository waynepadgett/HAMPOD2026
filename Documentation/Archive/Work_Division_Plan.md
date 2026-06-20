# HAMPOD Work Division Plan

This document divides the remaining HAMPOD work into three parallel tracks designed to minimize conflicts.

## Overview

The work is divided into three tracks:

1. **Student Track** - Core HAMPOD features, 10 weeks at a few hours per week
2. **Engineer Track A** - Testing infrastructure (no main code changes)
3. **Engineer Track B** - Hardware and OS experiments (no main code changes)

**Critical Rule**: Only the student modifies the main HAMPOD codebase during the 10-week period. Engineers work on isolated projects that don't touch the core code.


## Track 1: Student Developer

**Goal**: Learn embedded systems development through implementing real features

**Time commitment**: 3-5 hours per week for 10 weeks

**Prerequisites**: Basic C programming, ability to SSH into a Raspberry Pi

### Phase 1: Setup and Familiarization (Weeks 1-2)

**Week 1: Environment Setup**
- Set up development environment using RPi_Setup_Guide.md
- Run regression tests to verify setup works
- Read through README.md and ICOMReaderManual2.md

**Week 2: Code Exploration**
- Trace what happens when a key is pressed
- Understand the path from keypad input to speech output
- Document which files handle each step

### Phase 2: Keypad Remapping System (Weeks 3-6)

**Week 3: Design the keypad.conf Format**

Create a simple, human-readable configuration file for keypad mapping. The format should be:
- Plain ASCII, comma-separated values
- 4 key values per line (one row of the keypad)
- Maps physical keys to logical HamPod keys

Example keypad.conf:
```
# HAMPOD Keypad Configuration
# Each line is one row of the physical keypad
# Format: key1, key2, key3, key4
# Valid keys: 0-9, A, B, C, D, *, #, X, Y
# The two 0 keys must both be 0 (hardware limitation)
# Enter (#) can only appear once

1, 2, 3, A
4, 5, 6, B
7, 8, 9, C
*, 0, #, D
```

Design notes:
- The two zero positions on some keypads are indistinguishable in hardware, so both must map to 0
- The Enter key (#) can only be defined once
- X and Y are reserved for future expansion keys

**Week 4: Implement keypad.conf Parser**
- Read keypad.conf at startup
- Parse the CSV format
- Store the mapping in a lookup table
- Handle errors (missing file uses default mapping)

**Week 5: Integrate with Key Handler**
- Modify key handling to use the mapping table
- Physical scan code → lookup → logical key
- Test with default and modified mappings

**Week 6: Testing and Edge Cases**
- Test invalid configurations
- Test missing file behavior
- Write documentation for keypad.conf

### Phase 3: Frequency Announcement Fix (Weeks 7-8)

**Week 7: Understand Current Behavior**
- Study how frequency readback works
- Find where the speech string is generated
- Document current "point" vs "dot" behavior

**Week 8: Implement the Fix**

Per ICOMReaderManual2.md Section 11.3:
- "point" is spoken for the main decimal (MHz separator)
- "dot" is spoken after the 1 kHz digit when there are more than 3 significant figures after the decimal

Examples:
- 14.250 → "fourteen point two five zero" (3 digits, no dot)
- 14.2505 → "fourteen point two five zero dot five" (dot before sub-kHz)
- 7.123456 → "seven point one two three dot four five six"

### Phase 4: Set Mode Key Behavior (Weeks 9-10)

**Week 9: Study Set Mode**
- Read ICOMReaderManual2.md Section 6
- Trace how [A] and [B] work as increment/decrement in Set Mode
- Identify what needs to change

**Week 10: Implement Fix**
- Fix increment/decrement key behavior to match specification
- Test with at least one Set Mode parameter
- Document the changes

### Deliverables

1. keypad.conf file format and parser
2. "dot" announcement fix for frequency readback
3. Set Mode increment/decrement key behavior
4. Documentation updates for all changes


## Track 2: Engineer A (Testing Infrastructure)

**Focus**: Tools and testing that don't modify core HAMPOD code

**Rule**: Do NOT modify any files in Firmware/ or Software2/src/

### Tasks

**Simulated Radio**
- Create a standalone program that responds to Hamlib commands
- Allows testing HAMPOD without physical radio hardware
- Could run on a second RPi or the same system on a different port
- Location: New directory Testing/simulated_radio/

**Lower Performance Board Testing**
- Test current HAMPOD on Raspberry Pi 3B
- Profile CPU and memory usage
- Document minimum hardware requirements
- Report issues but don't fix them (student will fix if needed)

### Files Owned

- Testing/simulated_radio/ (new directory)
- Documentation/hardware_compatibility.md (new file)

### Timeline

| Week | Task |
|------|------|
| 1-3 | Design simulated radio architecture |
| 4-6 | Implement simulated radio |
| 7-8 | Test on Pi 3B, document findings |
| 9-10 | Polish and document |


## Track 3: Engineer B (Hardware and OS Experiments)

**Focus**: Hardware experiments and OS hardening that don't touch core code

**Rule**: Do NOT modify any files in Firmware/ or Software2/src/

### Tasks

**Pimoroni Speaker Board**
- Experiment with Pimoroni speaker HAT or pHAT
- Document ALSA configuration needed
- Test audio quality for speech output
- Location: New directory Hardware/pimoroni_audio/

**RAM Disk OS Configuration**
- Research running Raspberry Pi OS from RAM disk
- Configuration files remain on SD card
- Protects against corruption on power-down
- Document the setup process
- Location: Documentation/ramdisk_os_setup.md

### Files Owned

- Hardware/pimoroni_audio/ (new directory)
- Documentation/ramdisk_os_setup.md (new file)

### Timeline

| Week | Task |
|------|------|
| 1-4 | Pimoroni speaker experiments |
| 5-8 | RAM disk OS research and testing |
| 9-10 | Document findings |


## Work Isolation Summary

### Student Owns (Weeks 1-10)

All core HAMPOD code:
- Firmware/*
- Software2/*
- Software2/config/keypad.conf (new)

### Engineers May NOT Touch

- Firmware/*
- Software2/*

### Engineers May Create/Modify

- Testing/* (new)
- Hardware/* (new)
- Documentation/*.md (new files only, not existing docs)


## Success Criteria

### Student Track
- keypad.conf works and allows remapping
- Frequency "dot" announcement is correct
- Set Mode increment/decrement works
- Changes are documented and tested

### Engineer Track A
- Simulated radio enables hardware-free testing
- Pi 3B compatibility is documented

### Engineer Track B
- Pimoroni speaker feasibility is documented
- RAM disk OS procedure is documented (even if not fully implemented)


## If Engineers Run Out of Work

That's fine! The engineer tasks are intentionally scoped smaller because:
1. Engineers have less time available
2. Avoiding code conflicts with the student is the priority
3. These are experimental/research tasks

If finished early, engineers can:
- Help review student code (without making changes)
- Expand documentation
- Research future features without implementing them
