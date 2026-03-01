# Implementation Plan: HAMPOD CLI, Logging, and Maintenance Utilities

## Summary
This plan details the implementation of a centralized `hampod` command-line interface (CLI) to manage the entire HAMPOD system efficiently. The goal is to provide a single, unified entry point (symlinked to the system PATH) for starting the system, viewing help documentation, managing TTS caches, updating from the repository gracefully, and handling configuration backups. Additionally, logging mechanisms will be refactored and consolidated for better maintainability and troubleshooting.

## 1. The `hampod` CLI Script (`Documentation/scripts/hampod_cli.sh`)
Create a master bash script `hampod_cli.sh` that will parse subcommands. This script will consolidate various disjointed commands into one easy-to-remember interface.

**Subcommands to Implement:**
- `hampod start [--no-build] [--debug]`: Wraps the existing `run_hampod.sh` logic to launch both the Firmware and Software2 components cleanly.
  - `--no-build`: Skips the build step.
  - `--debug`: Runs hampod with verbose Hamlib debug logging enabled.
- `hampod help`: Displays a comprehensive usage guide showing all available commands, a list of available regression tests (e.g., Phase 0 Integration), the start command, cache clearing commands, the reset command, and valid flags for the `install_hampod.sh` script.
- `hampod clear-cache`: Clears only the Text-to-Speech (TTS) cache directory to free up space or regenerate audio files. It will not touch system logs or configurations.
- `hampod reset`: Performs a hard reset of the system state. Unlike `clear-cache`, this will clear system logs (`/tmp/firmware.log`, `/tmp/hampod_output.txt`, etc.), remove stale IPC pipes, stop background processes, and optionally prompt to reset the `hampod.conf` to its factory default.
- `hampod backup-config`: Copies `Software2/config/hampod.conf` to a timestamped backup location (e.g., `hampod.conf.bak_YYYYMMDD`).
- `hampod restore-config`: Prompts the user with available backups and restores the configuration from a selected backup file.

## 2. System Symlink (`/usr/local/bin/hampod`)
To ensure the `hampod` command is globally available, an installation step must be added (or a separate `setup_cli.sh` created) to symlink the CLI script into the system PATH.
**Command:**
```bash
sudo ln -sf /path/to/HAMPOD2026/Documentation/scripts/hampod_cli.sh /usr/local/bin/hampod
```
This guarantees that running `hampod <command>` will work regardless of the current working directory.

## 3. Graceful Pull/Update Script (`Documentation/scripts/update_hampod.sh`)
Currently, `git pull` complains due to compiled artifacts or local edits in `hampod.conf`. This script will provide a seamless update experience.

**Workflow Workflow:**
1. **Stash Config:** Temporarily copy `Software2/config/hampod.conf` to `/tmp/hampod.conf.bak`.
2. **Clean Environment:** Run `make clean` in both `Firmware/` and `Software2/` to remove compiled binaries that might cause conflicts.
3. **Pull Changes:** Execute `git pull origin main` (or the respective branch).
4. **Restore Config:** Move the stashed configuration file back into place, preserving user preferences.
5. **Rebuild:** Run `make` in the respective directories to recompile with the latest codebase.

## 4. Logging Refactor
Currently, logging is functional but scattered across different outputs and `/tmp/` files. We will structure this to make debugging straightforward.

- **Centralization:** Centralize all logs into a single directory (e.g., `~/HAMPOD2026/logs/` or `/var/log/hampod/`).
- **Standardization:** Enforce a uniform logging format across both C-code components:
  `[TIMESTAMP] [COMPONENT] [LEVEL] Message` (e.g., `[2026-02-28 15:00:01] [FIRMWARE] [INFO] Keypad initialized.`)
- **Redirection:** Update `hampod start` and individual C applications to consistently append to these specific log files instead of dumping everything to standard output.