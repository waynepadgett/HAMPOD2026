# Radio Test Infrastructure

## Radio Detection

Before running radio tests, the script must detect whether a supported radio
is connected and powered on. If not, the job exits 0 (skip) rather than failing.

Detection uses hamlib's `rigctl` to attempt a quick connection:

```bash
# IC-7300 example (hamlib model 3085, adjust port as needed)
RADIO_PORT="/dev/ttyUSB0"
if rigctl -m 3085 -r "$RADIO_PORT" -C retry=1 get_freq > /dev/null 2>&1; then
    echo "Radio detected — running radio tests."
else
    echo "No radio detected — skipping radio tests."
    exit 0
fi
```

Run `rigctl -l | grep -i icom` or `rigctl -l | grep -i kenwood` to find the
correct hamlib model number for each supported radio.

## Supported Radios

| Radio       | Hamlib Model | Default Port |
|-------------|-------------|--------------|
| IC-7300     | 3085        | /dev/ttyUSB0 |
| TS-570      | 228         | /dev/ttyUSB0 |
| TS-2000     | 221         | /dev/ttyUSB0 |

## Hamlib Virtual/Dummy Radio

Hamlib includes a dummy radio backend (`RIG_MODEL_DUMMY`, model number 1) that
simulates a radio entirely in software. This could allow radio control logic to
be tested without physical hardware.

**Status: Needs research.** The dummy backend may not accurately simulate the
CI-7300 or TS-570 command sets. If it supports the commands HAMPOD uses
(frequency get/set, mode, PTT), some radio tests could be moved to Job 1
(no-radio) using the dummy backend.

To test the dummy backend manually:
```bash
rigctl -m 1  # Interactive dummy radio session
```

## Planned Test Coverage

- [ ] Radio connection detection (hamlib probe)
- [ ] Frequency read-back after entering a frequency via keypad
- [ ] Band change confirmation announcement
- [ ] Connect/disconnect recovery (hamlib polling behavior)
- [ ] PTT state handling (future)
