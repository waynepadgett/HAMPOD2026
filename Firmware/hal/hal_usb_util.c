/**
 * @file hal_usb_util.c
 * @brief USB Device Enumeration Utilities Implementation
 *
 * Enumerates audio devices by parsing /proc/asound/cards and resolving
 * USB port paths via /sys/class/sound symlinks.
 */

#include "hal_usb_util.h"
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/**
 * Get the USB port path for an ALSA card by reading symlinks
 */
int hal_usb_get_port_path(int card_number, char *port_path, size_t len) {
  char sysfs_path[256];
  char link_target[512];
  ssize_t link_len;
  char *port_start;

  if (port_path == NULL || len == 0) {
    return -1;
  }

  port_path[0] = '\0';

  /* Read the device symlink for this card */
  snprintf(sysfs_path, sizeof(sysfs_path), "/sys/class/sound/card%d/device",
           card_number);

  link_len = readlink(sysfs_path, link_target, sizeof(link_target) - 1);
  if (link_len < 0) {
    return -1;
  }
  link_target[link_len] = '\0';

  /* The symlink for USB devices looks like:
   * ../../../1-2:1.0  (for card on USB port 1-2)
   * ../../../1-1.4:1.0  (for card on USB hub port 1-1.4)
   * We need to extract the "1-2" or "1-1.4" part.
   * USB ports have format: N-N (bus-port) or N-N.N (bus-port.hub)
   * Non-USB devices look like: ../../../soc/XXXX or ../../../platform/...
   */

  /* Look for the port pattern at the end of the path */
  port_start = strrchr(link_target, '/');
  if (port_start == NULL) {
    port_start = link_target; /* No slash, entire thing is the port */
  } else {
    port_start++; /* Skip the slash */
  }

  /* USB port format: starts with digit, contains '-', may have ':' at end
   * Examples: "1-2:1.0", "1-1.4:1.0", "2-3:1.0" */
  if (port_start[0] >= '1' && port_start[0] <= '9' &&
      strchr(port_start, '-') != NULL) {
    /* This looks like a USB port (e.g., "1-2:1.0") */
    char *interface = strchr(port_start, ':');
    size_t port_len;

    if (interface) {
      port_len = interface - port_start;
    } else {
      port_len = strlen(port_start);
    }

    if (port_len >= len - 30) {
      return -1; /* Buffer too small */
    }

    snprintf(port_path, len, "/sys/bus/usb/devices/%.*s", (int)port_len,
             port_start);
    return 0;
  }

  /* Not a USB device */
  return -1;
}

/**
 * Enumerate all audio devices by parsing /proc/asound/cards
 */
int hal_usb_enumerate_audio(AudioDeviceInfo *devices, int max_devices,
                            int *found) {
  FILE *fp;
  char line[512];
  int count = 0;

  if (devices == NULL || found == NULL || max_devices <= 0) {
    return -1;
  }

  *found = 0;

  fp = fopen("/proc/asound/cards", "r");
  if (fp == NULL) {
    fprintf(stderr, "HAL USB: Failed to open /proc/asound/cards\n");
    return -1;
  }

  /* Parse cards file - format is:
   *  0 [vc4hdmi0      ]: vc4-hdmi - vc4-hdmi-0
   *                      vc4-hdmi-0
   *  2 [Device         ]: USB-Audio - USB2.0 Device
   *                      C-Media Electronics Inc. USB2.0 Device
   */
  while (fgets(line, sizeof(line), fp) != NULL && count < max_devices) {
    int card_num;
    char card_id[64];
    char card_driver[64];
    char card_name[128];

    /* Look for lines starting with a card number */
    if (sscanf(line, " %d [%63[^]]] : %63s - %127[^\n]", &card_num, card_id,
               card_driver, card_name) >= 3) {

      AudioDeviceInfo *dev = &devices[count];
      memset(dev, 0, sizeof(AudioDeviceInfo));

      dev->card_number = card_num;

      /* Use the friendly name from after " - " */
      strncpy(dev->card_name, card_name, sizeof(dev->card_name) - 1);

      /* Build the ALSA device path */
      snprintf(dev->device_path, sizeof(dev->device_path), "plughw:%d,0",
               card_num);

      /* Check if this is a USB device and get port path */
      if (hal_usb_get_port_path(card_num, dev->usb_port,
                                sizeof(dev->usb_port)) == 0) {
        dev->is_usb = 1;
      } else {
        dev->is_usb = 0;
        dev->usb_port[0] = '\0';
      }

      printf("HAL USB: Found card %d: %s [%s] usb=%d\n", dev->card_number,
             dev->card_name, dev->device_path, dev->is_usb);

      count++;
    }
  }

  fclose(fp);
  *found = count;

  return 0;
}

/**
 * Find the best audio device by preference
 *
 * Priority order:
 * 1. Preferred device by name (e.g., "USB2.0 Device") - external USB
 * 2. Any external USB device (excluding internal headphone adapters)
 * 3. Internal headphone (bcm2835 Headphones on Pi3/4, USB Audio CODEC on Pi5)
 * 4. HDMI (only as last resort)
 */
int hal_usb_find_audio(const char *preferred_name, AudioDeviceInfo *result) {
  AudioDeviceInfo devices[MAX_AUDIO_DEVICES];
  int found = 0;
  int i;
  AudioDeviceInfo *preferred = NULL;
  AudioDeviceInfo *external_usb = NULL;
  AudioDeviceInfo *internal_headphone = NULL;
  AudioDeviceInfo *hdmi = NULL;
  AudioDeviceInfo *fallback = NULL;

  if (result == NULL) {
    return -1;
  }

  if (hal_usb_enumerate_audio(devices, MAX_AUDIO_DEVICES, &found) != 0) {
    return -1;
  }

  if (found == 0) {
    fprintf(stderr, "HAL USB: No audio devices found\n");
    return -1;
  }

  /* Scan for matching devices by priority */
  for (i = 0; i < found; i++) {
    AudioDeviceInfo *dev = &devices[i];

    /* Check if this is an internal headphone device:
     * - "bcm2835 Headphones" - Pi 3/4 built-in 3.5mm jack
     * - "USB Audio CODEC" on internal hub - Pi 5 headphone adapter
     * - Anything with "Headphone" in name
     */
    int is_internal_headphone = 0;
    if (strstr(dev->card_name, "bcm2835") != NULL ||
        strstr(dev->card_name, "Headphone") != NULL ||
        strstr(dev->card_name, "headphone") != NULL) {
      is_internal_headphone = 1;
    }
    /* USB Audio CODEC on internal hub (port 1-1.x) is Pi 5 headphone */
    if (strstr(dev->card_name, "USB Audio CODEC") != NULL) {
      if (strstr(dev->usb_port, "1-1.") != NULL) {
        is_internal_headphone = 1;
      }
    }

    /* Check if this is HDMI */
    int is_hdmi = (strstr(dev->card_name, "HDMI") != NULL ||
                   strstr(dev->card_name, "hdmi") != NULL ||
                   strstr(dev->card_name, "vc4-hdmi") != NULL);

    /* Priority 1: Preferred device by name (must be external USB) */
    if (preferred_name && preferred == NULL && dev->is_usb) {
      if (strstr(dev->card_name, preferred_name) != NULL &&
          !is_internal_headphone) {
        preferred = dev;
        printf("HAL USB: Found preferred device: %s\n", dev->card_name);
      }
    }

    /* Priority 2: Any external USB device (not internal headphone, not HDMI) */
    if (external_usb == NULL && dev->is_usb && !is_internal_headphone &&
        !is_hdmi) {
      external_usb = dev;
    }

    /* Priority 3: Internal headphone */
    if (internal_headphone == NULL && is_internal_headphone) {
      internal_headphone = dev;
    }

    /* Priority 4: HDMI */
    if (hdmi == NULL && is_hdmi) {
      hdmi = dev;
    }

    /* Fallback: first device found */
    if (fallback == NULL) {
      fallback = dev;
    }
  }

  /* Select based on priority */
  if (preferred != NULL) {
    *result = *preferred;
    printf("HAL USB: Selected preferred device: %s (%s)\n", result->card_name,
           result->device_path);
  } else if (external_usb != NULL) {
    *result = *external_usb;
    printf("HAL USB: Selected external USB: %s (%s)\n", result->card_name,
           result->device_path);
  } else if (internal_headphone != NULL) {
    *result = *internal_headphone;
    printf("HAL USB: Selected internal headphone: %s (%s)\n", result->card_name,
           result->device_path);
  } else if (hdmi != NULL) {
    *result = *hdmi;
    printf("HAL USB: Selected HDMI: %s (%s)\n", result->card_name,
           result->device_path);
  } else if (fallback != NULL) {
    *result = *fallback;
    printf("HAL USB: Selected fallback device: %s (%s)\n", result->card_name,
           result->device_path);
  } else {
    fprintf(stderr, "HAL USB: No suitable audio device found\n");
    return -1;
  }

  return 0;
}
