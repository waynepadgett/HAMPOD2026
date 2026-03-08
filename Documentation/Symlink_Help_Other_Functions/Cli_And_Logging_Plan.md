# Implementation Plan: HAMPOD CLI, Logging, and Maintenance Utilities

## Summary
This plan details the implementation of a centralized `hampod` command-line interface (CLI) to manage the entire HAMPOD system efficiently, and a refactoring of the system's logging mechanisms. 

To follow Git best practices and keep changes isolated, this work will be split across two separate branches:
1.  **`feature/hampod-cli-and-symlink`**: Focuses on creating the new CLI tools and system symlink without altering existing core behavior.
2.  **`feature/logging-refactor`**: Focuses on the more pervasive changes required to centralize and standardize logging across the C components and bash scripts.

---

## Branch 1: `feature/hampod-cli-and-symlink`
This branch implements the new user-facing CLI and update management tools. **To facilitate easier review and testing, this work will be broken down into the following bite-sized commits.**

### Quick Reference TODO List
- TODO: Edge Cases Testing
  - [x] **"Already Running" Start**: Verify `hampod start` doesn't run multiple instances of firmware.elf, fighting over serial ports.
  - [ ] **Config Version Skew**: Check if `restore-config` handles missing fields from old backups gracefully.
  - [ ] **TTS Caching Permissions**: Test `clear-cache` to ensure it removes both root (`/root/.cache/...`) and user (`~/.cache/...`) TTS caches.
  - [ ] **Symlink Pathing**: Ensure `setup_cli.sh` checks if `/usr/local/bin` is in `$PATH` during installation.
- TODO: Define error handling and exit codes for CLI commands
  - [ ] Implement exit code `0` for success.
  - [ ] Implement exit code `2` for invalid arguments.
  - [ ] Implement exit code `3` for runtime/system errors.
  - [ ] Add explicit error handling/checking to the symlink setup script.

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



commit 6



(note that im not sure this is needed double check me on that)
change the update_hampod.sh script to use the same method as the hampod_cli.sh script to find the script directory. and add to the CLI as `update`

side feature: decrease the polling rate for disconnected radios from 5 seconds to 1 second.
note that this doesnt rly belong in this branch, but it is a small change that will make the system more responsive, and i dont commit to main. 

---
### Commit 7: Documentation, Integration, and Pre-Merge Testing
Finalize the branch by documenting the new CLI, integrating the setup script, and ensuring the system is ready for the `main` branch.



- **Documentation Updates:**
  - Add documentation for the `hampod` CLI and all of its commands (`help`, `start`, `update`, `clear-cache`, `reset`, `backup-config`, `restore-config`, and `monitor_mem`) to the root `README.md`.
  - Update `Documentation/Project_Overview_and_Onboarding/RPi_Setup_Guide.md` with any necessary references to the CLI or the symlink setup.
- **Integration:**
  - Call `./Documentation/scripts/setup_cli.sh` from within the main `./Documentation/scripts/install_hampod.sh` script so the symlink is created automatically during a fresh install.




- **Testing Plan:**
  0. do a fresh install. 
  1. Carefully run through the checklist in `Documentation/sop_merge_to_main/Short_Do_This_Before_Merge_To_Main.md`.
  2. Perform exhaustive manual testing of all `hampod` CLI commands to verify unexpected inputs are handled gracefully and permissions (e.g., `sudo`) function identically when called globally vs. locally.
  3. test the config mode on the rpi. 
    note for this i may need to write a test procedure and add it to the short do this before merge to main...

    

extra testing for edge cases

1. The "Already Running" Start
In 

hampod_cli.sh
, the 

start
 command delegates to 

run_hampod.sh
. Does the system check if an instance is already active? If a user accidentally runs hampod start twice, you might end up with two firmware.elf processes fighting over the same serial ports or audio devices.

2. Config Version Skew
When using restore-config, if the user restores a backup from three months ago onto a brand new version of the software that has new mandatory fields in hampod.conf, will the system handle the missing keys gracefully? It might be worth a quick check to see if the firmware "fills in the blanks" with defaults if a key is missing.

3. Permissions on the TTS Cache
Your clear-cache command uses sudo rm -rf /root/.cache/hampod/tts. This is correct since the firmware runs as root, but if a user has been running bits of the system manually (without sudo) for testing, they might have a cache in ~/.cache/hampod/tts as well. Probably a non-issue for the target Pi environment, but something to keep in mind.


5. Symlink Setup Pathing
In setup_cli.sh (which I assume is what the installer calls), make sure it checks if /usr/local/bin is in the user's $PATH. Most modern distros have it, but every now and then a minimal install misses it.


CRITICAL TODO: define error handling for the symlink setup script

Instead of just describing behavior, define it:

All CLI commands must exit with:
0 on success
non-zero on failure
2 for invalid arguments
3 for runtime/system errors





## Branch 2: `feature/logging-refactor`
This branch handles changes to how the system outputs information. Isolating this work ensures that if bugs are introduced during the refactor, they don't impact the new CLI utilities developed in Branch 1.

### 4. Logging Refactor
Currently, logging is functional but scattered across different outputs and `/tmp/` files. We will structure this to make debugging straightforward.

- **Centralization:** Centralize all logs into a single directory (e.g., `~/HAMPOD2026/logs/` or `/var/log/hampod/`).
- **Standardization:** Enforce a uniform logging format across both C-code components:
  `[TIMESTAMP] [COMPONENT] [LEVEL] Message` (e.g., `[2026-02-28 15:00:01] [FIRMWARE] [INFO] Keypad initialized.`)
- **Redirection:** Update `hampod start` and individual C applications to consistently append to these specific log files instead of dumping everything to standard output.

