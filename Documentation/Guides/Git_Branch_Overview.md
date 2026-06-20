# Git Branch Overview

> **Last Updated:** 2026-06-20

This document describes the branch strategy, naming conventions, and current branches for the HAMPOD2026 repository.

---

## Branch Strategy

### Main Branch

- **`main`** — Production-ready code. Only merged via pull request after review.
  - Must always build and deploy cleanly
  - All integration tests must pass before merge
  - Protected branch: no direct pushes

### Development Branch

- **`June2026`** — Current development branch. Active work happens here.
  - Feature branches merge into this for integration
  - Periodically merged to `main` when stable

### Feature Branches

Short-lived branches for specific features or fixes. Named with the `feature/` prefix:

```
feature/{feature-name}
```

Examples:
- `feature/frequency-mode` — Frequency Mode implementation
- `feature/normal-mode` — Normal Mode implementation
- `feature/set-mode` — Set Mode implementation
- `feature/config-mode` — Configuration Mode
- `feature/radio-auto-reconnect` — Auto-reconnect on serial disconnect
- `feature/interrupt-and-persistent-piper` — Interruptible audio, persistent Piper TTS
- `feature/performance-cache` — TTS caching and performance tuning
- `feature/startup-shutdown-protection` — Boot scripts, SD card protection
- `feature/hampod-cli-and-symlink` — CLI interface
- `feature/comm-router` — Communication router
- `feature/speech-speed-config` — Speech speed configuration
- `feature/preserve-config` — Config preservation across updates
- `feature/pi3-overclock` — Raspberry Pi 3 overclocking
- `feature/symlink_help_other_functions` — Debugging and logging

### Other Branch Types

| Prefix | Purpose | Example |
|--------|---------|---------|
| `fix/` | Bug fixes | `fix/audio-repeat-bug` |
| `refactor/` | Code refactoring | `refactor-docs` |
| `archive/` | Preserved historical states | `archive/dec3-bug-fixes` |
| `rebuild/` | Major rewrites | `rebuild/clean-dec3` |
| `pr-test-*` | PR testing | `pr-test-merge-coderabbit` |

---

## Naming Conventions

### Format

```
{type}/{descriptive-name}
```

### Rules

1. **Lowercase with hyphens** — `feature/frequency-mode` not `feature/Frequency_Mode`
2. **Descriptive** — Name should describe what the branch does
3. **Short** — Keep under 40 characters
4. **No dates in name** — Use git log for history
5. **No ticket numbers** — This repo doesn't use issue tracking

### Good Examples

- `feature/config-mode`
- `fix/audio-repeat-bug`
- `refactor/radio-module`

### Bad Examples

- `feature/new-stuff` — Too vague
- `feature/2026-06-20-changes` — Don't put dates in branch names
- `feature/DOYLE-CHANGES` — Use lowercase, hyphens not underscores

---

## Current Branches

### Active

| Branch | Purpose | Status |
|--------|---------|--------|
| `main` | Production-ready code | Stable |
| `June2026` | Current development branch | Active |
| `Doyle-changes-May26` | Doyle's radio features integration | Merged to development |

### Feature Branches (Merged)

These branches have been merged and are preserved for reference:

| Branch | Description |
|--------|-------------|
| `feature/frequency-mode` | Frequency Mode implementation |
| `feature/normal-mode` | Normal Mode with radio queries |
| `feature/set-mode` | Set Mode for radio parameter adjustment |
| `feature/config-mode` | Configuration Mode UI |
| `feature/radio-auto-reconnect` | Auto-reconnect on serial disconnect |
| `feature/interrupt-and-persistent-piper` | Interruptible audio, persistent Piper |
| `feature/performance-cache` | TTS caching, overclocking |
| `feature/startup-shutdown-protection` | Boot scripts, SD card protection |
| `feature/hampod-cli-and-symlink` | CLI interface |
| `feature/comm-router` | Communication router |
| `feature/speech-speed-config` | Speech speed configuration |
| `feature/preserve-config` | Config preservation across updates |
| `feature/pi3-overclock` | Raspberry Pi 3 overclocking |
| `feature/symlink_help_other_functions` | Debugging and logging |

### Archive Branches

| Branch | Description |
|--------|-------------|
| `archive/dec3-bug-fixes` | Preserved state from Dec 3 bug fixes |
| `rebuild/clean-dec3` | Clean rebuild starting point |

---

## Merge Workflow

See [Merge_SOP.md](Merge_SOP.md) for the full procedure.

### Quick Summary

1. Create feature branch from `main` or `June2026`
2. Work on feature, commit regularly
3. Push branch to origin
4. Create pull request
5. Review and test
6. Merge to `June2026` (or `main` for production-ready)
7. Delete feature branch after merge

### Branch Protection Rules

- `main` requires pull request for merge
- All CI checks must pass before merge
- At least one review required for `main`
- Force pushes to `main` are prohibited
