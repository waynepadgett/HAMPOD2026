# Interrupt Fix and Persistent Piper Implementation

> [!NOTE]
> **STATUS: ✅ COMPLETE - MERGED TO MAIN**
> - Phase 1 (Interrupt Bypass): ✅ Implemented
> - Phase 2 (Persistent Piper): ✅ Working
> - Phase 3 (Direct ALSA): ✅ Implemented
> - Beep Support: ✅ Working (with occasional double-beep on recovery)

---

## Summary

This document describes the implementation of three major audio improvements:

1. **Direct ALSA Audio** - Replaces `aplay` pipeline with `snd_pcm_*` API
2. **True Interrupt Capability** - Uses `snd_pcm_drop()` for immediate stop
3. **Persistent Piper TTS** - Keeps Piper running for faster response times
4. **Beep Support** - Keypress beeps via IO thread bypass

---

## Implementation Details

### Direct ALSA Audio (hal_audio_usb.c)

Replaced `popen("aplay ...")` with direct ALSA PCM API:

- **`snd_pcm_open()`** - Opens USB audio device directly
- **`snd_pcm_writei()`** - Writes audio samples to device
- **`snd_pcm_drop()`** - Immediately stops and discards buffer (interrupt)
- **`snd_pcm_prepare()`** - Resets device for new audio
- **`snd_pcm_drain()`** - Waits for current audio to finish (beeps)

Configuration:
- Sample rate: 16kHz (matching Piper)
- Channels: Mono
- Format: 16-bit signed little-endian
- Buffer: 100ms (1600 samples)
- Period: 25ms (400 samples)

### Interrupt Bypass (audio_firmware.c)

The IO thread now handles special packets immediately without queueing:

1. **Interrupt packets ('i')**: Calls `hal_audio_interrupt()` and `hal_tts_interrupt()`, clears queue
2. **Beep packets ('b')**: Plays beep immediately via `hal_audio_play_beep()`

This ensures interrupts and beeps are processed even when TTS is blocking the main audio thread.

### Persistent Piper (hal_tts_piper.c)

Piper TTS runs as a persistent subprocess:

- Started once at init, reused for all utterances
- Text sent via stdin, audio received via stdout
- Newline acts as sentence delimiter
- Empty line flushes/triggers generation
- Interrupt drains stdout pipe to discard buffered audio

### Beep Playback

Beeps are cached in RAM at startup from WAV files:
- `pregen_audio/beep_keypress.wav` - 50ms, 1000Hz
- `pregen_audio/beep_hold.wav` - 100ms, 500Hz
- `pregen_audio/beep_error.wav` - 200ms, 400Hz

Beep flow:
1. IO thread receives 'b' packet with beep type
2. Clears interrupt flag
3. Writes samples directly to ALSA
4. Explicitly starts playback if device was idle
5. Drains buffer to ensure beep completes
6. Prepares device for next audio

**Known issue**: Occasional double-beep when ALSA recovery is triggered. This is a tradeoff for reliable beep playback.

---

## Key Files Modified

| File | Changes |
|------|---------|
| `Firmware/hal/hal_audio_usb.c` | Direct ALSA implementation, beep caching |
| `Firmware/hal/hal_tts_piper.c` | Persistent Piper subprocess |
| `Firmware/audio_firmware.c` | Interrupt and beep bypass in IO thread |
| `Software2/src/speech.c` | Interrupt sends 'i' packet |
| `Firmware/hampod_queue.c` | Added `clear_queue()` function |

---

## Testing

Verified behavior:
- ✅ TTS interrupts immediately when new key pressed
- ✅ Keypress beeps play before TTS response
- ✅ Rapid keypresses don't cause hangs
- ✅ Slow keypresses produce beeps
- ✅ Beeps work when interrupting speech

---

## Architecture Diagram

```
Software2                    Firmware
┌─────────────┐             ┌─────────────────────────────┐
│  keypad.c   │──beep──────▶│  IO Thread                  │
│  speech.c   │──interrupt─▶│   ├─ 'i' → hal_audio_interrupt()
│             │──tts───────▶│   ├─ 'b' → hal_audio_play_beep()
└─────────────┘             │   └─ 'd' → queue → Main Thread
                            │                              │
                            │  Main Thread                 │
                            │   └─ hal_tts_speak()         │
                            │        └─ hal_audio_write_raw()
                            └─────────────────────────────┘
                                         │
                                         ▼
                            ┌─────────────────────────────┐
                            │  ALSA (snd_pcm_*)           │
                            │   └─ USB Audio Device       │
                            └─────────────────────────────┘
```
