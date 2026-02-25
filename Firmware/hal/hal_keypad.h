#ifndef HAL_KEYPAD_H
#define HAL_KEYPAD_H

/**
 * @file hal_keypad.h
 * @brief Hardware Abstraction Layer for keypad input
 *
 * This HAL provides a unified interface for different keypad implementations
 * (USB, matrix, etc.) allowing the firmware to remain hardware-agnostic.
 */

#include <stdint.h>

/**
 * @brief Keypad event structure
 *
 * Represents a single keypad event with validation status
 */
typedef struct {
  char key; /**< Character representation ('0'-'9', 'A'-'D', '*', '#', or '-'
               for invalid) */
  int raw_code; /**< Raw keycode from device (implementation-specific) */
  unsigned char
      valid; /**< 1 if valid single key, 0 if invalid/multiple/no key */
} KeypadEvent;

/**
 * @brief Initialize keypad hardware
 *
 * Opens and configures the keypad device for reading.
 * Must be called before any other HAL keypad functions.
 *
 * @return 0 on success, negative error code on failure
 */
int hal_keypad_init(void);

/**
 * @brief Read a keypad event (non-blocking)
 *
 * Reads the current state of the keypad. If no key is pressed,
 * or multiple keys are pressed, returns an invalid event.
 *
 * @return KeypadEvent structure with current keypad state
 */
KeypadEvent hal_keypad_read(void);

/**
 * @brief Cleanup keypad resources
 *
 * Closes the keypad device and releases any allocated resources.
 * Should be called before program termination.
 */
void hal_keypad_cleanup(void);

/**
 * @brief Get keypad implementation name
 *
 * Returns a string describing the current keypad implementation
 * (e.g., "USB Keypad", "Matrix Keypad")
 *
 * @return Pointer to static string containing implementation name
 */
const char *hal_keypad_get_impl_name(void);

/**
 * @brief Set keypad layout mode
 *
 * Selects between calculator-style (7-8-9 top, label-based) and
 * phone-style (1-2-3 top, position-based) keypad mapping.
 *
 * @param phone_layout 1 for phone-style, 0 for calculator-style (default)
 */
void hal_keypad_set_phone_layout(int phone_layout);

#endif /* HAL_KEYPAD_H */
