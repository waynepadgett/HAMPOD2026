# HAMPOD Firmware Makefile
# Raspberry Pi Version with HAL
# Last Updated: November 29, 2025

# Compiler and flags
CC = cc
CFLAGS = -Wall -DSHAREDLIB
LDFLAGS = -lpthread -lasound

# HAL sources and objects
HAL_SRCS = hal/hal_keypad_usb.c hal/hal_audio_usb.c
HAL_OBJS = $(HAL_SRCS:.c=.o)

# Main targets
.PHONY: all clean debug install

all: firmware.elf hampod_firm_packet.o audio_firmware.o keypad_firmware.o

# Main firmware build
firmware.elf: firmware.o $(HAL_OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

firmware.o: firmware.c keypad_firmware.h audio_firmware.h hampod_queue.h hampod_firm_packet.h
	$(CC) $(CFLAGS) -c firmware.c -o firmware.o $(LDFLAGS)

# Individual object files for Software layer linkage
hampod_firm_packet.o: hampod_firm_packet.c hampod_firm_packet.h
	$(CC) $(CFLAGS) -c hampod_firm_packet.c -o hampod_firm_packet.o

audio_firmware.o: audio_firmware.c audio_firmware.h hal/hal_audio.h
	$(CC) $(CFLAGS) -c audio_firmware.c -o audio_firmware.o

keypad_firmware.o: keypad_firmware.c keypad_firmware.h hal/hal_keypad.h
	$(CC) $(CFLAGS) -c keypad_firmware.c -o keypad_firmware.o

# HAL object files
hal/%.o: hal/%.c hal/hal_keypad.h hal/hal_audio.h
	$(CC) $(CFLAGS) -c $< -o $@

# Debug build with symbols
debug:
	@echo "Building Debug version"
	$(CC) -DDEBUG $(CFLAGS) -g -c firmware.c -o firmware.o $(LDFLAGS)
	$(CC) $(CFLAGS) firmware.o $(HAL_SRCS) -o firmware.elf $(LDFLAGS)

# Clean build artifacts
clean:
	@rm -f *.o *.elf
	@rm -f hal/*.o
	@echo "Clean complete"

# Install to system location (optional)
install: firmware.elf
	@echo "Installing firmware.elf to /usr/local/bin/"
	@sudo cp firmware.elf /usr/local/bin/hampod-firmware
	@sudo chmod +x /usr/local/bin/hampod-firmware
	@echo "Installation complete"
