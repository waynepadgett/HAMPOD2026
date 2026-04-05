# Expected Transcript Fixtures

This directory holds `.txt` files containing the expected TTS output for each
audio HITL test. After whisper.cpp transcribes a captured audio clip, the
result is compared against the corresponding file here.

## Naming Convention

```
<test_name>.txt
```

Example:
```
startup_announcement.txt
key_press_1.txt
key_press_enter.txt
mode_transition_frequency.txt
```

## Format

Each file contains the expected spoken text, lowercase, no punctuation
(to match whisper.cpp output format):

```
# startup_announcement.txt
system ready
```

```
# key_press_1.txt
one
```

## Matching Strategy

Exact match is too brittle (whisper.cpp may vary slightly). Use a fuzzy match
or check that the expected string is a substring of the transcript. Define
the matching logic in each test script and document the threshold here once
decided.
