# HAMPOD Documentation Changelog

> **Last Updated:** 2026-06-21

This file tracks all changes to the Documentation/ directory.

---

## [2026-06-21] Set Mode Plans Consolidated — Phase_3 + Correction Merged

### Added
- `Planning/Phase_3_Set_Mode_Plan.md` — Rewritten as single comprehensive document:
  - Merged implementation (12 chunks) + spec correction (5 phases) content
  - Fixed all stale items found by pre-merge audit (state machine, key routing, AGC API, shift notes, test script misnaming)
  - Added spec compliance table, bonus parameters (Tuning Step, VOX, Filter Number, Keyer Speed), corrected IC-7300 notes

### Moved
- `Planning/Set_Mode_Correction_Plan.md` → `Planning/Completed/Set_Mode_Correction_Plan.md` — Content merged, plan archived
- `Planning/Documentation_Plan.md` → `Planning/Completed/Documentation_Plan.md` — All 4 tasks complete, plan archived

### Changed
- `Documentation/DOCS_OVERVIEW.md` — Removed Documentation_Plan and Set_Mode_Correction_Plan from Active Work; added both to Completed Phases
- `Documentation/Planning/Documentation_Plan.md` — Task 2 marked ✅ COMPLETED (Set Mode plans consolidated)

---

## [2026-06-21] Multiple_Modes_Guide.md Created

### Added
- `Guides/Multiple_Modes_Guide.md` — How Normal, Frequency, and Set modes work together. Content verified against `main.c`, `frequency_mode.c`, and `set_mode.c`. Covers mode purpose, entry/exit, key routing priority, and Set vs Config Mode distinction.

### Changed
- `Planning/Documentation_Plan.md` — Task 1 marked ✅ COMPLETED; guide count 8→9

---

## [2026-06-21] Comm_Router_Plan.md — Verified Complete, Archived

### Changed
- `Planning/Comm_Router_Plan.md` — Status header flipped 🔄→✅, date bumped to 2026-06-21

### Moved
- `Planning/Comm_Router_Plan.md` → `Planning/Completed/Comm_Router_Plan.md` — All 12 implementation steps verified against source code

---

## [2026-06-21] Integration_Test_Plan.md — Verified Complete, Archived

### Changed
- `Planning/Integration_Test_Plan.md` — Status 🔄→✅, date bumped, checklist ticked (all 9 steps)
- `Planning/Documentation_Plan.md` — Task 3 marked ✅ COMPLETED (both plans archived)

### Moved
- `Planning/Integration_Test_Plan.md` → `Planning/Completed/Integration_Test_Plan.md` — All 9 steps verified against `main_phase0.c`

---

## [2026-06-21] Regression_Testing_Plan.md — Stale→Updated, Overhauled

### Changed
- `Planning/Regression_Testing_Plan.md` — Status ⚠️→✅, date bumped, future section rewritten
- Test 3 renamed "Phase 0.9" → "Phase Zero" to match source `main_phase0.c`
- 55-line stale "Future Test Improvements" section replaced with completed/inventory/remaining (42% reduction)
- Added current test inventory: `Software2/tests/` (6 files), `Firmware/hal/tests/` (7 files), regression scripts (4)
- `Planning/Documentation_Plan.md` — Task 4 marked ✅ COMPLETED
- `Documentation/DOCS_OVERVIEW.md` — ⚠️ Stale → 🔄 In Progress

---

## [2026-06-21] Set_Mode_Correction_Plan.md — Verified Against Code

### Changed
- `Planning/Set_Mode_Correction_Plan.md` — Rewrote stale sections after code audit
- Executive Summary table: 5 of 6 discrepancies verified as already fixed (beeps, Set announcement, [*] key)
- Phase 1 (Audio Feedback): All 4 steps marked ✅ Complete with file/line refs
- Phase 2 (Software Integration): All 3 steps marked ✅ Complete with call counts per file
- Phase 3.1: Diff replaced with verification note — `[*]` already calls `set_mode_exit()`
- Git Strategy: Restructured to show completed commits vs remaining items
- Verification Checklist: 12 of 17 items checked as done
- Estimated Effort: Revised from 6-10h to ~2h remaining

### Remaining
- Phase 3.3: `test_set_mode.c` still needs to be created
- Phase 4: Frequency announcement "dot" vs "point" still needs fix
- Phase 5: Integration test script and doc review still pending

---

## [2026-06-20] Documentation_Plan.md Cleanup

### Changed
- `Planning/Documentation_Plan.md` — Rewrote from 605 lines to ~80 lines
- Removed bloat: Master Analysis, Repository Audit Plan, Diagram Plan, Documents to Create, Execution Timeline, Questions, Success Criteria
- Flagged `Guides/Multiple_Modes_Guide.md` as missing (HIGH PRIORITY)
- Marked Set Mode plan consolidation as requiring separate commit
- Noted Comm_Router and Integration_Test plan archival as deferred open question

---

## [2026-06-20] Documentation Refactor

### Added
- `Documentation/DOCS_OVERVIEW.md` — Central index of all documentation
- `Documentation/CHANGELOG.md` — This file
- `Documentation/Planning/` — Folder for active and completed plans
- `Documentation/Planning/Completed/` — Subfolder for verified completed plans
- `Documentation/Guides/` — Folder for how-to documentation
- `Documentation/Reference/` — Folder for architecture, specs, and lookups
- `Documentation/Archive/` — Folder for stale and superseded documents
- Status markers (`✅ Completed`, `🔄 In Progress`, `🔴 Not Started`, `⚠️ Stale`) on all planning documents

### Changed
- Reorganized `Documentation/` from flat structure to category-based folders
- Standardized all filenames to `Title_Case_With_Underscores.md`
- Updated root `README.md` links to match new file paths

### Renamed
| Old Name | New Name |
|----------|----------|
| `config_mode_implementation_plan.md` | `Config_Mode_Impl_Plan.md` |
| `multiple_synthesizers_plan.md` | `Multiple_Synthesizers_Plan.md` |
| `Future_Work_ Ideas.md` | `Future_Work_Ideas.md` |
| `Hampod_RPi_Change_Plan.md` | `RPi_Migration_Plan.md` |
| `Keypad_Layout_Overview_Excerpts_And_Sources.md` | `Keypad_Layout_Overview.md` |
| `Interrupt_And_Piper_Persistent_Plan.md` | `Interrupt_And_Piper_Plan.md` |

### Moved to Planning/
| File | From | Status |
|------|------|--------|
| `Documentation_Plan.md` | `Project_Overview_and_Onboarding/DOCUMENTATION_PLAN.md` | 🔄 In Progress |
| `Fresh_Start_Master_Plan.md` | `Project_Overview_and_Onboarding/Fresh_Start_Big_Plan.md` | 🔄 In Progress |
| `Phase_3_Set_Mode_Plan.md` | `Project_Overview_and_Onboarding/Fresh_Start_Phase_3_Plan.md` | 🔄 In Progress |
| `Regression_Testing_Plan.md` | Root level | ⚠️ Stale |
| `Multiple_Synthesizers_Plan.md` | Root level | 🔄 Partial |
| `Startup_Device_Plan.md` | `Project_Overview_and_Onboarding/Startup_Device_Handling_Plan.md` | 🔄 Partial |
| `TTS_Caching_Plan.md` | `Performance_And_Cache/Implementation_Plan.md` | 🔄 Partial |
| `Memory_Leak_Investigation.md` | `Symlink_Help_Other_Functions/Memory_Leak_Log.md` | 🔄 Partial |
| `Comm_Router_Plan.md` | `Project_Overview_and_Onboarding/Comm_Router_Plan.md` | 🔄 In Progress |
| `Set_Mode_Correction_Plan.md` | `Project_Overview_and_Onboarding/Set_Mode_Correction_Plan.md` | 🔄 In Progress |
| `Integration_Test_Plan.md` | `Project_Overview_and_Onboarding/Step0.9_Integration_Test_Plan.md` | 🔄 In Progress |
| `Refactor_Todos.md` | `Performance_And_Cache/Refactor_Todos.md` | 🔴 Not Started |

### Moved to Planning/Completed/
| File | From |
|------|------|
| `Phase_0_Core_Infra_Plan.md` | `Project_Overview_and_Onboarding/Fresh_Start_Phase_Zero_Plan.md` |
| `Phase_1_Freq_Mode_Plan.md` | `Project_Overview_and_Onboarding/Fresh_Start_Phase_1_Plan.md` |
| `Phase_2_Normal_Mode_Plan.md` | `Project_Overview_and_Onboarding/Fresh_Start_Phase_2_Plan.md` |
| `RPi_Migration_Plan.md` | Root level |
| `Audio_Latency_Plan.md` | `Project_Overview_and_Onboarding/Audio_Latency_Improvement_Plan.md` |
| `Interrupt_And_Piper_Plan.md` | Root level |
| `Firmware_Bug_Fix_Plan.md` | `Project_Overview_and_Onboarding/Firmware_Bug_Fix_Plan.md` |
| `Firmware_Piper_Option_Plan.md` | `Project_Overview_and_Onboarding/Firmware_Update_Piper_Option_Plan.md` |
| `Configuration_Function_Plan.md` | `Project_Overview_and_Onboarding/Configuration_Function_Plan.md` |
| `Config_Mode_Impl_Plan.md` | Root level |
| `Startup_Shutdown_Protection_Plan.md` | Root level |
| `Install_Script_Plan.md` | `Project_Overview_and_Onboarding/Completed_Tasks/Hampod_Install_Script_Plan.md` |
| `Overclocking_Plan.md` | `Performance_And_Cache/Overclock_And_Todos.md` |
| `Commit_History_Analysis.md` | `Project_Overview_and_Onboarding/Completed_Tasks/Commit_History_Analysis.md` |
| `Firmware_Test_Fix.md` | `Project_Overview_and_Onboarding/Completed_Tasks/Firmware_Test_Fix.md` |

### Moved to Guides/
| File | From |
|------|------|
| `RPi_Setup_Guide.md` | `Project_Overview_and_Onboarding/RPi_Setup_Guide.md` |
| `RPi_Setup_Guide_Accessible.txt` | `Project_Overview_and_Onboarding/RPi_Setup_Guide_Accessible.txt` |
| `CLI_Help.md` | `Symlink_Help_Other_Functions/Cli_And_Logging_Plan.md` |
| `Debugging.md` | `Symlink_Help_Other_Functions/Debugging_System_Instability_Plan.md` |
| `Merge_SOP.md` | `sop_merge_to_main/Long_Do_This_Before_Merge_To_Main.md` |
| `Merge_SOP_Short.md` | `sop_merge_to_main/Short_Do_This_Before_Merge_To_Main.md` |
| `SOP_Merge_Transcript.md` | `sop_merge_to_main/SOP_Transcript.md` |

### Moved to Reference/
| File | From |
|------|------|
| `Keypad_Layout_Overview.md` | Root level |
| `Key_Mapping_Process.md` | Root level |
| `Currently_Implemented_Keys.md` | `Project_Overview_and_Onboarding/Currently_Implemented_Keys.md` |
| `Comparison_With_2025_Team.md` | `Project_Overview_and_Onboarding/Comparison_With_2025_Team_Results.md` |
| `Speech_Synthesizer_Comparison.md` | `Project_Overview_and_Onboarding/Speech_Synthesizer_Comparison.md` |
| `Frequency_Mode_Diagram.md` | Root level |
| `Doyle_Changes_Integration.md` | Root level |
| `Manual_Tests_For_Set_Mode.md` | Root level |
| `Cheaper_Hardware_Analysis.md` | Root level |

### Moved to Archive/
| File | From | Reason |
|------|------|--------|
| `Freq_Mode_Original_Design.md` | `Project_Overview_and_Onboarding/Fresh_Start_First_Freq_Mode.md` | Superseded |
| `Paying_Tech_Debt.md` | `Project_Overview_and_Onboarding/Completed_Tasks/Paying_Tech_Debt.md` | Obsolete |
| `Future_Work_Ideas.md` | Root level | Informal brainstorm |
| `Future_Work_Behavior_Map.md` | Root level | Behavior map |
| `Work_Division_Plan.md` | Root level | Historical |
| `2025_12_5_Notes.md` | `Project_Overview_and_Onboarding/2025_12_5_Notes.md` | Stale session notes |
| `Obsidian_Notes.md` | `Project_Overview_and_Onboarding/Completed_Tasks/Obsidian_Notes.md` | Stale notes |
| `Test_Diagram.md` | Root level | Placeholder |

### Removed
- Empty directories: `Performance_And_Cache/`, `Project_Overview_and_Onboarding/Completed_Tasks/`, `sop_merge_to_main/`, `Symlink_Help_Other_Functions/`, `Project_Overview_and_Onboarding/`
