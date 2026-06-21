# HAMPOD Documentation

> **Last Updated:** 2026-06-21

This is the central index for all HAMPOD documentation. Use this file to find what you need.

---

## Quick Links

| Document | Description |
|----------|-------------|
| [Project Plan](Planning/Fresh_Start_Master_Plan.md) | Master implementation plan with phase tracker |
| [RPi Setup Guide](Guides/RPi_Setup_Guide.md) | Get HAMPOD running on a Raspberry Pi |
| [System Architecture](Project_Spec/System_Architecture.md) | High-level system design |
| [CLI Help](Guides/CLI_Help.md) | Command-line interface reference |
| [Merge SOP](Guides/Merge_SOP.md) | How to merge branches to main |

---

## By Audience

### New Developers (Start Here)

1. [RPi Setup Guide](Guides/RPi_Setup_Guide.md) — Flash OS, install dependencies
2. [CLI Help](Guides/CLI_Help.md) — Start/stop/update HAMPOD
3. [Debugging](Guides/Debugging.md) — Logs, diagnostics, troubleshooting
4. [Multiple Modes Guide](Guides/Multiple_Modes_Guide.md) — How Normal, Frequency, Set modes work together
5. [Key Mapping Reference](Reference/Currently_Implemented_Keys.md) — What each key does

### Contributors

- [Merge SOP](Guides/Merge_SOP.md) — Full merge procedure
- [Merge SOP (Short)](Guides/Merge_SOP_Short.md) — Quick checklist
- [SOP Transcript](Guides/SOP_Merge_Transcript.md) — Training walkthrough
- [Planning/](Planning/) — Active work and upcoming tasks

### Management

- [Project Plan](Planning/Fresh_Start_Master_Plan.md) — Phase tracker with status
- [Comparison With 2025 Team](Reference/Comparison_With_2025_Team.md) — Why we rewrote
- [Commit History Analysis](Planning/Completed/Commit_History_Analysis.md) — Development timeline

---

## Planning

Active plans, in-progress work, and completed phases.

### Active Work


#### Scratch TODO notes
dad will work on next:
- test doyles new features and make punch list
then new feat:
- add status state flag for current radio's manufacturer b/c radios within manufacturer family usually have similar features. wants to make the key function handling check what radio manufacture so u can branch out to do 1 thing if kenwood and a diff thing if other etc. 


| Priority | Plan | What's Left | Effort |
|----------|------|-------------|--------|
| **P0** | [Silent_Killer_Bug_Hunt.md](Planning/Silent_Killer_Bug_Hunt.md) | Check `dmesg`/syslog after every restart for segfaults. Enable core dumps. Add signal handler to `main.c`. Reproduce with stress test. | M |
| **P0** | [Phase_3_Set_Mode_Plan.md](Planning/Phase_3_Set_Mode_Plan.md) | Create `test_set_mode.c` (8 test cases). Fix frequency "dot" format in `normal_mode.c` + `frequency_mode.c`. Update `Currently_Implemented_Keys.md`. | S |
| **P1** | [TTS_Caching_Plan.md](Planning/TTS_Caching_Plan.md) | Create `warmup_tts_cache.sh` script for common phrases. Implement LRU eviction (currently hard cap). | S/M |
| **P1** | [Startup_Device_Plan.md](Planning/Startup_Device_Plan.md) | Implement USB device enumeration (`hal_usb_util.c`). Add change detection for keypad/speaker port swaps. | L |
| **P1** | [Memory_Leak_Investigation.md](Planning/Memory_Leak_Investigation.md) | Analyze monitoring data to identify leak source. Memory usage growing from 215MB→237MB over 15min in logs. | XL |
| **P1** | [Multiple_Synthesizers_Plan.md](Planning/Multiple_Synthesizers_Plan.md) | Implement `hal_tts_flite.c`. Create engine dispatch layer. | L |
| **P2** | [Refactor_Todos.md](Planning/Refactor_Todos.md) | Radio layer helpers, config unit tests, dead code removal, script/path consistency. See plan for full list. | XL |
| **P2** | [Regression_Testing_Plan.md](Planning/Regression_Testing_Plan.md) | Living doc — add tests as features complete. No immediate action. | — |
| **P2** | [Fresh_Start_Master_Plan.md](Planning/Fresh_Start_Master_Plan.md) | Meta-tracker — update when phases complete. No direct work. | — |

### Completed Phases

| Document | Phase | Description |
|----------|-------|-------------|
| [Phase_0_Core_Infra_Plan.md](Planning/Completed/Phase_0_Core_Infra_Plan.md) | Phase 0 | Core infrastructure (comm, speech, keypad, radio, config) |
| [Phase_1_Freq_Mode_Plan.md](Planning/Completed/Phase_1_Freq_Mode_Plan.md) | Phase 1 | Frequency Mode implementation |
| [Phase_2_Normal_Mode_Plan.md](Planning/Completed/Phase_2_Normal_Mode_Plan.md) | Phase 2 | Normal Mode implementation |
| [RPi_Migration_Plan.md](Planning/Completed/RPi_Migration_Plan.md) | Migration | NanoPi → Raspberry Pi hardware migration |
| [Audio_Latency_Plan.md](Planning/Completed/Audio_Latency_Plan.md) | Audio | ALSA tuning, persistent audio device |
| [Interrupt_And_Piper_Plan.md](Planning/Completed/Interrupt_And_Piper_Plan.md) | Audio | Interruptible audio, persistent Piper TTS |
| [Firmware_Bug_Fix_Plan.md](Planning/Completed/Firmware_Bug_Fix_Plan.md) | Firmware | USB I/O bugs, device reset, watchdog |
| [Firmware_Piper_Option_Plan.md](Planning/Completed/Firmware_Piper_Option_Plan.md) | Firmware | Piper TTS integration |
| [Configuration_Function_Plan.md](Planning/Completed/Configuration_Function_Plan.md) | Config | config.c module (INI parser, undo) |
| [Config_Mode_Impl_Plan.md](Planning/Completed/Config_Mode_Impl_Plan.md) | Config | Config Mode implementation |
| [Startup_Shutdown_Protection_Plan.md](Planning/Completed/Startup_Shutdown_Protection_Plan.md) | System | Boot scripts, SD card protection |
| [Install_Script_Plan.md](Planning/Completed/Install_Script_Plan.md) | Tooling | Automated install script |
| [Overclocking_Plan.md](Planning/Completed/Overclocking_Plan.md) | Performance | CPU/GPU overclock settings |
| [Commit_History_Analysis.md](Planning/Completed/Commit_History_Analysis.md) | Analysis | Development timeline (64 commits) |
| [Comm_Router_Plan.md](Planning/Completed/Comm_Router_Plan.md) | Comm | Router thread architecture, all 12 steps done |
| [Integration_Test_Plan.md](Planning/Completed/Integration_Test_Plan.md) | Test | Phase 0 integration test, all 9 steps verified |
| [Firmware_Test_Fix.md](Planning/Completed/Firmware_Test_Fix.md) | Firmware | Firmware test fixes |
| [Documentation_Plan.md](Planning/Completed/Documentation_Plan.md) | Docs | Documentation refactor — all 4 tasks completed (guide, plan consolidation, archival, regression update) |
| [Set_Mode_Correction_Plan.md](Planning/Completed/Set_Mode_Correction_Plan.md) | Set Mode | Spec verification, beep system, [*] fix — content merged into Phase_3_Set_Mode_Plan |

---

## Guides

How-to documentation for setup, operation, and contribution.

| Document | Description |
|----------|-------------|
| [RPi_Setup_Guide.md](Guides/RPi_Setup_Guide.md) | Manual Raspberry Pi setup instructions |
| [RPi_Setup_Guide_Accessible.txt](Guides/RPi_Setup_Guide_Accessible.txt) | Accessible version of setup guide |
| [CLI_Help.md](Guides/CLI_Help.md) | Command-line interface usage |
| [Debugging.md](Guides/Debugging.md) | Debug logging, diagnostics, troubleshooting |
| [Multiple_Modes_Guide.md](Guides/Multiple_Modes_Guide.md) | How modes work together |
| [Currently_Implemented_Keys.md](Reference/Currently_Implemented_Keys.md) | Quick reference for all key bindings |
| [Merge_SOP.md](Guides/Merge_SOP.md) | Full merge standard operating procedure |
| [Merge_SOP_Short.md](Guides/Merge_SOP_Short.md) | Condensed merge checklist |
| [SOP_Merge_Transcript.md](Guides/SOP_Merge_Transcript.md) | Training transcript for merge process |
| [Git_Branch_Overview.md](Guides/Git_Branch_Overview.md) | Branch strategy and naming conventions |

---

## Reference

Architecture details, specifications, and lookup materials.

### Architecture

| Document | Description |
|----------|-------------|
| [Keypad_Layout_Overview.md](Reference/Keypad_Layout_Overview.md) | Keypad hardware layout and design |
| [Key_Mapping_Process.md](Reference/Key_Mapping_Process.md) | How key mappings are defined and dispatched |
| [Key_Dispatch_Sequence.png](Reference/Key_Dispatch_Sequence.png) | Rendered signal flow diagram (accompanies Key_Mapping_Process.md) |
| [Currently_Implemented_Keys.md](Reference/Currently_Implemented_Keys.md) | Current key bindings for all modes |
| [Frequency_Mode_Diagram.md](Reference/Frequency_Mode_Diagram.md) | Frequency Mode state diagram |

### Comparisons & Analysis

| Document | Description |
|----------|-------------|
| [Comparison_With_2025_Team.md](Reference/Comparison_With_2025_Team.md) | Old vs new architecture comparison |
| [Speech_Synthesizer_Comparison.md](Reference/Speech_Synthesizer_Comparison.md) | TTS engine comparison (Piper chosen) |
| [Doyle_Changes_Integration.md](Reference/Doyle_Changes_Integration.md) | Analysis of Doyle's contributions |
| [Cheaper_Hardware_Analysis.md](Reference/Cheaper_Hardware_Analysis.md) | Budget hardware options |

### Testing

| Document | Description |
|----------|-------------|
| [Manual_Tests_For_Set_Mode.md](Reference/Manual_Tests_For_Set_Mode.md) | Manual test procedures for all modes |

### Project Specification

| Document | Description |
|----------|-------------|
| [Project_Spec/System_Architecture.md](Project_Spec/System_Architecture.md) | High-level system design |
| [Project_Spec/System_Architecture_Detail.md](Project_Spec/System_Architecture_Detail.md) | Detailed architecture |
| [Project_Spec/Project_Objectives_and_Target_Users.md](Project_Spec/Project_Objectives_and_Target_Users.md) | Goals and audience |
| [Project_Spec/Project_Functional_Requirements.md](Project_Spec/Project_Functional_Requirements.md) | Feature requirements |
| [Project_Spec/Hardware_Constraints.md](Project_Spec/Hardware_Constraints.md) | Hardware limitations |
| [Project_Spec/Project_Plan.md](Project_Spec/Project_Plan.md) | Integration strategy, risks, milestones |

### Reference Libraries

| Directory / Document | Description |
|----------------------|-------------|
| [Hamlib API Reference](Reference/hamlib-api.md) | Hamlib C API function reference (rig, rotator, amplifier, utilities) |
| [Original_Hampod_Docs/](Original_Hampod_Docs/) | Original ICOMReader/K3Reader/KenwoodReader/Yaesu manuals (ICOMReader_Manual_Add_On.txt, ICOMReader_Manual_v106.txt, K3Reader_Manual_v120.txt, KenwoodReader_Manual_v104.txt, Yaesu_FT8X7_Manual.txt, Yaesu_Manual_100.txt) |
| [Original_Hampod_Firmware/](Original_Hampod_Firmware/) | Original Hampod firmware binaries: fw_ICOMICOM_v106b1, fw_ICOMICOM_v206b1, fw_K3K3_v120, fw_K3K3_v220, fw_KWKW_v104b16, fw_KWKW_v204b16, fw_YaesuYaesu_v100b21, fw_YaesuYaesu_v200b21 |
| [Old_NanoPi_Docs/](Old_NanoPi_Docs/) | Legacy NanoPi documentation |
| [Hardware_Files/](Hardware_Files/) | Schematics, PCB designs |
| [Formal_methods_tangent/](Formal_methods_tangent/) | Formal specification experiments |

---

## Archive

Historical, superseded, or stale documents kept for reference.

| Document | Reason |
|----------|--------|
| [Freq_Mode_Original_Design.md](Archive/Freq_Mode_Original_Design.md) | Superseded by Phase_1_Freq_Mode_Plan |
| [Paying_Tech_Debt.md](Archive/Paying_Tech_Debt.md) | Obsolete — Fresh Start replaced this approach |
| [Future_Work_Ideas.md](Archive/Future_Work_Ideas.md) | Informal brainstorm list |
| [Future_Work_Behavior_Map.md](Archive/Future_Work_Behavior_Map.md) | Behavior map diagram |
| [Work_Division_Plan.md](Archive/Work_Division_Plan.md) | Historical work division |
| [2025_12_5_Notes.md](Archive/2025_12_5_Notes.md) | Session notes from Dec 5, 2025 |
| [Obsidian_Notes.md](Archive/Obsidian_Notes.md) | Raw planning notes |
| [Test_Diagram.md](Archive/Test_Diagram.md) | Placeholder with no content |
| [behavior_map_architecture.png](Archive/behavior_map_architecture.png) | Architecture diagram (accompanies Future_Work_Behavior_Map.md) |

---

## Scripts

Operational scripts in `Documentation/scripts/`:

| Script | Purpose |
|--------|---------|
| `install_hampod.sh` | Full automated installation |
| `install_piper.sh` | Install Piper TTS |
| `run_hampod.sh` | Start HAMPOD |
| `run_hampod_service.sh` | Start as systemd service |
| `hampod_on_powerup.sh` | Configure autostart on boot |
| `power_down_protection.sh` | Enable read-only OS protection |
| `update_hampod.sh` | Safe update from GitHub |
| `hampod_cli.sh` | CLI wrapper |
| `setup_cli.sh` | Install CLI commands |
| `monitor_mem.sh` | Memory usage tracker |
| `regenerate_audio_piper.sh` | Rebuild Piper audio cache |
| `fix_beep_format.sh` | Fix beep audio format |
| `test_and_deploy.ps1` | Windows deploy helper |
| `deploy_and_run_imitation.ps1` | Windows test deploy |

### Deprecated Tests

| Script | Notes |
|--------|-------|
| `deprecated_tests/Regression_Imitation_Software.sh` | Deprecated — use regression scripts above |
| `deprecated_tests/Regression_Frequency_Mode.sh` | Deprecated — use regression scripts above |
| `deprecated_tests/Regression_Normal_Mode.sh` | Deprecated — use regression scripts above |
| `deprecated_tests/Regression_HAL_Integration.sh` | Deprecated — use regression scripts above |
| `deprecated_tests/remote_install.sh` | Deprecated — use `install_hampod.sh` |
| `deprecated_tests/remote_install.ps1` | Deprecated — use `install_hampod.sh` |
| `deprecated_tests/run_remote_test.sh` | Deprecated — use regression scripts above |

### Regression Tests

| Script | Tests |
|--------|-------|
| `Regression_Phase0_Integration.sh` | Phase 0 integration |
| `Regression_Phase_One_Manual_Radio_Test.sh` | Phase 1 manual radio |
| `Regression_Phase_Two_Manual_Test.sh` | Phase 2 manual |
| `Regression_Phase_Three_Manual_Test.sh` | Phase 3 manual |
