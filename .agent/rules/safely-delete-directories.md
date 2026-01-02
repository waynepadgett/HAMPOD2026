# Safer Directory Deletion

When deleting directories, avoid using `rm -rf` or `rm -r` whenever possible.
Instead, follow this two-step process:
1. Delete the files inside the directory first (e.g., `rm dir/*`).
2. Remove the empty directory using `rmdir dir`.

This prevents accidental mass deletion of nested subdirectories.
