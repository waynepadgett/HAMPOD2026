# Implementation Plan: HAMPOD CLI, Logging, and Maintenance Utilities

## Summary
This plan details the implementation of a centralized `hampod` command-line interface (CLI) to manage the entire HAMPOD system efficiently, and a refactoring of the system's logging mechanisms. 

To follow Git best practices and keep changes isolated, this work will be split across two separate branches:
1.  **`feature/hampod-cli-and-symlink`**: Focuses on creating the new CLI tools and system symlink without altering existing core behavior.
2.  **`feature/logging-refactor`**: Focuses on the more pervasive changes required to centralize and standardize logging across the C components and bash scripts.

---

## Branch 1: `feature/hampod-cli-and-symlink`
This branch implements the new user-facing CLI and update management tools. **To facilitate easier review and testing, this work will be broken down into the following bite-sized commits.**

### Commit 1: Core CLI Structure & basic commands
Create the master bash script `Documentation/scripts/hampod_cli.sh` and implement the foundational commands (`help` and `start`).
- `hampod help`: Displays usage guide and available commands.
- `hampod start [--no-build] [--debug]`: Wraps the existing `run_hampod.sh` logic.
- **Testing Plan:** 
  1. Run `./Documentation/scripts/hampod_cli.sh help` and verify the output is correct.
  2. Run `./Documentation/scripts/hampod_cli.sh start --no-build` and ensure it successfully delegates to `run_hampod.sh` without building.

### Commit 2: CLI Maintenance Utilities (`clear-cache` & `reset`)
Add the system maintenance commands to the CLI.
- `hampod clear-cache`: Clears only the Text-to-Speech (TTS) cache directory.
- `hampod reset`: Performs a hard reset of the system state (clearing logs, stale pipes, stopping processes).
- **Testing Plan:**
  1. prompt the user to hit some keys on the hampod while its running, and then clear the TTS cache, run `./Documentation/scripts/hampod_cli.sh clear-cache`, and verify only the cache is emptied.
  2. Ensure background dummy processes/pipes exist, run `./Documentation/scripts/hampod_cli.sh reset`, and verify the system state is clean.

### Commit 3: CLI Configuration Management (`backup-config` & `restore-config`)
Add configuration backup and restore functionality.
- `hampod backup-config`: Copies `Software2/config/hampod.conf` to a timestamped backup location.
the backup config should have an interactive script to name the backup file. so that it is user friendly to know what backup is what. it should also include the date and time of the backup.

- `hampod restore-config`: Prompts the user with available backups and restores the selected file.
- **Testing Plan:**
  1. Run `./Documentation/scripts/hampod_cli.sh backup-config` and verify a timestamped `.bak` file is created in the correct location containing the current config.
  2. Modify `hampod.conf`, run `./Documentation/scripts/hampod_cli.sh restore-config`, select the backup, and verify the config is reverted.

### Commit 4: System Symlink
Add a `setup_cli.sh` script (or simple documented step) to safely symlink the CLI to `/usr/local/bin/hampod`.
- **Testing Plan:**
  1. Run the symlink setup command.
  2. Navigate to `/tmp` and simply run `hampod help` to verify the command works globally from the system PATH.

### Commit 5: Graceful Pull/Update Script (`update_hampod.sh`)
Create the `Documentation/scripts/update_hampod.sh` script to handle clean code updates.
- **Workflow:** Stash config -> `make clean` -> `git pull` -> Restore config -> `make`.
- **Testing Plan:**
  1. Modify `hampod.conf` locally.
  2. Run `./Documentation/scripts/update_hampod.sh`.
  3. Verify that the system pulls cleanly without conflict, recompiles, and the local `hampod.conf` edits are preserved.

---

## Branch 2: `feature/logging-refactor`
This branch handles changes to how the system outputs information. Isolating this work ensures that if bugs are introduced during the refactor, they don't impact the new CLI utilities developed in Branch 1.

### 4. Logging Refactor
Currently, logging is functional but scattered across different outputs and `/tmp/` files. We will structure this to make debugging straightforward.

- **Centralization:** Centralize all logs into a single directory (e.g., `~/HAMPOD2026/logs/` or `/var/log/hampod/`).
- **Standardization:** Enforce a uniform logging format across both C-code components:
  `[TIMESTAMP] [COMPONENT] [LEVEL] Message` (e.g., `[2026-02-28 15:00:01] [FIRMWARE] [INFO] Keypad initialized.`)
- **Redirection:** Update `hampod start` and individual C applications to consistently append to these specific log files instead of dumping everything to standard output.