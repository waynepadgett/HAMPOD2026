# HAMPOD Firmware Makefile
# Raspberry Pi Version with HAL
# Last Updated: December 13, 2025

# Compiler and flags
CC = cc
CFLAGS = -Wall -DSHAREDLIB
LDFLAGS = -lpthread -lasound

# TTS Engine Selection (Default: Piper)
ifndef TTS_ENGINE
TTS_ENGINE = piper
endif

# TTS Speed (Default: 1.0, lower = faster)
ifndef TTS_SPEED
TTS_SPEED = 1.0
endif

# Select TTS HAL source based on engine
ifeq ($(TTS_ENGINE),festival)
TTS_SRC = hal/hal_tts_festival.c
TTS_FLAGS = -DUSE_FESTIVAL
else
TTS_SRC = hal/hal_tts_piper.c hal/hal_tts_cache.c
TTS_FLAGS = -DUSE_PIPER -DPIPER_SPEED=\"$(TTS_SPEED)\"
endif

CFLAGS += $(TTS_FLAGS)

# HAL sources and objects (including TTS HAL and USB util)
HAL_SRCS = hal/hal_keypad_usb.c hal/hal_audio_usb.c hal/hal_usb_util.c $(TTS_SRC)
HAL_OBJS = $(HAL_SRCS:.c=.o)

# Main targets
.PHONY: all clean debug install check-piper

all: firmware.elf imitation_software hampod_firm_packet.o audio_firmware.o keypad_firmware.o

# Pre-build check for Piper (only when TTS_ENGINE=piper)
check-piper:
ifeq ($(TTS_ENGINE),piper)
	@which piper > /dev/null 2>&1 || { \
		echo ""; \
		echo "================================================="; \
		echo "WARNING: 'piper' not found in PATH."; \
		echo "The firmware will fail at runtime."; \
		echo "================================================="; \
		echo "To install Piper, run:"; \
		echo "    ./Documentation/scripts/install_piper.sh"; \
		echo ""; \
		echo "Or build with Festival:"; \
		echo "    make TTS_ENGINE=festival"; \
		echo "================================================="; \
		exit 1; \
	}
endif

# Imitation Software (Integration Test Tool)
imitation_software: imitation_software.c hampod_firm_packet.o
	$(CC) $(CFLAGS) -DSHAREDLIB -o imitation_software imitation_software.c hampod_firm_packet.o

# Main firmware build (depends on check-piper for Piper builds)
firmware.elf: check-piper firmware.o hampod_firm_packet.o audio_firmware.o keypad_firmware.o hampod_queue.o $(HAL_OBJS)
	$(CC) $(CFLAGS) firmware.o hampod_firm_packet.o audio_firmware.o keypad_firmware.o hampod_queue.o $(HAL_OBJS) -o $@ $(LDFLAGS)

firmware.o: firmware.c keypad_firmware.h audio_firmware.h hampod_queue.h hampod_firm_packet.h
	$(CC) $(CFLAGS) -c firmware.c -o firmware.o $(LDFLAGS)

# Individual object files for Software layer linkage
hampod_firm_packet.o: hampod_firm_packet.c hampod_firm_packet.h
	$(CC) $(CFLAGS) -c hampod_firm_packet.c -o hampod_firm_packet.o

hampod_queue.o: hampod_queue.c hampod_queue.h
	$(CC) $(CFLAGS) -c hampod_queue.c -o hampod_queue.o

audio_firmware.o: audio_firmware.c audio_firmware.h hal/hal_audio.h hal/hal_tts.h
	$(CC) $(CFLAGS) -c audio_firmware.c -o audio_firmware.o

keypad_firmware.o: keypad_firmware.c keypad_firmware.h hal/hal_keypad.h
	$(CC) $(CFLAGS) -c keypad_firmware.c -o keypad_firmware.o

# HAL object files
hal/%.o: hal/%.c hal/hal_keypad.h hal/hal_audio.h hal/hal_tts.h hal/hal_usb_util.h
	$(CC) $(CFLAGS) -c $< -o $@

# USB util has fewer dependencies
hal/hal_usb_util.o: hal/hal_usb_util.c hal/hal_usb_util.h
	$(CC) $(CFLAGS) -c $< -o $@

# Debug build with symbols
debug:
	@echo "Building Debug version"
	$(CC) -DDEBUG $(CFLAGS) -g -c firmware.c -o firmware.o $(LDFLAGS)
	$(CC) $(CFLAGS) firmware.o $(HAL_SRCS) -o firmware.elf $(LDFLAGS)

# Clean build artifacts
clean:
	@rm -f *.o *.elf imitation_software
	@rm -f hal/*.o
	@echo "Clean complete"

# Install to system location (optional)
install: firmware.elf
	@echo "Installing firmware.elf to /usr/local/bin/"
	@sudo cp firmware.elf /usr/local/bin/hampod-firmware
	@sudo chmod +x /usr/local/bin/hampod-firmware
	@echo "Installation complete"

