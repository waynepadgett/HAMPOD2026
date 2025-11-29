# HAL Sources
HAL_SRCS = hal/hal_keypad_usb.c hal/hal_audio_usb.c

firmware.elf: firmware.o
	cc -Wall firmware.o $(HAL_SRCS) -o firmware.elf -lpthread -lasound

firmware.o: firmware.c
	cc $(CFLAGS) -Wall -c firmware.c -o firmware.o -lpthread

firmware.c:
	@echo "Missing firmware.c file. Repull the git repo"

debug:
	@echo "Building Debug version"
	cc -DDEBUG -Wall -g -c firmware.c -o firmware.o -lpthread
	cc -Wall firmware.o $(HAL_SRCS) -o firmware.elf -lpthread -lasound

clean:
	@rm *.o
	@rm *.elf
