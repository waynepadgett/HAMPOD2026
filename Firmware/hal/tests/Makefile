# HAL Test Programs Makefile
# Compiles test programs for USB keypad and audio HAL

CC = gcc
CFLAGS = -Wall -I.. -lm
HAL_DIR = ..

# Test executables
TARGETS = test_keypad test_audio test_integration

all: $(TARGETS)

test_keypad: test_hal_keypad.c $(HAL_DIR)/hal_keypad_usb.c
	$(CC) -o $@ $^ $(CFLAGS)
	@echo "Built: test_keypad"
	@echo "Run with: ./test_keypad"

test_audio: test_hal_audio.c $(HAL_DIR)/hal_audio_usb.c
	$(CC) -o $@ $^ $(CFLAGS)
	@echo "Built: test_audio"
	@echo "Run with: ./test_audio"

test_integration: test_hal_integration.c $(HAL_DIR)/hal_keypad_usb.c $(HAL_DIR)/hal_audio_usb.c
	$(CC) -o $@ $^ $(CFLAGS)
	@echo "Built: test_integration"
	@echo "Run with: ./test_integration"

clean:
	rm -f $(TARGETS)
	rm -f /tmp/hampod_test.wav /tmp/hampod_speak.wav

.PHONY: all clean
