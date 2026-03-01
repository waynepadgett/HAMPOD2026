/**
 * @file config_mode.c
 * @brief Configuration Mode implementation for HAMPOD2026
 *
 * Part of Config Mode feature implementation.
 */

#include "config_mode.h"
#include "comm.h"
#include "config.h"
#include "hampod_core.h"
#include "speech.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ============================================================================
// Module State
// ============================================================================

static ConfigModeState g_state = CONFIG_MODE_OFF;
static ConfigModeParameter g_current_param = CONFIG_PARAM_VOLUME;

// We use config.c's undo stack, so we need to track how deep to undo if
// cancelled.
static int g_undo_depth_on_entry = 0;

// ============================================================================
// Forward declarations
// ============================================================================

static void announce_param_value(ConfigModeParameter param);
static void apply_volume_live(int vol);

// ============================================================================
// Initialization
// ============================================================================

void config_mode_init(void) {
  g_state = CONFIG_MODE_OFF;
  g_current_param = CONFIG_PARAM_VOLUME;
  DEBUG_PRINT("config_mode_init: Initialized\n");
}

// ============================================================================
// State Queries
// ============================================================================

bool config_mode_is_active(void) { return g_state != CONFIG_MODE_OFF; }

ConfigModeState config_mode_get_state(void) { return g_state; }

ConfigModeParameter config_mode_get_parameter(void) { return g_current_param; }

// ============================================================================
// Mode Entry/Exit
// ============================================================================

void config_mode_enter(void) {
  if (g_state == CONFIG_MODE_OFF) {
    g_state = CONFIG_MODE_BROWSING;
    g_current_param = CONFIG_PARAM_VOLUME;
    g_undo_depth_on_entry = config_get_undo_count();
    speech_say_text("Configuration Mode");
    announce_param_value(g_current_param);
    DEBUG_PRINT("config_mode_enter: Entered Configuration Mode\n");
  }
}

void config_mode_exit_save(void) {
  if (g_state != CONFIG_MODE_OFF) {
    g_state = CONFIG_MODE_OFF;
    speech_say_text("Configuration saved");
    config_save(); // Ensure any pending config changes are flused
    DEBUG_PRINT("config_mode_exit_save: Exited Configuration Mode (Saved)\n");
  }
}

void config_mode_exit_discard(void) {
  if (g_state != CONFIG_MODE_OFF) {
    g_state = CONFIG_MODE_OFF;

    // Revert all changes made during this session
    int current_undos = config_get_undo_count();
    while (current_undos > g_undo_depth_on_entry) {
      config_undo();
      current_undos = config_get_undo_count();
    }

    // Must manually re-apply the live settings to audio/piper
    apply_volume_live(config_get_volume());
    comm_set_speech_speed(config_get_speech_speed());

    speech_say_text("Configuration cancelled");
    DEBUG_PRINT(
        "config_mode_exit_discard: Exited Configuration Mode (Discarded)\n");
  }
}

// ============================================================================
// Live System Application Helpers
// ============================================================================

static void apply_volume_live(int vol) {
  int card = config_get_audio_card_number();
  char vol_cmd[256];
  if (card >= 0) {
    snprintf(vol_cmd, sizeof(vol_cmd),
             "amixer -c %d -q sset PCM %d%% 2>/dev/null", card, vol);
  } else {
    snprintf(vol_cmd, sizeof(vol_cmd), "amixer -q sset PCM %d%% 2>/dev/null",
             vol);
  }
  system(vol_cmd);
}

// ============================================================================
// Nav and Value Handling
// ============================================================================

static void announce_param_value(ConfigModeParameter param) {
  char buffer[64];

  switch (param) {
  case CONFIG_PARAM_VOLUME:
    snprintf(buffer, sizeof(buffer), "Volume, %d percent", config_get_volume());
    break;

  case CONFIG_PARAM_SPEECH_SPEED: {
    float speed = config_get_speech_speed();
    int speed_whole = (int)speed;
    int speed_frac = (int)((speed - speed_whole) * 10 + 0.5);
    snprintf(buffer, sizeof(buffer), "Speed, %d point %d", speed_whole,
             speed_frac);
  } break;

  case CONFIG_PARAM_LAYOUT: {
    const char *layout = config_get_keypad_layout();
    snprintf(buffer, sizeof(buffer), "Keypad Layout, %s", layout);
  } break;

  case CONFIG_PARAM_SHUTDOWN:
    snprintf(buffer, sizeof(buffer), "System Shutdown, press Enter to confirm");
    break;

  default:
    snprintf(buffer, sizeof(buffer), "Unknown parameter");
    break;
  }

  speech_say_text(buffer);
}

static void change_param_value(ConfigModeParameter param, bool increment) {
  switch (param) {
  case CONFIG_PARAM_VOLUME: {
    int vol = config_get_volume();
    if (increment) {
      vol += 10;
      if (vol > 100)
        vol = 100;
    } else {
      vol -= 10;
      if (vol < 10)
        vol = 10;
    }
    config_set_volume(vol);
    apply_volume_live(vol);
    announce_param_value(param);
  } break;

  case CONFIG_PARAM_SPEECH_SPEED: {
    float speed = config_get_speech_speed();
    if (increment) {
      speed += 0.1f;
      if (speed > 2.0f)
        speed = 2.0f;
    } else {
      speed -= 0.1f;
      if (speed < 0.1f)
        speed = 0.1f;
    }
    config_set_speech_speed(speed);
    comm_set_speech_speed(speed);
    announce_param_value(param);
  } break;

  case CONFIG_PARAM_LAYOUT: {
    const char *cur = config_get_keypad_layout();
    const char *next_layout =
        (strcmp(cur, "calculator") == 0) ? "phone" : "calculator";
    config_set_keypad_layout(next_layout);
    announce_param_value(param);
  } break;

  case CONFIG_PARAM_SHUTDOWN:
    // Non-numeric. Explain the exit requirement
    speech_say_text("Press Enter to shut down, or navigate away");
    break;

  default:
    if (config_get_key_beep_enabled())
      comm_play_beep(COMM_BEEP_ERROR);
    break;
  }
}

// ============================================================================
// Key Handling
// ============================================================================

bool config_mode_handle_key(char key, bool is_hold) {
  if (g_state == CONFIG_MODE_OFF) {
    return false;
  }

  DEBUG_PRINT("config_mode_handle_key: key='%c' hold=%d state=%d\n", key,
              is_hold, g_state);

  // [C] Hold - Save and Exit
  if (key == 'C' && is_hold) {
    config_mode_exit_save();
    return true;
  }

  // [B] Hold - Discard and Exit
  if (key == 'B' && is_hold) {
    config_mode_exit_discard();
    return true;
  }

  // Prevent hold actions other than B and C from doing anything
  if (is_hold) {
    if (config_get_key_beep_enabled())
      comm_play_beep(COMM_BEEP_ERROR);
    return true;
  }

  // Step forward [A] / backward [B] through parameters
  if (key == 'A') {
    int next_param = (int)g_current_param + 1;
    if (next_param >= CONFIG_PARAM_COUNT)
      next_param = 0;
    g_current_param = (ConfigModeParameter)next_param;
    announce_param_value(g_current_param);
    return true;
  } else if (key == 'B') {
    int prev_param = (int)g_current_param - 1;
    if (prev_param < 0)
      prev_param = CONFIG_PARAM_COUNT - 1;
    g_current_param = (ConfigModeParameter)prev_param;
    announce_param_value(g_current_param);
    return true;
  }

  // Increment [C] / Decrement [D] values
  if (key == 'C') {
    change_param_value(g_current_param, true);
    return true;
  } else if (key == 'D') {
    change_param_value(g_current_param, false);
    return true;
  }

  // Shutdown execution
  if (key == '#' && g_current_param == CONFIG_PARAM_SHUTDOWN) {
    speech_say_text("Shutting down");
    system("sudo shutdown -h now");
    return true;
  }

  // Other keys
  if (config_get_key_beep_enabled()) {
    comm_play_beep(COMM_BEEP_ERROR);
  }

  return true; // We consume all keys in config mode
}
