/**
 * @file test_hal_usb_util.c
 * @brief Unit tests for USB device enumeration utility
 */

#include "../hal_usb_util.h"
#include <stdio.h>
#include <string.h>

static int tests_passed = 0;
static int tests_failed = 0;

#define TEST_ASSERT(cond, msg)                                                 \
  do {                                                                         \
    if (cond) {                                                                \
      printf("  [PASS] %s\n", msg);                                            \
      tests_passed++;                                                          \
    } else {                                                                   \
      printf("  [FAIL] %s\n", msg);                                            \
      tests_failed++;                                                          \
    }                                                                          \
  } while (0)

void test_enumerate_audio(void) {
  printf("\n=== Test: Enumerate Audio Devices ===\n");

  AudioDeviceInfo devices[MAX_AUDIO_DEVICES];
  int found = 0;

  int result = hal_usb_enumerate_audio(devices, MAX_AUDIO_DEVICES, &found);

  TEST_ASSERT(result == 0, "hal_usb_enumerate_audio returns success");
  TEST_ASSERT(found > 0, "At least one audio device found");

  printf("  Found %d audio devices:\n", found);
  for (int i = 0; i < found; i++) {
    printf("    Card %d: %s (%s) USB=%d Port=%s\n", devices[i].card_number,
           devices[i].card_name, devices[i].device_path, devices[i].is_usb,
           devices[i].usb_port[0] ? devices[i].usb_port : "N/A");
  }
}

void test_find_audio_preferred(void) {
  printf("\n=== Test: Find Audio Device (Preferred) ===\n");

  AudioDeviceInfo result;

  int ret = hal_usb_find_audio("USB2.0 Device", &result);

  if (ret == 0) {
    TEST_ASSERT(result.card_name[0] != '\0', "Device name populated");
    TEST_ASSERT(result.device_path[0] != '\0', "Device path populated");
    printf("  Selected: %s (%s)\n", result.card_name, result.device_path);

    if (result.is_usb) {
      TEST_ASSERT(result.usb_port[0] != '\0',
                  "USB port path populated for USB device");
      printf("  USB Port: %s\n", result.usb_port);
    }
  } else {
    printf("  [INFO] No preferred device found, will fall back\n");
  }
}

void test_find_audio_any_usb(void) {
  printf("\n=== Test: Find Audio Device (Any USB) ===\n");

  AudioDeviceInfo result;

  int ret = hal_usb_find_audio(NULL, &result);

  TEST_ASSERT(ret == 0, "hal_usb_find_audio returns a device");

  if (ret == 0) {
    printf("  Selected: %s (%s) USB=%d\n", result.card_name, result.device_path,
           result.is_usb);
    if (result.usb_port[0]) {
      printf("  USB Port: %s\n", result.usb_port);
    }
  }
}

void test_get_port_path(void) {
  printf("\n=== Test: Get USB Port Path ===\n");

  AudioDeviceInfo devices[MAX_AUDIO_DEVICES];
  int found = 0;

  if (hal_usb_enumerate_audio(devices, MAX_AUDIO_DEVICES, &found) != 0 ||
      found == 0) {
    printf("  [SKIP] No devices to test\n");
    return;
  }

  /* Find a USB device to test */
  for (int i = 0; i < found; i++) {
    if (devices[i].is_usb) {
      char port_path[128];
      int ret = hal_usb_get_port_path(devices[i].card_number, port_path,
                                      sizeof(port_path));

      TEST_ASSERT(ret == 0, "hal_usb_get_port_path succeeds for USB device");
      TEST_ASSERT(port_path[0] != '\0', "Port path is not empty");
      TEST_ASSERT(strstr(port_path, "/sys/bus/usb/devices/") != NULL,
                  "Port path has expected prefix");

      printf("  Card %d port: %s\n", devices[i].card_number, port_path);
      return;
    }
  }

  printf("  [SKIP] No USB devices available to test port path\n");
}

int main(void) {
  printf("=============================================\n");
  printf("  HAMPOD USB Utility Unit Tests\n");
  printf("=============================================\n");

  test_enumerate_audio();
  test_find_audio_preferred();
  test_find_audio_any_usb();
  test_get_port_path();

  printf("\n=============================================\n");
  printf("  Results: %d passed, %d failed\n", tests_passed, tests_failed);
  printf("=============================================\n");

  return tests_failed > 0 ? 1 : 0;
}
