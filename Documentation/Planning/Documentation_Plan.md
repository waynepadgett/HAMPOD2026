# Documentation_Plan.md

> **Status:** 🔄 In Progress
> **Last Updated:** 2026-06-21

**Authors:** Amber Padgett, Wayne Padgett
**Purpose:** Track remaining documentation cleanup tasks after the 2026-06-20 refactor

---

# What Already Exists

The 2026-06-20 refactor completed most documentation organization:

- `Documentation/DOCS_OVERVIEW.md` — central index of all docs
- `Documentation/CHANGELOG.md` — tracks refactor changes
- `Documentation/Guides/` — 9 how-to guides (modes, CLI, debugging, merge SOP, etc.)
- `Documentation/Reference/` — architecture, specs, comparisons
- `Documentation/Planning/` — active and completed plans
- `Documentation/Archive/` — superseded documents
- Top-level `README.md` — project overview, quick start, configuration

---

# Remaining Tasks

## 1. ✅ COMPLETED — Missing Guide File

`Guides/Multiple_Modes_Guide.md` was created on 2026-06-21. Content verified against `main.c`, `frequency_mode.c`, and `set_mode.c` source files. Covers Normal Mode (default/query), Frequency Mode (direct entry), and Set Mode (radio parameter adjustment) with correct key routing priority.

## 2. 🔧 SEPARATE COMMIT REQUIRED — Consolidate Set Mode Plans

Two plans cover overlapping Set Mode work:

| Plan | Scope | Status |
|------|-------|--------|
| `Phase_3_Set_Mode_Plan.md` | Set Mode implementation (12 chunks) | In Progress — code done, testing pending |
| `Set_Mode_Correction_Plan.md` | Correct behavior to match ICOMReaderManual2.md | In Progress — phases 1-5 |

**Action:** Merge into a single comprehensive Set Mode plan. This requires careful reading of both files to avoid losing implementation details.

**Commit:** Must be a dedicated commit. Do not bundle with other changes.

## 3. ✅ COMPLETED — Plan Archival

Both plans verified and archived to `Planning/Completed/`:

| Plan | Verification | Result |
|------|-------------|--------|
| `Comm_Router_Plan.md` | All 12 steps verified against `comm.h`, `comm.c`, `speech.c`, `test_comm_queue.c` + git log | ✅ Done, archived |
| `Integration_Test_Plan.md` | All 9 steps from plan match `main_phase0.c` line-for-line. Regression script exists. | ✅ Done, archived |

## 4. ✅ COMPLETED — Stale Regression_Testing_Plan.md

`Regression_Testing_Plan.md` (created 2025-12-19) was marked ⚠️ Stale in `DOCS_OVERVIEW.md`. Now updated:

- All three core test procedures verified against current source code
- "Future Test Improvements" section overhauled — completed items moved to ✅ table, untracked tests inventoried
- Added current test inventory: `Software2/tests/` (6 files), `Firmware/hal/tests/` (7 files), regression scripts (4)
- Test 3 naming fixed: "Phase 0.9" → "Phase Zero" to match `main_phase0.c`
- Cross-reference added to archived `Integration_Test_Plan.md`
- Status in `DOCS_OVERVIEW.md`: ⚠️ Stale → 🔄 In Progress

---

# What Was Deleted (and Why)

This plan was originally 605 lines. The following sections were removed as bloat:

| Removed Section | Lines | Reason |
|-----------------|-------|--------|
| Master Analysis (plan-by-plan breakdown) | 360 | Duplicates DOCS_OVERVIEW.md status tracking |
| Repository Audit Plan (Phases A-C) | 67 | Commit history already analyzed in Completed/ |
| Diagram Plan | 18 | Architecture diagrams exist in Reference/ |
| Documents to Create (11 docs) | 20 | Target directory removed in refactor; most docs already exist |
| Execution Timeline (3 weeks) | 37 | Over-engineered for remaining work |
| Questions to Answer | 20 | Research questions, not actionable plan items |
| Success Criteria | 18 | Unmeasurable; README already achieves these |
| Documentation Goals (per-audience) | 20 | Already met by existing docs |
