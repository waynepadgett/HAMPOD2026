# Audio HAL Tests Makefile

CC = gcc
CFLAGS = -Wall -I..

# Source files
HAL_SRC = ../hal_audio_usb.c
TEST_SRC = test_hal_audio.c

# Targets
test_hal_audio: $(TEST_SRC) $(HAL_SRC)
	$(CC) $(CFLAGS) -o $@ $^ -lasound

.PHONY: all clean test

all: test_hal_audio

test: test_hal_audio
	./test_hal_audio

clean:
	rm -f test_hal_audio
