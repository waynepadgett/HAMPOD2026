/**
 * @file hal_keypad_usb.c
 * @brief USB Keypad implementation of the keypad HAL
 *
 * This implementation reads keypad events from a USB numeric keypad
 * using the Linux input event system (/dev/input/eventX).
 */

#include "hal_keypad.h"
#include <fcntl.h>
#include <glob.h>
#include <linux/input.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

/* Implementation constants */
#define KEYPAD_DEVICE_PATTERN "/dev/input/by-id/*-kbd*"
#define KEYPAD_EVENT_PATTERN "/dev/input/event*"

/* Set to 1 for phone-style keypad (1-2-3 on top row, positional mapping),
   set to 0 for calculator-style keypad (7-8-9 on top row, label mapping) */
#define KEYPAD_PHONE_LAYOUT 0

/* File descriptor for keypad device */
static int keypad_fd = -1;

#if !KEYPAD_PHONE_LAYOUT
/* Debouncing state for '00' key (calculator mode only) */
static struct {
  int last_key;
  struct timeval last_time;
  int suppress_next;
} debounce_state = {-1, {0, 0}, 0};
#endif

/* Key hold tracking state */
static struct {
  char held_key; /* Currently held key character, '-' for none */
  int held_code; /* Raw keycode of held key, -1 for none */
} hold_state = {'-', -1};

#if KEYPAD_PHONE_LAYOUT
/*
 * Phone-mode 0/00 key disambiguation state.
 *
 * The USB keypad's '0' and '00' keys both send KEY_KP0. The '00' key
 * sends two KEY_KP0 events within ~5ms. We defer reporting the first
 * KEY_KP0 and wait to see if a second arrives quickly. If so, it's '00'.
 *
 * In phone mode: single '0' key → '*', double '00' key → '0'
 */
#define KP0_DISAMBIG_WINDOW_US                                                 \
  30000 /* 30ms window to detect 00 key (measured: 16-24ms) */

static struct {
  int pending;              /* 1 if waiting to determine 0 vs 00 */
  struct timeval wall_time; /* Wall-clock time when pending was set */
  int raw_code;             /* Saved raw keycode */
} kp0_defer = {0, {0, 0}, 0};

/* Stash for events read during 0/00 disambiguation */
static struct input_event stashed_ev;
static int has_stashed_ev = 0;
#endif

/* Keycode to HAMPOD symbol mapping table */
static const struct {
  int keycode;
  char symbol;
} keymap[] = {
#if KEYPAD_PHONE_LAYOUT
    /*
     * Phone-style positional mapping for 19-key USB calculator keypad.
     * Maps positions to match the original 16-key HAMPOD layout:
     *
     * USB Physical:              HAMPOD Phone Symbol:
     * ┌────┬────┬────┬────┐     ┌────┬────┬────┬────┐
     * │NumL│ /  │ *  │ BS │     │ -- │ -- │ -- │ A  │
     * ├────┼────┼────┼────┤     ├────┼────┼────┼────┤
     * │ 7  │ 8  │ 9  │ -  │     │ 1  │ 2  │ 3  │ B  │
     * ├────┼────┼────┼────┤     ├────┼────┼────┼────┤
     * │ 4  │ 5  │ 6  │ +  │     │ 4  │ 5  │ 6  │ C  │
     * ├────┼────┼────┤    │     ├────┼────┼────┼────┤
     * │ 1  │ 2  │ 3  │Ent │     │ 7  │ 8  │ 9  │ D  │
     * ├────┼────┼────┤    │     ├────┼────┼────┼────┤
     * │ 0  │ 00 │ .  │    │     │ *  │ 0  │ #  │ D  │
     * └────┴────┴────┴────┘     └────┴────┴────┴────┘
     *
     * Note: KEY_KP0 is NOT in this table; it is handled specially
     * by the 0/00 disambiguation logic (single→'*', double→'0').
     */

    /* Numeric keys - swapped rows for phone-style positioning */
    {KEY_KP1, '7'},
    {KEY_KP2, '8'},
    {KEY_KP3, '9'},
    {KEY_KP4, '4'},
    {KEY_KP5, '5'},
    {KEY_KP6, '6'},
    {KEY_KP7, '1'},
    {KEY_KP8, '2'},
    {KEY_KP9, '3'},

    /* Right column: A, B, C, D from top to bottom */
    {KEY_BACKSPACE, 'A'}, /* Top-right → A */
    {KEY_KPMINUS, 'B'},   /* - → B */
    {KEY_KPPLUS, 'C'},    /* + → C */
    {KEY_KPENTER, 'D'},   /* Enter → D */

    /* Bottom row (non-KP0 keys) */
    {KEY_KPDOT, '#'}, /* . (DEL) → # */

/* Top row keys - unmapped in phone mode (NumLock, /, *) */
/* These will fall through to '-' (unmapped) in map_keycode_to_symbol */

#else
    /* Calculator-style: 7-8-9 on top, 1-2-3 on bottom (default) */
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
    {KEY_KPSLASH, 'A'},    /* / → A */
    {KEY_KPASTERISK, 'B'}, /* * → B */
    {KEY_KPMINUS, 'C'},    /* - → C */
    {KEY_KPPLUS, 'D'},     /* + → D */

    /* Special keys */
    {KEY_KPENTER, '#'},   /* ENTER → # */
    {KEY_KPDOT, '*'},     /* . (DEL) → * (alternative mapping) */
    {KEY_NUMLOCK, 'X'},   /* NUM_LOCK → X */
    {KEY_BACKSPACE, 'Y'}, /* BACKSPACE → Y */
#endif

    /* End of table marker */
    {-1, '\0'}};

/**
 * @brief Find USB keypad device
 *
 * Searches for USB keypad in /dev/input
 *
 * @param device_path Buffer to store found device path
 * @param max_len Maximum length of buffer
 * @return 0 on success, -1 on failure
 */
static int find_usb_keypad(char *device_path, size_t max_len) {
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
  return '-'; /* Invalid/unmapped key */
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
  KeypadEvent event = {'-', 0, 0}; /* Default: invalid event */
  struct input_event ev;
  ssize_t bytes_read;

  /* Check if device is initialized */
  if (keypad_fd < 0) {
    return event;
  }

#if KEYPAD_PHONE_LAYOUT
  /*
   * Phone-mode: check for deferred 0/00 disambiguation timeout.
   * If a KEY_KP0 is pending and enough wall-clock time has elapsed
   * without a second KEY_KP0, it was a single '0' press (maps to '*').
   */
  if (kp0_defer.pending) {
    /*
     * Read events from buffer, skipping releases and non-key events,
     * looking for the second KEY_KP0 press that indicates '00' key.
     * The '00' key sends: press, release, press, release — all within ~24ms.
     * We need to skip the intervening release to find the second press.
     */
    if (!has_stashed_ev) {
      while (1) {
        bytes_read = read(keypad_fd, &ev, sizeof(ev));
        if (bytes_read != sizeof(ev))
          break; /* Buffer empty */

        if (ev.type != EV_KEY)
          continue; /* Skip non-key events (SYN, etc) */

        if (ev.code == KEY_KP0 && ev.value == 1) {
          /* Found second KEY_KP0 press — this is the '00' key */
          kp0_defer.pending = 0;
          event.key = '0'; /* '00' maps to '0' in phone mode */
          event.raw_code = ev.code;
          event.valid = 1;
          hold_state.held_key = event.key;
          hold_state.held_code = ev.code;
          return event;
        }

        if (ev.value == 1) {
          /* A different key was pressed — stash it, resolve pending as '*' */
          stashed_ev = ev;
          has_stashed_ev = 1;
          break;
        }

        /* Release (value=0) or repeat (value=2): skip and keep draining */
      }
    }

    /* Check wall-clock timeout */
    struct timeval now;
    gettimeofday(&now, NULL);
    long elapsed_us = (now.tv_sec - kp0_defer.wall_time.tv_sec) * 1000000L +
                      (now.tv_usec - kp0_defer.wall_time.tv_usec);

    if (elapsed_us >= KP0_DISAMBIG_WINDOW_US || has_stashed_ev) {
      /* Timeout or different key arrived: single '0' press → '*' */
      kp0_defer.pending = 0;
      event.key = '*';
      event.raw_code = kp0_defer.raw_code;
      event.valid = 1;
      hold_state.held_key = event.key;
      hold_state.held_code = event.raw_code;
      return event;
    }

    /* Still within window, no second KEY_KP0 yet — keep waiting */
    return event;
  }

  /* Process stashed event from disambiguation, if any */
  if (has_stashed_ev) {
    ev = stashed_ev;
    has_stashed_ev = 0;
    bytes_read = sizeof(ev);
  } else {
    bytes_read = read(keypad_fd, &ev, sizeof(ev));
  }
#else
  /* Calculator mode: simple read */
  bytes_read = read(keypad_fd, &ev, sizeof(ev));
#endif

  if (bytes_read == sizeof(ev) && ev.type == EV_KEY) {

    if (ev.value == 1) { /* Key press down */

#if KEYPAD_PHONE_LAYOUT
      /* Phone-mode: defer KEY_KP0 for 0/00 disambiguation */
      if (ev.code == KEY_KP0) {
        kp0_defer.pending = 1;
        gettimeofday(&kp0_defer.wall_time, NULL);
        kp0_defer.raw_code = ev.code;
        /* Don't report yet - return invalid event */
        return event;
      }
#else
      /* Calculator-mode: handle '00' key debouncing */
      /* The '00' key on many keypads sends two KEY_KP0 events rapidly */
      /* We suppress the second one if it comes within 50ms of the first */
      if (ev.code == KEY_KP0 && debounce_state.last_key == KEY_KP0) {
        /* Calculate time difference in microseconds */
        long time_diff =
            (ev.time.tv_sec - debounce_state.last_time.tv_sec) * 1000000L +
            (ev.time.tv_usec - debounce_state.last_time.tv_usec);

        /* If within 50ms (50000 microseconds), suppress this event */
        if (time_diff < 50000) {
          /* Return invalid event to suppress this duplicate '0' */
          return event;
        }
      }

      /* Store this event for next debounce check */
      debounce_state.last_key = ev.code;
      debounce_state.last_time = ev.time;
#endif

      /* Map the keycode to symbol */
      char key_char = map_keycode_to_symbol(ev.code);

      /* Update hold state */
      hold_state.held_key = key_char;
      hold_state.held_code = ev.code;

      /* Return the press event */
      event.raw_code = ev.code;
      event.key = key_char;
      event.valid = (key_char != '-') ? 1 : 0;

    } else if (ev.value == 0) { /* Key release */

      /* Only clear hold state if this release matches the held key */
      if (ev.code == hold_state.held_code) {
        hold_state.held_key = '-';
        hold_state.held_code = -1;
      }
      /* Don't report release events as key presses */

    } else if (ev.value == 2) { /* Key repeat (held) */

      /* Report the held key again */
      if (hold_state.held_key != '-') {
        event.raw_code = hold_state.held_code;
        event.key = hold_state.held_key;
        event.valid = 1;
      }
    }
  }
  /* If no event available (bytes_read <= 0), just return invalid event */
  /* Linux input system provides ev.value == 2 (repeat) for held keys */

  return event;
}

void hal_keypad_cleanup(void) {
  if (keypad_fd >= 0) {
    close(keypad_fd);
    keypad_fd = -1;
    printf("HAL Keypad: Cleaned up\n");
  }
}

const char *hal_keypad_get_impl_name(void) { return "USB Numeric Keypad"; }
