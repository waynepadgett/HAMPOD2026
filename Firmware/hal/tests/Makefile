# Audio HAL Tests Makefile

CC = gcc
CFLAGS = -Wall -I.. -DBEEP_BASE_PATH=\"../../pregen_audio/\"

# Source files
HAL_AUDIO_SRC = ../hal_audio_usb.c
HAL_USB_SRC = ../hal_usb_util.c

# Targets
test_hal_audio: test_hal_audio.c $(HAL_AUDIO_SRC)
	$(CC) $(CFLAGS) -o $@ $^ -lasound

test_hal_usb_util: test_hal_usb_util.c $(HAL_USB_SRC)
	$(CC) $(CFLAGS) -o $@ $^

.PHONY: all clean test

all: test_hal_audio test_hal_usb_util

test: all
	./test_hal_audio
	./test_hal_usb_util

clean:
	rm -f test_hal_audio test_hal_usb_util
