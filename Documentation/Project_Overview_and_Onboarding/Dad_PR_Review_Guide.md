# Dad's Guide: How to Review and Merge a Pull Request

When Amber or a Claude agent finishes a change to HAMPOD, they open a **Pull Request** (PR) on GitHub. A PR is simply a way of saying: "Here are some changes I'd like to add to the project — please take a look before we make them official." You are the final safety check. No coding knowledge is required. Your job is to read what changed, make sure the tests passed, and then click Merge. This guide walks you through every step.

---

## Step 1: Open the Pull Request

Ask Antigravity to create one using GitHub CLI.

---

## Step 2: Read the Summary

When you open the PR, you'll see several things on the page. Focus on these three:

1. **The PR title** — A short plain-English description of what changed (e.g., "Add voice command for weather lookup"). This is written by Amber or the agent.
2. **The description body** — A few sentences or bullet points explaining *why* the change was made. Read this to get the full picture.
3. **The CodeRabbit bot comment** — CodeRabbit is an AI that automatically reviews every PR the moment it is opened. It reads the code and posts a comment summarizing what changed and flagging anything that looks suspicious or risky. Look for a comment from the user **coderabbitai** — it will have a distinctive banner. Read its summary and any warnings it highlights.

If anything in the title, description, or CodeRabbit comment looks wrong, confusing, or unexpected — **text Amber before doing anything else.**

---

## Step 3: Check the Test Results

The Raspberry Pi 5 connected to the HAMPOD project automatically runs a suite of tests on every PR and reports the result back to GitHub. Scroll toward the bottom of the PR page and look for a section called **"Checks"** or **"All checks have passed"** (or failed).

| What you see | What it means |
|---|---|
| Green checkmark ✅ | Tests passed — safe to merge |
| Yellow circle 🟡 | Tests are still running — wait a few minutes and refresh the page |
| Red X ❌ | Tests failed — **DO NOT MERGE** |

### Step 3a: What to do if tests fail (red X)

1. Click the red X to see a summary of which test failed.
2. **Do not merge.** Take a screenshot and text it to Amber.
3. Amber will push a fix. The tests re-run automatically when she does.
4. Once the status goes green ✅, come back and proceed to Step 4.

---

## Step 4: Leave a Comment (Optional but Encouraged)

You can leave a comment on the PR before merging — to ask a question, give feedback, or just say you're good to go. To do this:

- Scroll down past the file changes to the comment box at the bottom of the PR page.
- Type your comment and click **"Comment"**.

Here are some examples of things you might write:
- "Looks good, merging!"
- "Why was this file changed?"
- "Does this affect the IC-7300?"

There are no wrong questions. Amber would rather you ask than wonder.

---

## Step 5: Merge the Pull Request

Once you're satisfied — the description makes sense, CodeRabbit has no red flags, and the tests are green — here's how to merge:

1. Scroll to the very bottom of the PR page.
2. Click the green **"Merge pull request"** button.
3. A small confirmation box appears — click **"Confirm merge"**.
4. The changes are now live in the target branch. Done!
5. GitHub will show a **"Delete branch"** button after merging. It is safe (and tidy) to click it. This removes the temporary working branch that was created just for this change — the code itself is already safely saved in the main branch, so nothing is lost.

---

## Step 6: dev vs main — Which Branch Is This Going Into?

HAMPOD uses two main branches. Think of them like this:

| Branch | What it is | When to merge |
|---|---|---|
| **dev** | The testing version. New features and fixes land here first to be verified before going to the real device. | Merge freely — as long as the tests are green and the change makes sense to you. |
| **main** | The stable production version. This is what actually gets installed and runs on the real HAMPOD hardware. | Only merge here when dev has been running well and Amber explicitly gives you the thumbs up. |

Most PRs you see will be targeting **dev**. PRs targeting **main** are a bigger deal — Amber will always flag those explicitly and walk you through it before asking you to merge.

---

## Quick Reference Card

```
Email or notification arrives about a new PR
→ Open the PR link
→ Read the title + description + CodeRabbit comment
→ Check the test results (wait for green ✅ if needed)
→ Leave a comment if you have any questions
→ Click "Merge pull request" → "Confirm merge"
→ Click "Delete branch"

RULE: Never merge a PR with a red X ❌.
```
