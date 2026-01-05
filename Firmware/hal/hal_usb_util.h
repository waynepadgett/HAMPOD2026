/**
 * @file hal_usb_util.h
 * @brief USB Device Enumeration Utilities
 *
 * Provides functions to enumerate USB audio devices and get their
 * physical port paths for deterministic device selection.
 */

#ifndef HAL_USB_UTIL_H
#define HAL_USB_UTIL_H

#include <stddef.h>

/**
 * Information about an audio device
 */
typedef struct {
  char card_name[64];   /* e.g., "USB2.0 Device" */
  int card_number;      /* ALSA card number */
  char usb_port[128];   /* e.g., "/sys/bus/usb/devices/1-2" */
  char device_path[64]; /* e.g., "plughw:2,0" */
  int is_usb;           /* 1 if USB device, 0 otherwise */
} AudioDeviceInfo;

#define MAX_AUDIO_DEVICES 16

/**
 * Enumerate all audio devices on the system
 *
 * @param devices Array to fill with device info
 * @param max_devices Maximum number of devices to enumerate
 * @param found Output: number of devices found
 * @return 0 on success, -1 on failure
 */
int hal_usb_enumerate_audio(AudioDeviceInfo *devices, int max_devices,
                            int *found);

/**
 * Find the best audio device by preference
 *
 * Priority order:
 * 1. Device matching preferred_name (e.g., "USB2.0 Device")
 * 2. Any external USB audio device
 * 3. Headphones
 * 4. Default system audio
 *
 * @param preferred_name Preferred device name (or NULL for any USB)
 * @param result Output: selected device info
 * @return 0 if found, -1 if no audio device available
 */
int hal_usb_find_audio(const char *preferred_name, AudioDeviceInfo *result);

/**
 * Get the USB port path for an ALSA card
 *
 * @param card_number ALSA card number
 * @param port_path Output buffer for port path
 * @param len Size of port_path buffer
 * @return 0 on success, -1 if not a USB device or error
 */
int hal_usb_get_port_path(int card_number, char *port_path, size_t len);

#endif /* HAL_USB_UTIL_H */
