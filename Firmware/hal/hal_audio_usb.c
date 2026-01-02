/**
 * @file hal_audio_usb.c
 * @brief USB Audio implementation of the audio HAL
 * 
 * This implementation plays audio through a USB audio device
 * using ALSA (aplay command).
 */

#include "hal_audio.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Default audio device - uses system default which should be configured for dmix */
static char audio_device[256] = "default";
static int initialized = 0;

/**
 * @brief Detect USB audio device
 * 
 * Parses output of 'aplay -l' to find USB audio device
 * 
 * @return 0 on success, -1 on failure
 */
static int detect_usb_audio(void) {
    FILE *fp;
    char line[512];
    char card_name[64] = "";
    
    /* Run aplay -l to list audio devices */
    fp = popen("aplay -l 2>/dev/null", "r");
    if (fp == NULL) {
        fprintf(stderr, "HAL Audio: Failed to run aplay -l\n");
        return -1;
    }
    
    /* Parse output looking for USB device */
    while (fgets(line, sizeof(line), fp) != NULL) {
        /* Look for card line, typically: "card 1: Device [...]" */
        if (strstr(line, "card") && strstr(line, ":")) {
            /* Extract card number and name */
            int card_num;
            char name[128];
            if (sscanf(line, "card %d: %127s", &card_num, name) == 2) {
                /* Remove trailing colon if present */
                char *colon = strchr(name, ',');
                if (colon) *colon = '\0';
                
                /* Prefer USB or Device in the name */
                if (strstr(line, "USB") || strstr(name, "Device")) {
                    /* Use plughw format for direct access without needing .asoundrc */
                    snprintf(audio_device, sizeof(audio_device), "plughw:%d,0", card_num);
                    pclose(fp);
                    return 0;
                }
                
                /* Save first device as fallback */
                if (card_name[0] == '\0') {
                    strncpy(card_name, name, sizeof(card_name) - 1);
                }
            }
        }
    }
    
    pclose(fp);
    
    /* If USB not found but we have a device, use that */
    if (card_name[0] != '\0') {
        snprintf(audio_device, sizeof(audio_device), 
                 "sysdefault:CARD=%s", card_name);
        return 0;
    }
    
    /* No audio device found */
    fprintf(stderr, "HAL Audio: No audio devices found\n");
    return -1;
}

/* HAL Implementation Functions */

int hal_audio_init(void) {
    if (initialized) {
        return 0;  /* Already initialized */
    }
    
    /* Attempt to detect USB audio device */
    if (detect_usb_audio() != 0) {
        fprintf(stderr, "HAL Audio: Using default device: %s\n", audio_device);
        /* Continue anyway with default, don't fail */
    } else {
        printf("HAL Audio: Detected audio device: %s\n", audio_device);
    }
    
    initialized = 1;
    return 0;
}

int hal_audio_set_device(const char* device_name) {
    if (device_name == NULL) {
        return -1;
    }
    
    strncpy(audio_device, device_name, sizeof(audio_device) - 1);
    audio_device[sizeof(audio_device) - 1] = '\0';
    
    printf("HAL Audio: Device set to: %s\n", audio_device);
    return 0;
}

const char* hal_audio_get_device(void) {
    return audio_device;
}

int hal_audio_play_file(const char* filepath) {
    char command[1024];
    int result;
    
    if (!initialized) {
        fprintf(stderr, "HAL Audio: Not initialized\n");
        return -1;
    }
    
    if (filepath == NULL) {
        return -1;
    }
    
    /* Build aplay command with device specification */
    snprintf(command, sizeof(command), 
             "aplay -D %s '%s'", 
             audio_device, filepath);
    
    /* Execute command */
    result = system(command);
    
    if (result != 0) {
        fprintf(stderr, "HAL Audio: Failed to play file: %s\n", filepath);
        return -1;
    }
    
    return 0;
}

void hal_audio_cleanup(void) {
    initialized = 0;
    printf("HAL Audio: Cleaned up\n");
}

const char* hal_audio_get_impl_name(void) {
    return "USB Audio (ALSA)";
}
