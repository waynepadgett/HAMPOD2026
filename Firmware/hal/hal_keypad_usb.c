/**
 * @file hal_keypad_usb.c
 * @brief USB Keypad implementation of the keypad HAL
 * 
 * This implementation reads keypad events from a USB numeric keypad
 * using the Linux input event system (/dev/input/eventX).
 */

#include "hal_keypad.h"
#include <linux/input.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <glob.h>

/* Implementation constants */
#define KEYPAD_DEVICE_PATTERN "/dev/input/by-id/*-kbd*"
#define KEYPAD_EVENT_PATTERN "/dev/input/event*"

/* File descriptor for keypad device */
static int keypad_fd = -1;

/* Keycode to HAMPOD symbol mapping table */
static const struct {
    int keycode;
    char symbol;
} keymap[] = {
    /* Numeric keys 0-9 */
    {KEY_KP0, '0'},
    {KEY_KP1, '1'},
    {KEY_KP2, '2'},
    {KEY_KP3, '3'},
    {KEY_KP4, '4'},
    {KEY_KP5, '5'},
    {KEY_KP6, '6'},
    {KEY_KP7, '7'},
    {KEY_KP8, '8'},
    {KEY_KP9, '9'},
    
    /* Function keys mapped to A-D */
    {KEY_KPSLASH, 'A'},      /* / → A */
    {KEY_KPASTERISK, 'B'},   /* * → B */
    {KEY_KPMINUS, 'C'},      /* - → C */
    {KEY_KPPLUS, 'D'},       /* + → D */
    
    /* Special keys */
    {KEY_KPENTER, '#'},      /* ENTER → # */
    {KEY_KPDOT, '*'},        /* . (DEL) → * (alternative mapping) */
    
    /* End of table marker */
    {-1, '\0'}
};

/**
 * @brief Find USB keypad device
 * 
 * Searches for USB keypad in /dev/input
 * 
 * @param device_path Buffer to store found device path
 * @param max_len Maximum length of buffer
 * @return 0 on success, -1 on failure
 */
static int find_usb_keypad(char* device_path, size_t max_len) {
    glob_t glob_result;
    int ret = -1;
    
    /* Try to find device by ID first (more reliable) */
    if (glob(KEYPAD_DEVICE_PATTERN, 0, NULL, &glob_result) == 0) {
        if (glob_result.gl_pathc > 0) {
            strncpy(device_path, glob_result.gl_pathv[0], max_len - 1);
            device_path[max_len - 1] = '\0';
            ret = 0;
        }
        globfree(&glob_result);
    }
    
    /* If not found by ID, could add fallback logic here */
    /* For now, return error if not found */
    
    return ret;
}

/**
 * @brief Map keycode to HAMPOD symbol
 * 
 * @param keycode Linux input keycode
 * @return HAMPOD symbol character, or '-' if not mapped
 */
static char map_keycode_to_symbol(int keycode) {
    for (int i = 0; keymap[i].keycode != -1; i++) {
        if (keymap[i].keycode == keycode) {
            return keymap[i].symbol;
        }
    }
    return '-';  /* Invalid/unmapped key */
}

/* HAL Implementation Functions */

int hal_keypad_init(void) {
    char device_path[256];
    
    /* Find the USB keypad device */
    if (find_usb_keypad(device_path, sizeof(device_path)) != 0) {
        fprintf(stderr, "HAL Keypad: USB keypad device not found\n");
        return -1;
    }
    
    /* Open the device */
    keypad_fd = open(device_path, O_RDONLY | O_NONBLOCK);
    if (keypad_fd < 0) {
        perror("HAL Keypad: Failed to open device");
        return -1;
    }
    
    printf("HAL Keypad: Initialized USB keypad at %s\n", device_path);
    return 0;
}

KeypadEvent hal_keypad_read(void) {
    KeypadEvent event = {'-', 0, 0};  /* Default: invalid event */
    struct input_event ev;
    ssize_t bytes_read;
    
    /* Check if device is initialized */
    if (keypad_fd < 0) {
        return event;
    }
    
    /* Read input event (non-blocking) */
    bytes_read = read(keypad_fd, &ev, sizeof(ev));
    
    if (bytes_read == sizeof(ev)) {
        /* We only care about key press events */
        if (ev.type == EV_KEY && ev.value == 1) {  /* 1 = key press */
            event.raw_code = ev.code;
            event.key = map_keycode_to_symbol(ev.code);
            event.valid = (event.key != '-') ? 1 : 0;
        }
    }
    
    return event;
}

void hal_keypad_cleanup(void) {
    if (keypad_fd >= 0) {
        close(keypad_fd);
        keypad_fd = -1;
        printf("HAL Keypad: Cleaned up\n");
    }
}

const char* hal_keypad_get_impl_name(void) {
    return "USB Numeric Keypad";
}
