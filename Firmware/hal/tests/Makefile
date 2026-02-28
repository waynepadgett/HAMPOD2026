# HAL Test Programs Makefile
# Compiles test programs for USB keypad, audio, and integration tests
# Last Updated: January 2026

CC = gcc
CFLAGS = -Wall -I.. -DUSE_PIPER -DBEEP_BASE_PATH=\"../../pregen_audio/\" -DPIPER_MODEL_PATH=\"../../models/en_US-lessac-low.onnx\"
LDFLAGS = -lasound -lm
HAL_DIR = ..

# HAL source files
HAL_KEYPAD = $(HAL_DIR)/hal_keypad_usb.c
HAL_AUDIO = $(HAL_DIR)/hal_audio_usb.c
HAL_TTS = $(HAL_DIR)/hal_tts_piper.c
HAL_TTS_CACHE = $(HAL_DIR)/hal_tts_cache.c
HAL_USB_UTIL = $(HAL_DIR)/hal_usb_util.c

# Test executables
TARGETS = test_hal_audio test_hal_usb_util test_hal_keypad test_hal_integration test_interrupt_bypass test_persistent_piper

.PHONY: all clean test

all: $(TARGETS)

# Audio HAL unit tests (automated)
test_hal_audio: test_hal_audio.c $(HAL_AUDIO) $(HAL_TTS) $(HAL_TTS_CACHE) $(HAL_USB_UTIL)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "Built: test_hal_audio"
	@echo "Run with: ./test_hal_audio"

# USB utility tests (automated)
test_hal_usb_util: test_hal_usb_util.c $(HAL_USB_UTIL)
	$(CC) $(CFLAGS) -o $@ $^
	@echo "Built: test_hal_usb_util"
	@echo "Run with: ./test_hal_usb_util"

# Keypad HAL test (manual - waits for key presses)
test_hal_keypad: test_hal_keypad.c $(HAL_KEYPAD)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "Built: test_hal_keypad"
	@echo "Run with: sudo ./test_hal_keypad"
	@echo "NOTE: This test requires manual interaction (pressing keys)"

# Integration test: Keypad + Audio + TTS (manual - waits for key presses)
test_hal_integration: test_hal_integration.c $(HAL_KEYPAD) $(HAL_AUDIO) $(HAL_TTS) $(HAL_TTS_CACHE) $(HAL_USB_UTIL)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "Built: test_hal_integration"
	@echo "Run with: sudo ./test_hal_integration"
	@echo "NOTE: This test requires manual interaction (pressing keys)"

# Interrupt bypass tests (Phase 1 feature)
test_interrupt_bypass: test_interrupt_bypass.c $(HAL_AUDIO) $(HAL_TTS) $(HAL_TTS_CACHE) $(HAL_USB_UTIL)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) -lpthread
	@echo "Built: test_interrupt_bypass"
	@echo "Run with: ./test_interrupt_bypass"

# Persistent Piper tests (Phase 2 feature)
test_persistent_piper: test_persistent_piper.c $(HAL_AUDIO) $(HAL_TTS) $(HAL_TTS_CACHE) $(HAL_USB_UTIL)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "Built: test_persistent_piper"
	@echo "Run with: ./test_persistent_piper"

# Run automated tests only
test: test_hal_audio test_hal_usb_util test_interrupt_bypass
	@echo ""
	@echo "=== Running Automated HAL Tests ==="
	./test_hal_audio
	./test_hal_usb_util
	./test_interrupt_bypass
	@echo ""
	@echo "=== Automated Tests Complete ==="
	@echo "For manual tests, run: sudo ./test_hal_keypad or sudo ./test_hal_integration"

clean:
	rm -f $(TARGETS)
	rm -f /tmp/hampod_test.wav /tmp/hampod_speak.wav
