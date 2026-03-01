# Multiple TTS Synthesizers Support

## Goal
Enable runtime switching between TTS engines (Piper, Flite, Festival) via `hampod.conf`.

## Key Design Decisions

### Audio Pipeline Compatibility
- **All Engines:** Output 16kHz/mono/16-bit
- **Latency:** `hal_audio_play_file()` streams directly from RAM (`/dev/shm`) and handles interrupts instantly via `snd_pcm_drop()`. Since synthesis is decoupled from playback via RAM file, total latency for Flite/Festival = (Synthesis Time) + (File Write Time). Compute time for these is much lower than Piper, so real-time experience remains excellent.

### RAM-Based Audio for Flite/Festival
Both write to `/dev/shm/hampod_tts.wav` (tmpfs = pure RAM), avoiding SD card wear and I/O latency.

### Per-Engine Config Blocks
```ini
[audio]
tts_engine = piper

[tts.piper]
# speed: 0.1 = very fast, 1.0 = normal, 3.0 = very slow (Piper length_scale)
speed = 0.8
noise_scale = 0.667
noise_w = 0.8

[tts.flite]
voice = slt
# speed: 0.5 = fast, 1.0 = normal, 2.0 = slow (Flite duration_stretch)
speed = 1.0

[tts.festival]
voice = kal_diphone
# speed: 0.5 = fast, 1.0 = normal, 2.0 = slow (Festival Duration_Stretch)
speed = 1.0
```

### Speed Control Implementation
All three engines support speed control via duration-stretch parameters:
- **Piper:** `--length_scale X` (0.1–3.0, lower = faster)
- **Flite:** `--setf duration_stretch=X` (0.5–2.0, lower = faster)
- **Festival:** `text2wave -eval "(Parameter.set 'Duration_Stretch X)"` (0.5–2.0, lower = faster)

All use the same semantic: **lower value = faster speech**.

---

## Architecture
(unchanged)

---

## Implementation Phases

### Phase 0: Setup
- Create feature branch: `git checkout -b feature/multiple-synthesizers`
- Create test scaffold: `Firmware/tests/test_tts_switching.c`

**Verify:** Branch created, test compiles.

---

### Phase 1: Engine Interface + Refactor Existing Code

#### [NEW] [hal_tts_engine.h](file:///Users/waynepadgett/Documents/developer/HAMPOD2026/Firmware/hal/hal_tts_engine.h)
Define `TtsEngine` struct with function pointers:
```c
typedef struct {
  int (*init)(void);
  int (*speak)(const char *text, const char *output_file);
  void (*interrupt)(void);
  void (*cleanup)(void);
  const char* (*get_name)(void);
  int (*set_speed)(float speed);
} TtsEngine;
```

#### [MODIFY] [hal_tts_piper.c](file:///Users/waynepadgett/Documents/developer/HAMPOD2026/Firmware/hal/hal_tts_piper.c)
Rename functions: `hal_tts_init` → `piper_init`, etc. Export `TtsEngine piper_engine`.

#### [MODIFY] [hal_tts_festival.c](file:///Users/waynepadgett/Documents/developer/HAMPOD2026/Firmware/hal/hal_tts_festival.c)
Rename functions. Implement `festival_set_speed()` using `Duration_Stretch`.
Use `/dev/shm/hampod_tts.wav` and `text2wave -F 16000`.

**Unit Test:** Verify function pointers populate correctly.
**Verify:** `make` succeeds.

---

### Phase 2: Flite Implementation

#### [NEW] [hal_tts_flite.c](file:///Users/waynepadgett/Documents/developer/HAMPOD2026/Firmware/hal/hal_tts_flite.c)
- `flite_init()` — check for `flite` binary
- `flite_speak()` — `flite -voice slt --setf duration_stretch=X -o /dev/shm/hampod_tts.wav "text"` + `hal_audio_play_file()`
- `flite_interrupt()` — kill flite process + `hal_audio_interrupt()`
- `flite_set_speed()` — Updates internal `duration_stretch` variable.

**Unit Test:** Verify `flite_init()` detects missing binary gracefully.
**Verify:** `which flite` on Pi, compile succeeds.

---

### Phase 3: Dispatch Layer

#### [NEW] [hal_tts_dispatch.c](file:///Users/waynepadgett/Documents/developer/HAMPOD2026/Firmware/hal/hal_tts_dispatch.c)
```c
static TtsEngine *active_engine = NULL;

int hal_tts_switch_engine(const char *name) {
  // cleanup current → select new → init
  // re-apply current speed setting to new engine
}
```
Implements all `hal_tts_*()` functions by delegating to `active_engine->*()`.

#### [MODIFY] [hal_tts.h](file:///Users/waynepadgett/Documents/developer/HAMPOD2026/Firmware/hal/hal_tts.h)
Add `int hal_tts_switch_engine(const char *engine_name);`

**Unit Test:** Switch engines, verify `hal_tts_get_impl_name()` changes.
**Verify:** Default engine (piper) still works end-to-end.

---

### Phase 4: Config + Communication

#### [MODIFY] [config.c](file:///Users/waynepadgett/Documents/developer/HAMPOD2026/Software2/src/config.c)
Parse `tts_engine` from `[audio]`. Add `config_get_tts_engine()`.

#### [MODIFY] [comm.c](file:///Users/waynepadgett/Documents/developer/HAMPOD2026/Software2/src/comm.c) / [comm.h](file:///Users/waynepadgett/Documents/developer/HAMPOD2026/Software2/include/comm.h)
Add `comm_set_tts_engine(const char *engine);` — sends 'e' packet.

#### [MODIFY] [audio_firmware.c](file:///Users/waynepadgett/Documents/developer/HAMPOD2026/Firmware/audio_firmware.c)
Handle 'e' packet → call `hal_tts_switch_engine()`.

#### [MODIFY] [main.c](file:///Users/waynepadgett/Documents/developer/HAMPOD2026/Software2/src/main.c)
Call `comm_set_tts_engine(config_get_tts_engine())` at startup.

**Verify:** Change config, restart, check log for engine switch.

---

### Phase 5: Makefile + Config File

#### [MODIFY] [makefile](file:///Users/waynepadgett/Documents/developer/HAMPOD2026/Firmware/makefile)
Link all engines: `hal_tts_piper.o hal_tts_flite.o hal_tts_festival.o hal_tts_dispatch.o`. Remove `TTS_ENGINE` conditional.

#### [MODIFY] [hampod.conf](file:///Users/waynepadgett/Documents/developer/HAMPOD2026/Software2/config/hampod.conf)
Add `tts_engine` and per-engine sections.

**Verify:** `make clean && make` works.

---

### Phase 6: Integration Test

#### [NEW] [test_tts_integration.c](file:///Users/waynepadgett/Documents/developer/HAMPOD2026/Firmware/tests/test_tts_integration.c)
- Switch to each engine, verify name
- Speak test phrase with each engine
- Verify interrupt works on each
- Switch engines mid-session

**Manual Verification:**
1. `tts_engine = piper` → hear Piper voice
2. `tts_engine = flite` → hear Flite voice
3. `tts_engine = festival` → hear Festival voice
4. Speed setting works with ALL engines

---

## Merge Criteria
- [ ] All phases complete
- [ ] Unit and integration tests pass
- [ ] Manual testing approved by user
- [ ] Documentation updated
