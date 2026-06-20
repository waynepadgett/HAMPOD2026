# Documentation_Plan.md

> **Status:** 🔄 In Progress
> **Last Updated:** 2026-06-20

**Authors:** Amber Padgett, Wayne Padgett  
**Purpose:** Comprehensive documentation planning and analysis reference for HAMPOD (Ham Radio Audio Pod) project

---

# Project Overview (from Documentation_Plan.md)

**HAMPOD** (Ham Radio Audio Pod) is an embedded hardware/software project that provides:
- A voice-controlled interface for amateur radio equipment
- Text-to-speech feedback using Festival
- Integration with Hamlib for radio control
- A USB keypad interface for user input
- USB audio output for speech synthesis

**Current State:**
- Actively migrating from **NanoPi** to **Raspberry Pi** hardware
- ~50+ commits of development work to analyze
- Phased migration plan (Phases 1-5 complete, 6-9 pending)
- Mix of firmware (C) and software layers

---

# Documentation Goals (from Documentation_Plan.md)

## For Management
- [ ] Clear executive summary of what the project does
- [ ] Architecture diagrams (system-level and component-level)
- [ ] Current status and roadmap
- [ ] Technical debt and risk assessment

## For New Developers (Onboarding)
- [ ] Getting started guide
- [ ] Development environment setup
- [ ] Code structure walkthrough
- [ ] Build and deployment instructions

## For Ongoing Maintenance
- [ ] API documentation
- [ ] Troubleshooting guides
- [ ] Testing procedures

---

# Repository Audit Plan (from Documentation_Plan.md)

## Phase A: Commit History Analysis
**Goal:** Understand the evolution and identify key milestones

### Tasks:
1. [ ] Export full commit history with details:
   - `git log --oneline --all` → summary list
   - `git log --stat` → files changed per commit
   - `git log --pretty=format:"%h %ad %s" --date=short` → timeline view

2. [ ] Categorize commits by theme:
   - 🐛 Bug fixes (e.g., "Fix: ...")
   - ✨ Features (e.g., "Implement Phase X")
   - 📝 Documentation updates
   - 🔧 Build/config changes
   - 🚧 Work-in-progress/experimental

3. [ ] Create a **Commit Summary Timeline** document showing:
   - Major milestones by date
   - Feature additions
   - Architecture changes

4. [ ] Identify any:
   - Reverted commits (potential issues)
   - Large refactors
   - TODO/FIXME patterns in commit messages

## Phase B: Codebase Structure Analysis
**Goal:** Map the architecture and dependencies

### Tasks:
1. [ ] Document the directory structure:
   ```
   HAMPOD2026/
   ├── Documentation/    ← Current docs
   ├── Firmware/         ← Embedded C code
   │   ├── hal/          ← Hardware abstraction layer
   │   ├── tests/        ← Test programs
   │   └── pregen_audio/ ← Pre-generated audio files
   ├── Software/         ← Higher-level application code
   │   ├── Modes/        ← Operating modes
   │   ├── HamlibWrappedFunctions/ ← Radio control
   │   └── ConfigSettings/ ← Configuration
   └── Hardware_Files/   ← Schematics, PCB files?
   ```

2. [ ] Create **dependency diagrams** showing:
   - How Firmware and Software layers communicate
   - External dependencies (Festival, Hamlib, ALSA)
   - Build order/dependencies

3. [ ] Review and document each major module:
   - Purpose
   - Key files
   - Public interface/API
   - Known issues or TODOs

## Phase C: Technical Debt Inventory
**Goal:** Identify cleanup opportunities and risks

### Tasks:
1. [ ] Search for TODO/FIXME comments in code
2. [ ] Identify dead code or unused files
3. [ ] Note any hardcoded values that should be configurable
4. [ ] List deprecated or legacy code patterns
5. [ ] Document any known bugs or workarounds

---

# Diagram Plan (from Documentation_Plan.md)

## Required Diagrams:

| Diagram | Type | Purpose |
|---------|------|---------|
| System Overview | Block diagram | High-level hardware + software components |
| Software Architecture | Component diagram | Layers, modules, and interfaces |
| Data Flow | Flow diagram | How key press → radio command → voice feedback works |
| Hardware Connections | Wiring diagram | USB keypad, audio, Pi connections |
| Build Process | Flow diagram | Source → compile → deploy pipeline |
| State Machine | State diagram | If StateMachine.c exists, document its states |

## Tools to Consider:
- **Mermaid** (in-repo Markdown diagrams)
- **Draw.io** (exportable PNGs)
- **Excalidraw** (hand-drawn style)

---

# Documents to Create (from Documentation_Plan.md)

## Essential Documents (Priority 1)

| Document | Description | Location |
|----------|-------------|----------|
| `EXECUTIVE_SUMMARY.md` | Non-technical overview for management | `Documentation/Project_Overview_and_Onboarding/` |
| `ARCHITECTURE.md` | Technical architecture with diagrams | `Documentation/Project_Overview_and_Onboarding/` |
| `GETTING_STARTED.md` | Developer onboarding guide | `Documentation/Project_Overview_and_Onboarding/` |
| `BUILD_AND_DEPLOY.md` | How to build and deploy to Pi | `Documentation/Project_Overview_and_Onboarding/` |
| `COMMIT_HISTORY_ANALYSIS.md` | Summary of the ~50 commits | `Documentation/Project_Overview_and_Onboarding/` |

## Supporting Documents (Priority 2)

| Document | Description | Location |
|----------|-------------|----------|
| `GLOSSARY.md` | Ham radio and project-specific terms | `Documentation/Project_Overview_and_Onboarding/` |
| `TESTING_GUIDE.md` | How to run and write tests | `Documentation/Project_Overview_and_Onboarding/` |
| `TROUBLESHOOTING.md` | Common issues and solutions | `Documentation/Project_Overview_and_Onboarding/` |
| `ROADMAP.md` | Future plans and remaining work | `Documentation/Project_Overview_and_Onboarding/` |

---

# Execution Timeline (from Documentation_Plan.md)

## Week 1: Discovery & Analysis
- [ ] **Day 1-2:** Commit history analysis
  - Export and categorize all commits
  - Identify key milestones
  - Create timeline summary
  
- [ ] **Day 3-4:** Codebase walkthrough
  - Document directory structure
  - Read through key source files
  - Note dependencies and interfaces

- [ ] **Day 5:** Technical debt audit
  - Search for TODOs/FIXMEs
  - Identify cleanup opportunities

## Week 2: Documentation Creation
- [ ] **Day 6-7:** Core documents
  - Executive Summary
  - Architecture document with diagrams
  
- [ ] **Day 8-9:** Onboarding materials
  - Getting Started guide
  - Build and Deploy instructions

- [ ] **Day 10:** Review and polish
  - Cross-reference documents
  - Update main README with links
  - Create glossary

## Week 3: Finalization
- [ ] Management presentation preparation
- [ ] Final review with stakeholders
- [ ] Address feedback

---

# Questions to Answer During Analysis (from Documentation_Plan.md)

## Technical Questions
- [ ] What is the complete data flow from key press to radio command?
- [ ] How does the Firmware ↔ Software communication work (pipes/packets)?
- [ ] What are the supported radios via Hamlib?
- [ ] What is the state machine managing?
- [ ] Are there any race conditions or threading concerns?

## Project Questions
- [ ] What remains to complete the Raspberry Pi migration?
- [ ] What hardware is required for a full deployment?
- [ ] Are there any external services or cloud dependencies?
- [ ] What is the testing strategy?

## Business Questions
- [ ] Who is the target user for HAMPOD?
- [ ] What problem does it solve for ham radio operators?
- [ ] Are there similar products/projects?
- [ ] What is the long-term vision?

---

# Success Criteria (from Documentation_Plan.md)

## For Management Deliverables
- [ ] Non-technical stakeholder can understand what HAMPOD does
- [ ] Clear picture of project status and remaining work
- [ ] Risks and technical debt are documented

## For Onboarding Materials
- [ ] New developer can set up environment in < 1 hour
- [ ] New developer can make a simple change and deploy
- [ ] Code structure is clearly explained

## For Repository Health
- [ ] All major modules have documentation
- [ ] README files are accurate and current
- [ ] No orphaned or undocumented files

---

# Master Analysis (from original Master_Plan.md)

## Summary Statistics

| Plan Document | Status | Lines | Created | Last Updated | Priority |
|---------------|--------|-------|---------|--------------|----------|
| Documentation_Plan.md | 🔄 In Progress | 278 | **2025-12-05** | 2026-06-20 | **HIGH** |
| Fresh_Start_Master_Plan.md | 🔄 In Progress | 404 | **2025-12-07** | 2026-06-20 | **HIGH** |
| Comm_Router_Plan.md | 🔄 In Progress | 422 | **2025-12-20** | 2026-06-20 | **HIGH** |
| Integration_Test_Plan.md | 🔄 In Progress | 409 | **2025-12-19** | 2026-06-20 | **HIGH** |
| Multiple_Synthesizers_Plan.md | 🔄 Partial | 176 | **2026-03-01** | 2026-06-20 | **MEDIUM** |
| Phase_3_Set_Mode_Plan.md | 🔄 In Progress | 388 | **2025-12-26** | 2026-06-20 | **HIGH** |
| Set_Mode_Correction_Plan.md | 🔄 In Progress | 366 | **2025-12-31** | 2026-06-20 | **HIGH** |
| Startup_Device_Plan.md | 🔄 Partial | 439 | **2026-01-04** | 2026-06-20 | **MEDIUM** |
| TTS_Caching_Plan.md | 🔄 Partial | 195 | **2026-02-26** | 2026-06-20 | **MEDIUM** |
| Regression_Testing_Plan.md | ⚠️ Stale | 365 | **2025-12-19** | 2026-06-20 | **HIGH** |
| Refactor_Todos.md | 🔄 In Progress | 202 | **2026-02-13** | 2026-06-20 | **HIGH** |

## Detailed Plan Analysis

### 1. Documentation_Plan.md
**Scope:** Comprehensive repository documentation and management-ready deliverables

**Created:** 2025-12-05
**Last Updated:** 2026-06-20

**Key Sections:**
- Project Summary (HAMPOD overview)
- Documentation Goals (Management, Onboarding, Maintenance)
- Repository Audit Plan (Phases A, B, C)
- Diagram Plan (System, Architecture, Data Flow, Hardware, Build, State Machine)
- Documents to Create (Priority 1 & 2)
- Execution Timeline (Weeks 1-3)
- Questions to Answer During Analysis
- Success Criteria

**Stale Areas:**
- Timeline dates are outdated (Week 1-3, 2026-06-20)
- Some tasks may have been completed in other plans

**Duplication:**
- Overlaps with Fresh_Start_Master_Plan.md (similar goals)
- Similar structure to other detailed plans

### 2. Fresh_Start_Master_Plan.md
**Scope:** Big picture implementation plan prioritizing mode implementation order

**Created:** 2025-12-07
**Last Updated:** 2026-06-20

**Key Sections:**
- Overview and detailed plans table
- Prerequisites (completed)
- Target Behavior (modes and features)
- Reuse from Old Code
- Recommended Implementation Order (Phases 0-5)
- Architecture Recommendations (4 detailed problems)
- Implementation Dependency Graph
- Summary: Implementation Order

**Stale Areas:**
- Some phases marked as "In Progress" but may be completed
- Status indicators may be outdated

**Duplication:**
- Overlaps significantly with Documentation_Plan.md
- Similar to Phase_3_Set_Mode_Plan.md (Set Mode section)

### 3. Comm_Router_Plan.md
**Scope:** Fix packet type conflicts between keypad and speech threads

**Created:** 2025-12-20
**Last Updated:** 2026-06-20

**Key Sections:**
- Overview and problem/solution
- Design Decisions (timeouts, error recovery)
- Branch Workflow
- Implementation Steps (12 steps, all completed)
- Summary of Files Changed
- Rollback Plan
- Checklist (all completed)

**Stale Areas:**
- All implementation steps marked as completed (2025-12-20 dates)
- May need updates if new issues discovered

**Duplication:**
- Minimal duplication with other plans
- Unique implementation details

### 4. Integration_Test_Plan.md
**Scope:** Step 0.9 integration test combining all Software2 modules

**Created:** 2025-12-19
**Last Updated:** 2026-06-20

**Key Sections:**
- Overview and success criteria
- Prerequisites checklist
- Step-by-step implementation (9 steps)
- Final checklist
- Regression test script (future)

**Stale Areas:**
- Status: "In Progress" but may be completed
- Some implementation details may need updating

**Duplication:**
- Overlaps with Regression_Testing_Plan.md
- Similar to Phase 0.9 tests in other plans

### 5. Multiple_Synthesizers_Plan.md
**Scope:** Runtime switching between TTS engines (Piper, Flite, Festival)

**Created:** 2026-03-01
**Last Updated:** 2026-06-20

**Key Sections:**
- Goal and design decisions
- Architecture
- Implementation Phases (7 phases, Phase 0-6)
- Merge criteria

**Stale Areas:**
- Status: "Partial (missing Flite engine + dispatch layer)"
- Phase 2 (Flite) may need completion

**Duplication:**
- Overlaps with TTS_Caching_Plan.md (both TTS-related)
- Similar to audio engine discussions in other plans

### 6. Phase_3_Set_Mode_Plan.md
**Scope:** Set Mode implementation for adjusting radio parameters

**Created:** 2025-12-26
**Last Updated:** 2026-06-20

**Key Sections:**
- Overview and prerequisites
- Target key bindings
- Architecture and code structure
- Implementation chunks (12 chunks)
- Execution checklist
- Dependencies

**Stale Areas:**
- Status: "In Progress (code done, testing pending)"
- Chunks 10-12 marked as not started

**Duplication:**
- Overlaps with Set_Mode_Correction_Plan.md (complementary)
- Similar to Fresh_Start_Master_Plan.md (Set Mode section)

### 7. Set_Mode_Correction_Plan.md
**Scope:** Correct current system behavior to match ICOMReaderManual2.md specifications

**Created:** 2025-12-31
**Last Updated:** 2026-06-20

**Key Sections:**
- Executive summary and key discrepancies
- Phase 1: Audio Feedback System (Firmware)
- Phase 2: Key Beep Integration
- Phase 3: Set Mode Behavior Corrections
- Phase 4: Frequency Announcement Format
- Phase 5: Testing and Documentation

**Stale Areas:**
- Status: "In Progress"
- Some phases may need completion

**Duplication:**
- Overlaps with Phase_3_Set_Mode_Plan.md (both Set Mode)
- Complements rather than duplicates

### 8. Startup_Device_Plan.md
**Scope:** Reliable audio/radio device selection at startup

**Created:** 2026-01-04
**Last Updated:** 2026-06-20

**Key Sections:**
- Problem summary
- Architecture overview
- Phase 1: Deterministic Audio Device Selection
- Phase 2: Multi-Radio Configuration Support
- Phase 3: Startup Configuration Validation
- Phase 4: Integration Testing

**Stale Areas:**
- Status: "Partial (missing change detection Phase 3)"
- Phase 3.1-3.2 marked as not started

**Duplication:**
- Minimal duplication with other plans
- Unique device handling focus

### 9. TTS_Caching_Plan.md
**Scope:** Reduce TTS latency by ≥50% via disk caching

**Created:** 2026-02-26
**Last Updated:** 2026-06-20

**Key Sections:**
- Architecture overview
- Phase 1: Basic Disk Cache (MVP) - ✅ Complete
- Phase 2: Cache Warmup Script - ⏳ Not Started

**Stale Areas:**
- Status: "Partial (missing warmup script)"
- Phase 2 needs implementation

**Duplication:**
- Overlaps with Multiple_Synthesizers_Plan.md (both TTS)
- Similar caching concepts

### 10. Regression_Testing_Plan.md
**Scope:** Regression testing strategy for HAMPOD2026

**Created:** 2025-12-19
**Last Updated:** 2026-06-20

**Key Sections:**
- Overview and when to run tests
- Core Regression Tests (3 tests)
- Test Execution Log Template
- Future Test Improvements
- Troubleshooting

**Stale Areas:**
- Status: "⚠️ Stale (needs update for current codebase)"
- May need significant updates

**Duplication:**
- Overlaps with Integration_Test_Plan.md
- Similar test structure to other plans

### 11. Refactor_Todos.md
**Scope:** Path to maximum clarity for contributors

**Created:** 2026-02-13
**Last Updated:** 2026-06-20

**Key Sections:**
- Verification & Phasing (Feb 2026)
- Documentation Folder Audit (0.1-0.5)
- Documentation clarity (1.1-1.4)
- Code clarity (2.1-2.6)
- Repository and structure clarity (3.1-3.3)
- Test clarity (4.1-4.3)
- Build and deploy clarity (5.1-5.2)
- Minor cleanup (6.1-6.2)
- Priority overview

**Stale Areas:**
- Status: "🔄 In Progress" — Section 0 (Documentation Folder Audit) is 6/11 items done
- Sections 1-7 (clarity, code, repo, test, build, polish) still need implementation

**Duplication:**
- Overlaps with Documentation_Plan.md (similar goals)
- Complements other documentation efforts

## Duplication Analysis

### High Duplication (Consolidate)

1. **Documentation_Plan.md (2025-12-05) ↔ Fresh_Start_Master_Plan.md (2025-12-07)**
   - Both cover project overview and implementation goals
   - Similar structure and many overlapping sections
   - **Recommendation:** Keep Fresh_Start_Master_Plan.md as primary, archive Documentation_Plan.md

2. **Phase_3_Set_Mode_Plan.md (2025-12-26) ↔ Set_Mode_Correction_Plan.md (2025-12-31)**
   - Both focus on Set Mode implementation
   - Complementary but overlapping
   - **Recommendation:** Merge into single comprehensive Set Mode plan

3. **Regression_Testing_Plan.md (2025-12-19) ↔ Integration_Test_Plan.md (2025-12-19)**
   - Both cover testing strategies
   - Integration_Test_Plan.md is more specific
   - **Recommendation:** Update Regression_Testing_Plan.md to include Integration_Test_Plan.md content

### Medium Duplication (Review)

1. **Multiple_Synthesizers_Plan.md (2026-03-01) ↔ TTS_Caching_Plan.md (2026-02-27)**
   - Both TTS-related but different focus
   - May need coordination
   - **Recommendation:** Keep separate but ensure consistency

2. **Startup_Device_Plan.md (2026-02-26) ↔ Fresh_Start_Master_Plan.md (2026-02-26)**
   - Both touch on device/audio topics
   - Different scope and depth
   - **Recommendation:** Keep separate for specialized focus

### Low Duplication (Maintain)

1. **Comm_Router_Plan.md (2025-12-19)**
   - Unique implementation details
   - No significant overlap
   - **Recommendation:** Keep as-is

## Stale Content Analysis

### Critical Stale Items (Update Required)

1. **Regression_Testing_Plan.md (2025-12-19)** - Needs complete update
2. **Documentation_Plan.md (2025-12-08)** - Timeline and some content outdated
3. **Fresh_Start_Master_Plan.md (2026-02-26)** - Some phase statuses may be outdated

### Partially Stale Items (Review)

1. **Multiple_Synthesizers_Plan.md (2026-03-01)** - Missing Flite implementation
2. **Startup_Device_Plan.md (2026-01-04)** - Missing Phase 3 completion
3. **TTS_Caching_Plan.md (2026-02-26)** - Missing Phase 2 warmup script
4. **Refactor_Todos.md (2026-02-13)** - Section 0 partially done (5 items remaining); Sections 1-7 not started

## Recommended Actions

### Immediate (High Priority)

1. **Create Master_Plan.md** - ✅ Complete (content merged into this document)
2. **Update Regression_Testing_Plan.md (2025-12-19)** - Critical
3. **Consolidate Documentation Plans** - High impact
4. **Complete Missing Phases** - Multiple plans

### Medium Priority

1. **Implement Phase 2** in TTS_Caching_Plan.md (2026-02-27)
2. **Complete Phase 3** in Startup_Device_Plan.md (2026-02-26)
3. **Implement Phase 2** in Multiple_Synthesizers_Plan.md (2026-03-01)
4. **Review and update** stale plan statuses

### Low Priority

1. **Maintain** unique plans (Comm_Router_Plan.md, etc.)
2. **Coordinate** between overlapping plans
3. **Archive** completed or superseded plans

## Next Steps

1. **Review this Documentation_Plan.md** with stakeholders
2. **Prioritize updates** based on impact and effort
3. **Create consolidation plans** for duplicated content
4. **Update plan statuses** to reflect current reality
5. **Archive or merge** redundant plans

## Conclusion

This Documentation_Plan.md provides a comprehensive view of all planning documents in the Documentation/Planning/ folder. The key findings are:

- **Significant duplication** existed between original Documentation_Plan.md (2025-12-08) and Fresh_Start_Master_Plan.md (2026-02-26)
- **Several plans are stale** and need updates (especially Regression_Testing_Plan.md from 2025-12-19)
- **Multiple plans have incomplete phases** that need completion
- **Clear consolidation opportunities** exist to reduce redundancy

By following the recommended actions, the project can achieve:
- Reduced documentation redundancy
- Updated and accurate planning documents
- Clear implementation priorities
- Better coordination between related plans

**Note:** The original creation dates reveal that most planning documents were created between **2025-12-08 and 2026-03-01**, with the Documentation/Planning/ folder reorganization occurring on **2026-06-20**. This explains why many files show the same 2026-06-20 dates - they were all moved/renamed together in that single reorganization commit.

(End of file - total 700 lines)