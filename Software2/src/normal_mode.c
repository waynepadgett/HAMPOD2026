/**
 * @file normal_mode.c
 * @brief Normal Mode implementation
 *
 * Part of Phase 2: Normal Mode Implementation
 */

#include "normal_mode.h"
#include "frequency_mode.h"
#include "hampod_core.h"
#include "radio.h"
#include "radio_queries.h"
#include "radio_setters.h"
#include "speech.h"

#include <stdio.h>
#include <string.h>

// ============================================================================
// Module State
// ============================================================================

static bool g_verbosity_enabled = true; // Auto-announcements on by default

// ============================================================================
// Internal Helpers
// ============================================================================

/**
 * @brief Announce the current frequency as speech
 */
static void announce_frequency(void) {
  double freq_hz = radio_get_frequency();
  if (freq_hz < 0) {
    speech_say_text("Frequency not available");
    return;
  }

  // Convert Hz to MHz and format
  double freq_mhz = freq_hz / 1000000.0;
  int mhz_part = (int)freq_mhz;

  // Get 5 decimal places for 10 Hz resolution
  // Use truncation (not rounding) to match radio display behavior
  // Add tiny epsilon to handle floating-point representation errors
  int decimals = (int)((freq_mhz - mhz_part) * 100000 + 0.0001);

  char text[128];
  if (decimals == 0) {
    snprintf(text, sizeof(text), "%d megahertz", mhz_part);
  } else {
    // Spell out each decimal digit for clarity
    char decimal_str[16];
    snprintf(decimal_str, sizeof(decimal_str), "%05d", decimals);

    char spoken_decimals[32] = "";
    for (int i = 0; i < 5; i++) {
      char digit[4];
      snprintf(digit, sizeof(digit), "%c ", decimal_str[i]);
      strcat(spoken_decimals, digit);
    }

    snprintf(text, sizeof(text), "%d point %s megahertz", mhz_part,
             spoken_decimals);
  }

  speech_say_text(text);
}

/**
 * @brief Announce S-meter reading
 */
static void announce_smeter(void) {
  char buffer[32];
  const char *reading = radio_get_smeter_string(buffer, sizeof(buffer));
  speech_say_text(reading);
}

/**
 * @brief Announce power meter reading
 */
static void announce_power_meter(void) {
  char buffer[32];
  const char *reading = radio_get_power_string(buffer, sizeof(buffer));
  speech_say_text(reading);
}

// ============================================================================
// Initialization
// ============================================================================

void normal_mode_init(void) {
  g_verbosity_enabled = true;
  DEBUG_PRINT("normal_mode_init: Initialized\n");
}

// ============================================================================
// Key Handling
// ============================================================================

bool normal_mode_handle_key(char key, bool is_hold, bool is_shifted) {
  DEBUG_PRINT("normal_mode_handle_key: key='%c' hold=%d shift=%d\n", key,
              is_hold, is_shifted);

  // [1] - VFO selection / [Shift+1] - VOX status query
  if (key == '1') {
    if (is_shifted && !is_hold) {
      // [Shift]+[1] - VOX status query
      int vox = radio_get_vox_status();
      if (vox < 0) {
        speech_say_text("VOX status unavailable");
      } else if (vox == 1) {
        speech_say_text("VOX is on");
      } else {
        speech_say_text("VOX is off");
      }
      return true;
    } else if (!is_hold) {
      // Select VFO A
      // Suppress polling announcement since we'll announce ourselves
      frequency_mode_suppress_next_poll();
      if (radio_set_vfo(RADIO_VFO_A) == 0) {
        speech_say_text("VFO A");
        announce_frequency();
      } else {
        speech_say_text("VFO A not available");
      }
    } else {
      // Select VFO B
      frequency_mode_suppress_next_poll();
      if (radio_set_vfo(RADIO_VFO_B) == 0) {
        speech_say_text("VFO B");
        announce_frequency();
      } else {
        speech_say_text("VFO B not available");
      }
    }
    return true;
  }

  // [2] - Announce current frequency
  if (key == '2' && !is_hold) {
    announce_frequency();
    return true;
  }

  // [0] - Announce current mode
  if (key == '0' && !is_hold) {
    const char *mode = radio_get_mode_string();
    speech_say_text(mode);
    return true;
  }

  // [4] - PreAmp (press) / AGC (hold) / Attenuation (shift+press)
  if (key == '4') {
    char buffer[64];
    if (is_shifted && !is_hold) {
      // [Shift]+[4] - Attenuation query
      int atten = radio_get_attenuation();
      if (atten == 0) {
        snprintf(buffer, sizeof(buffer), "Attenuation off");
      } else if (atten > 0) {
        snprintf(buffer, sizeof(buffer), "Attenuation %d D B", atten);
      } else {
        snprintf(buffer, sizeof(buffer), "Attenuation not available");
      }
      speech_say_text(buffer);
      return true;
    } else if (is_hold) {
      // [4] Hold - AGC query
      snprintf(buffer, sizeof(buffer), "A G C %s", radio_get_agc_string());
      speech_say_text(buffer);
      return true;
    } else {
      // [4] Press - PreAmp query
      int preamp = radio_get_preamp();
      if (preamp == 0) {
        snprintf(buffer, sizeof(buffer), "Pre amp off");
      } else if (preamp > 0) {
        snprintf(buffer, sizeof(buffer), "Pre amp %d", preamp);
      } else {
        snprintf(buffer, sizeof(buffer), "Pre amp not available");
      }
      speech_say_text(buffer);
      return true;
    }
  }

  // [7] - Noise Blanker query
  if (key == '7' && !is_hold) {
    char buffer[64];
    bool nb_on = radio_get_nb_enabled();
    int nb_level = radio_get_nb_level();
    snprintf(buffer, sizeof(buffer), "Noise blanker %s, level %d",
             nb_on ? "on" : "off", nb_level >= 0 ? nb_level : 0);
    speech_say_text(buffer);
    return true;
  }

  // [8] - Noise Reduction (press) / Mic Gain (hold)
  if (key == '8') {
    char buffer[64];
    if (is_hold) {
      // [8] Hold - Mic Gain query
      int mic = radio_get_mic_gain();
      if (mic >= 0) {
        snprintf(buffer, sizeof(buffer), "Mic gain %d percent", mic);
      } else {
        snprintf(buffer, sizeof(buffer), "Mic gain not available");
      }
      speech_say_text(buffer);
      return true;
    } else {
      // [8] Press - Noise Reduction query
      bool nr_on = radio_get_nr_enabled();
      int nr_level = radio_get_nr_level();
      snprintf(buffer, sizeof(buffer), "Noise reduction %s, level %d",
               nr_on ? "on" : "off", nr_level >= 0 ? nr_level : 0);
      speech_say_text(buffer);
      return true;
    }
  }

  // [9] - Compression (shift+press) / Power (hold)
  if (key == '9') {
    char buffer[64];
    if (is_shifted && !is_hold) {
      // [Shift]+[9] - Compression query
      int comp = radio_get_compression();
      bool comp_on = radio_get_compression_enabled();
      if (comp >= 0) {
        snprintf(buffer, sizeof(buffer), "Compression %s, level %d",
                 comp_on ? "on" : "off", comp);
      } else {
        snprintf(buffer, sizeof(buffer), "Compression not available");
      }
      speech_say_text(buffer);
      return true;
    } else if (is_hold) {
      // [9] Hold - Power level query
      int power = radio_get_power();
      if (power >= 0) {
        snprintf(buffer, sizeof(buffer), "Power %d percent", power);
      } else {
        snprintf(buffer, sizeof(buffer), "Power not available");
      }
      speech_say_text(buffer);
      return true;
    }
    // [9] Press alone - not assigned, fall through
  }

  // [*] - S-meter (press) / Power meter (hold)
  if (key == '*') {
    if (!is_hold) {
      announce_smeter();
    } else {
      announce_power_meter();
    }
    return true;
  }

  // [C] - Toggle verbosity (press) / Config mode entry (hold, not implemented)
  if (key == 'C' && !is_hold) {
    g_verbosity_enabled = !g_verbosity_enabled;
    if (g_verbosity_enabled) {
      speech_say_text("Announcements on");
    } else {
      speech_say_text("Announcements off");
    }
    return true;
  }

  // Key not handled by normal mode
  return false;
}

// ============================================================================
// Verbosity Control
// ============================================================================

void normal_mode_set_verbosity(bool enabled) {
  g_verbosity_enabled = enabled;
  DEBUG_PRINT("normal_mode_set_verbosity: %s\n", enabled ? "on" : "off");
}

bool normal_mode_get_verbosity(void) { return g_verbosity_enabled; }

// ============================================================================
// Radio Change Callbacks
// ============================================================================

void normal_mode_on_mode_change(const char *new_mode) {
  if (!g_verbosity_enabled) {
    return;
  }

  // Don't announce if frequency mode is active
  if (frequency_mode_is_active()) {
    return;
  }

  DEBUG_PRINT("normal_mode_on_mode_change: %s\n", new_mode);
  speech_say_text(new_mode);
}

void normal_mode_on_vfo_change(int new_vfo) {
  if (!g_verbosity_enabled) {
    return;
  }

  // Don't announce if frequency mode is active
  if (frequency_mode_is_active()) {
    return;
  }

  DEBUG_PRINT("normal_mode_on_vfo_change: %d\n", new_vfo);
  const char *vfo_name = radio_get_vfo_string();
  speech_say_text(vfo_name);
}
