# Audio Test Infrastructure

## Approach

Instead of playing audio through a physical speaker, tests route Piper TTS output
through the ALSA loopback module (`snd-aloop`). The loopback creates a virtual
soundcard with a paired playback/capture device — audio written to the playback
side can be recorded from the capture side programmatically.

After capturing, `whisper.cpp` transcribes the recording to text, and the
transcript is compared against an expected string stored in `../expected/`.

## Setup

```bash
# Load the loopback module (persist across reboots via /etc/modules)
sudo modprobe snd-aloop

# Verify it appears as a soundcard
aplay -l | grep Loopback
```

Configure HAMPOD to use the loopback playback device when running under CI.
The capture device on the other side can then be read with `arecord`.

## whisper.cpp

Build whisper.cpp on the Pi and place the binary somewhere on `$PATH`, or set
`WHISPER_BIN` in the test environment. A small model (e.g., `ggml-tiny.en.bin`)
is sufficient for verifying short TTS phrases.

```bash
# Example: transcribe a captured wav
$WHISPER_BIN -m /path/to/ggml-tiny.en.bin -f /tmp/capture.wav --no-timestamps
```

## Test Flow (per test)

1. Start HAMPOD (or the relevant subprocess) pointed at the loopback playback device
2. Start `arecord` capturing from the loopback capture device → `/tmp/capture.wav`
3. Trigger the action under test (key press simulation, startup, etc.)
4. Stop recording after a fixed timeout or silence detection
5. Run whisper.cpp on `/tmp/capture.wav` → transcript
6. Compare transcript to `../expected/<test_name>.txt`
7. Pass if strings match (fuzzy match ok for minor whisper variations)
