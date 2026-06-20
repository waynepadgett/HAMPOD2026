# Documentation_Plan.md

> **Status:** 🔄 In Progress
> **Last Updated:** 2026-06-20

**Authors:** Amber Padgett, Wayne Padgett
**Purpose:** Track remaining documentation cleanup tasks after the 2026-06-20 refactor

---

# What Already Exists

The 2026-06-20 refactor completed most documentation organization:

- `Documentation/DOCS_OVERVIEW.md` — central index of all docs
- `Documentation/CHANGELOG.md` — tracks refactor changes
- `Documentation/Guides/` — 8 how-to guides (CLI, debugging, merge SOP, etc.)
- `Documentation/Reference/` — architecture, specs, comparisons
- `Documentation/Planning/` — active and completed plans
- `Documentation/Archive/` — superseded documents
- Top-level `README.md` — project overview, quick start, configuration

---

# Remaining Tasks

## 1. ⚠️ HIGH PRIORITY — Missing Guide File

`DOCS_OVERVIEW.md` line 28 references a guide that does not exist on disk:

| Missing File | Referenced In | Description |
|--------------|---------------|-------------|
| `Guides/Multiple_Modes_Guide.md` | DOCS_OVERVIEW.md "New Developers" section | How Normal, Frequency, Set modes work together |

Note: Key mapping reference links to `Reference/Currently_Implemented_Keys.md` — this file exists.

**Action:** Create `Guides/Multiple_Modes_Guide.md`. Content can be sourced from the mode source files (`normal_mode.c`, `frequency_mode.c`, `set_mode.c`).

**Priority:** HIGH — new developers following the onboarding path will hit a broken link.

## 2. 🔧 SEPARATE COMMIT REQUIRED — Consolidate Set Mode Plans

Two plans cover overlapping Set Mode work:

| Plan | Scope | Status |
|------|-------|--------|
| `Phase_3_Set_Mode_Plan.md` | Set Mode implementation (12 chunks) | In Progress — code done, testing pending |
| `Set_Mode_Correction_Plan.md` | Correct behavior to match ICOMReaderManual2.md | In Progress — phases 1-5 |

**Action:** Merge into a single comprehensive Set Mode plan. This requires careful reading of both files to avoid losing implementation details.

**Commit:** Must be a dedicated commit. Do not bundle with other changes.

## 3. ❓ OPEN — Plan Archival

Two plans may be complete and ready to move to `Planning/Completed/`:

| Plan | Evidence | Status |
|------|----------|--------|
| `Comm_Router_Plan.md` | All 12 implementation steps marked completed | Possibly done |
| `Integration_Test_Plan.md` | Status says "In Progress" but may be done | Needs verification |

**Action:** Deferred. Not investigating now. Open question for later.

## 4. 📝 NOTED — Stale Regression_Testing_Plan.md

`Regression_Testing_Plan.md` (created 2025-12-19) is marked ⚠️ Stale in `DOCS_OVERVIEW.md`. This is already tracked. Update when regression testing is actively revisited.

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
