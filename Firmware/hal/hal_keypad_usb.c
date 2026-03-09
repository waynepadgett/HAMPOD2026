/**
 * @file hal_keypad_usb.c
 * @brief USB Keypad implementation of the keypad HAL
 *
 * This implementation reads keypad events from a USB numeric keypad
 * using the Linux input event system (/dev/input/eventX).
 *
 * Supports two layout modes selectable at runtime:
 *   - Calculator (default): 7-8-9 on top row, matching key labels
 *   - Phone: 1-2-3 on top row, positional mapping to original HAMPOD layout
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

/* 0/00 disambiguation window (measured: 16-24ms between events) */
#define KP0_DISAMBIG_WINDOW_US 30000

/* Runtime layout mode: 0 = calculator (default), 1 = phone */
static int g_phone_layout = 0;

/* File descriptors for all matching keypad devices */
#define MAX_KEYPADS 16
static int keypad_fds[MAX_KEYPADS];
static int num_keypads = 0;

/* Debouncing state for '00' key (calculator mode) */
static struct {
  int last_key;
  struct timeval last_time;
  int suppress_next;
} debounce_state = {-1, {0, 0}, 0};

/* Key hold tracking state */
static struct {
  char held_key; /* Currently held key character, '-' for none */
  int held_code; /* Raw keycode of held key, -1 for none */
} hold_state = {'-', -1};

/*
 * Phone-mode 0/00 key disambiguation state.
 *
 * The USB keypad's '0' and '00' keys both send KEY_KP0. The '00' key
 * sends two KEY_KP0 events within ~16-24ms. We defer reporting the first
 * KEY_KP0 and wait to see if a second arrives quickly. If so, it's '00'.
 *
 * In phone mode: single '0' key → '*', double '00' key → '0'
 */
static struct {
  int pending;              /* 1 if waiting to determine 0 vs 00 */
  struct timeval wall_time; /* Wall-clock time when pending was set */
  int raw_code;             /* Saved raw keycode */
  int source_fd_idx;        /* FD index of the pending keypress */
} kp0_defer = {0, {0, 0}, 0, -1};

/* Stash for events read during 0/00 disambiguation */
static struct input_event stashed_ev;
static int has_stashed_ev = 0;

/* Keycode-to-symbol mapping entry */
typedef struct {
  int keycode;
  char symbol;
} KeymapEntry;

/*
 * Calculator-style keymap: 7-8-9 on top, matches USB key labels.
 */
static const KeymapEntry keymap_calculator[] = {
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
    {KEY_KPDOT, '*'},     /* . (DEL) → * */
    {KEY_NUMLOCK, 'X'},   /* NUM_LOCK → X */
    {KEY_BACKSPACE, 'Y'}, /* BACKSPACE → Y */

    /* Standard QWERTY ALPHANUMERIC Fallbacks */
    {KEY_0, '0'},
    {KEY_1, '1'},
    {KEY_2, '2'},
    {KEY_3, '3'},
    {KEY_4, '4'},
    {KEY_5, '5'},
    {KEY_6, '6'},
    {KEY_7, '7'},
    {KEY_8, '8'},
    {KEY_9, '9'},
    {KEY_A, 'A'},
    {KEY_B, 'B'},
    {KEY_C, 'C'},
    {KEY_D, 'D'},
    {KEY_ENTER, '#'},
    {KEY_DOT, '*'},
    {KEY_X, 'X'},
    {KEY_Y, 'Y'},

    {-1, '\0'} /* End marker */
};

/*
 * Phone-style keymap: positional mapping for 19-key USB calculator keypad.
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
 * NOTE: KEY_KP0 is NOT in this table — it is handled by the
 * 0/00 disambiguation logic (single → '*', double → '0').
 */
static const KeymapEntry keymap_phone[] = {
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

    /* Top row keys (NumLock, /, *) unmapped — fall through to '-' */

    /* Standard QWERTY ALPHANUMERIC Fallbacks */
    {KEY_0, '0'},
    {KEY_1, '1'},
    {KEY_2, '2'},
    {KEY_3, '3'},
    {KEY_4, '4'},
    {KEY_5, '5'},
    {KEY_6, '6'},
    {KEY_7, '7'},
    {KEY_8, '8'},
    {KEY_9, '9'},
    {KEY_A, 'A'},
    {KEY_B, 'B'},
    {KEY_C, 'C'},
    {KEY_D, 'D'},
    {KEY_ENTER, '#'},
    {KEY_DOT, '*'},
    {KEY_X, 'X'},
    {KEY_Y, 'Y'},

    {-1, '\0'} /* End marker */
};

/**
 * @brief Find all USB keypad devices
 */
static int find_usb_keypads(char paths[MAX_KEYPADS][256]) {
  glob_t glob_result;
  int count = 0;

  if (glob(KEYPAD_DEVICE_PATTERN, 0, NULL, &glob_result) == 0) {
    for (size_t i = 0; i < glob_result.gl_pathc && count < MAX_KEYPADS; i++) {
      strncpy(paths[count], glob_result.gl_pathv[i], 255);
      paths[count][255] = '\0';
      count++;
    }
    globfree(&glob_result);
  }

  return count;
}

/**
 * @brief Map keycode to HAMPOD symbol using the active keymap
 */
static char map_keycode_to_symbol(int keycode) {
  const KeymapEntry *map = g_phone_layout ? keymap_phone : keymap_calculator;

  for (int i = 0; map[i].keycode != -1; i++) {
    if (map[i].keycode == keycode) {
      return map[i].symbol;
    }
  }
  return '-'; /* Invalid/unmapped key */
}

/* HAL Implementation Functions */

int hal_keypad_init(void) {
  char device_paths[MAX_KEYPADS][256];
  num_keypads = find_usb_keypads(device_paths);

  if (num_keypads == 0) {
    fprintf(stderr, "HAL Keypad: No USB keypad devices found\n");
    return -1;
  }

  int success = 0;
  for (int i = 0; i < num_keypads; i++) {
    keypad_fds[i] = open(device_paths[i], O_RDONLY | O_NONBLOCK);
    if (keypad_fds[i] >= 0) {
      printf("HAL Keypad: Opened USB keypad %d at %s\n", i, device_paths[i]);
      success++;
    } else {
      perror("HAL Keypad: Failed to open device");
    }
  }

  if (success == 0) {
    return -1;
  }

  // Condense the fd array
  int j = 0;
  for (int i = 0; i < num_keypads; i++) {
    if (keypad_fds[i] >= 0) {
      keypad_fds[j++] = keypad_fds[i];
    }
  }
  num_keypads = j;

  printf("HAL Keypad: Initialized %d USB keypads (layout: %s)\n", num_keypads,
         g_phone_layout ? "phone" : "calculator");
  return 0;
}

void hal_keypad_set_phone_layout(int phone_layout) {
  g_phone_layout = phone_layout ? 1 : 0;
  printf("HAL Keypad: Layout set to %s\n",
         g_phone_layout ? "phone" : "calculator");
}

KeypadEvent hal_keypad_read(void) {
  KeypadEvent event = {'-', 0, 0}; /* Default: invalid event */
  struct input_event ev;
  ssize_t bytes_read = -1;
  int current_fd_idx = -1;

  if (num_keypads == 0) {
    return event;
  }

  /* ── Phone-mode: 0/00 disambiguation ────────────────────────────── */
  if (g_phone_layout && kp0_defer.pending) {
    /*
     * Read events from buffer, skipping releases and non-key events,
     * looking for the second KEY_KP0 press that indicates '00' key.
     * The '00' key sends: press, release, press, release — all within ~24ms.
     */
    if (!has_stashed_ev) {
      while (1) {
        bytes_read = read(keypad_fds[kp0_defer.source_fd_idx], &ev, sizeof(ev));
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

  /* ── Read next event ────────────────────────────────────────────── */

  /* Process stashed event from disambiguation, if any */
  if (has_stashed_ev) {
    ev = stashed_ev;
    has_stashed_ev = 0;
    bytes_read = sizeof(ev);
    current_fd_idx = kp0_defer.source_fd_idx; /* Reset original context */
  } else {
    for (int i = 0; i < num_keypads; i++) {
      bytes_read = read(keypad_fds[i], &ev, sizeof(ev));
      if (bytes_read == sizeof(ev)) {
        current_fd_idx = i;
        break;
      }
    }
  }

  if (bytes_read == sizeof(ev) && ev.type == EV_KEY) {

    if (ev.value == 1) { /* Key press down */

      /* Phone-mode: defer KEY_KP0 for 0/00 disambiguation */
      if (g_phone_layout && ev.code == KEY_KP0) {
        kp0_defer.pending = 1;
        gettimeofday(&kp0_defer.wall_time, NULL);
        kp0_defer.raw_code = ev.code;
        kp0_defer.source_fd_idx = current_fd_idx; /* Store where it came from */
        return event;                             /* Don't report yet */
      }

      /* Calculator-mode: debounce '00' key */
      if (!g_phone_layout && ev.code == KEY_KP0 &&
          debounce_state.last_key == KEY_KP0) {
        long time_diff =
            (ev.time.tv_sec - debounce_state.last_time.tv_sec) * 1000000L +
            (ev.time.tv_usec - debounce_state.last_time.tv_usec);

        if (time_diff < 50000) {
          return event; /* Suppress duplicate '0' */
        }
      }

      /* Store for next debounce check (calculator mode) */
      if (!g_phone_layout) {
        debounce_state.last_key = ev.code;
        debounce_state.last_time = ev.time;
      }

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

      if (ev.code == hold_state.held_code) {
        hold_state.held_key = '-';
        hold_state.held_code = -1;
      }

    } else if (ev.value == 2) { /* Key repeat (held) */

      if (hold_state.held_key != '-') {
        event.raw_code = hold_state.held_code;
        event.key = hold_state.held_key;
        event.valid = 1;
      }
    }
  }

  return event;
}

void hal_keypad_cleanup(void) {
  for (int i = 0; i < num_keypads; i++) {
    if (keypad_fds[i] >= 0) {
      close(keypad_fds[i]);
      keypad_fds[i] = -1;
    }
  }
  num_keypads = 0;
  printf("HAL Keypad: Cleaned up\n");
}

const char *hal_keypad_get_impl_name(void) { return "USB Numeric Keypad"; }
