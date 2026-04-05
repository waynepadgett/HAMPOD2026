# Input Simulation Infrastructure

## Approach

HAMPOD reads keypad input via `/dev/input/` event devices (USB HID). To simulate
key presses in CI without a human pressing physical keys, we need to inject
synthetic input events.

## Options (TBD — pick one when implementing)

### Option A: `uinput` virtual device (preferred)
The Linux `uinput` kernel module lets you create a virtual input device and
write key events to it from userspace. HAMPOD's keypad HAL would see it as a
real USB keypad.

```bash
# Requires: python3-uinput or evdev library
# Example (Python):
import uinput
device = uinput.Device([uinput.KEY_1, uinput.KEY_ENTER, ...])
device.emit_click(uinput.KEY_1)
```

### Option B: `evemu` / `evemu-play`
Record real keypad events with `evemu-record`, then replay them with
`evemu-play` in CI. Good for complex multi-key sequences.

```bash
# Record (run once with real keypad, save to file):
evemu-record /dev/input/eventX > input/fixtures/press_1.evemu

# Replay in CI:
evemu-play /dev/input/eventX < input/fixtures/press_1.evemu
```

### Option C: pyserial (if keypad is serial-based)
If the keypad communicates via a serial protocol rather than raw HID, pyserial
can be used to send the expected byte sequences directly to the device file.

## Note on pyserial

The initial design notes mention pyserial. Verify how the USB keypad presents
itself on the Pi (`ls /dev/input/by-id/`) before choosing the input simulation
method — most USB keypads are HID and won't use a serial port.
