# Dad's PR Review Guide

## One-Time Setup

1. Install GitHub CLI: `brew install gh`
2. Authenticate: `gh auth login` — follow the prompts (choose GitHub.com, HTTPS, browser)

After that, Antigravity can handle opening PRs for you.

---

## Reviewing a PR (Every Time)

**1. Ask Antigravity to open the PR.** It will post a GitHub link.

**2. Click the link. Read:**
- The **title and description** — what changed and why
- The **coderabbitai** bot comment — AI review, read any warnings it flags

If anything looks wrong or unexpected → **text Amber before continuing.**

**3. Check the Checks section** (near the bottom of the page):

| Status | Action |
|---|---|
| ✅ green | Safe to merge |
| 🟡 yellow | Wait a few minutes, then refresh |
| ❌ red | **DO NOT MERGE** — screenshot and text Amber |

**4. Merge:**
Click **"Merge pull request"** → **"Confirm merge"** → **"Delete branch"**

---

**Rule: Never merge a red ❌.**

**Branch note:** All development happens on **dev**. When Amber is ready to deploy to the real device, she opens a PR from dev → **main**. Main is what actually runs on the hardware — that's the only PR you'll see.
