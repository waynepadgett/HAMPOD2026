/**
 * @file normal_mode.c
 * @brief Normal Mode implementation
 *
 * Part of Phase 2: Normal Mode Implementation
 */

#include "normal_mode.h"
#include "config_mode.h"
#include "frequency_mode.h"
#include "hampod_core.h"
#include "radio.h"
#include "radio_queries.h"
#include "radio_setters.h"
#include "speech.h"

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

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

bool normal_mode_handle_key(char key, bool is_hold, bool is_shifted, bool in_set_mode) {
    DEBUG_PRINT("normal_mode_handle_key: key='%c' hold=%d shift=%d\n", key, is_hold, is_shifted);
    // [0] - Announce current mode
    if (key == '0' && !in_set_mode) {
        if(!is_hold && !is_shifted){
        const char* mode = radio_get_mode_string();
        speech_say_text(mode);
        return true;
        }
        else if(is_shifted && !is_hold){
            float swr = radio_get_swr();
            char buffer[64];

            if (swr > 0.0f) {
                snprintf(buffer, sizeof(buffer), "S W R %.1f", swr);
            } else {
                snprintf(buffer, sizeof(buffer), "S W R not available");
            }

            speech_say_text(buffer);
            return true;
        }
        else if(!is_shifted && is_hold){
            if (radio_toggle_data_mode() == 0) {
                const char* mode = radio_get_mode_string();
                speech_say_text(mode);
            } else {
                speech_say_text("Failed");
            }
        }
    }
    // [1] - VFO selection
    if (key == '1'&& !in_set_mode) { // added not in set mode condition, only execute this when not in set mode
        if(is_shifted && !is_hold){
        int vox = radio_get_vox_status();

        if (vox < 0) {
            speech_say_text("VOX status unavailable, shift one pressed");
            } else if (vox == 1) {
            speech_say_text("VOX is now on");
            } else {
            speech_say_text("VOX is off");
            }
        }
        else if(is_shifted && is_hold ){ 
        // shift + hold → break-in status
            int break_in = radio_get_break_in_status();

            if (break_in < 0) {
                speech_say_text("Break in status unavailable");
            } 
            else if (break_in == 2) {
                speech_say_text("Full break in is on");
            } 
            else if (break_in == 1) {
                speech_say_text("Semi break in is on");
            } 
            else {
                speech_say_text("Break in is off");
            }
        }

        else if (!is_hold) {
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
    if (key == '2' && !in_set_mode){
        if(is_shifted && !is_hold){
            //shift 2 tunning step status
            int ts = radio_get_tuning_step();
            if(ts < 0){
                speech_say_text("tunning step unavailable");
            }else if (ts >= 1000) {
            char buffer[64];
            snprintf(buffer, sizeof(buffer), "Tuning step is %d kilohertz", ts / 1000);
            speech_say_text(buffer);
            }
            else {
            char buffer[64];
            snprintf(buffer, sizeof(buffer), "Tuning step is %d hertz", ts);
            speech_say_text(buffer);
            }
        }
        else if(is_shifted && is_hold){
            // shift hold 2 get squelch level
            
        }
        else if (!is_hold){
            announce_frequency();
            return true;
        }
        else{
            // hold 2 toggle memory scan
            int scan = radio_toggle_memory_scan();
            if(scan < 0){
                speech_say_text("memory scan unavailable");
            }else if(scan == 1){
                speech_say_text("memory scan on");
            }else{
                speech_say_text("memory scan diabled");
            }
        }
    }
    // [3] 
    if (key == '3' && !in_set_mode){
        if(is_shifted && !is_hold){
            //shift 3 Choose Duplex direction
        }
        else if(!is_hold){
            //press 3 toggle split mode
            int split = radio_toggle_split_mode();

            if (split < 0) {
                speech_say_text("Split mode unavailable");
            } else if (split == 1) {
                speech_say_text("Split mode is now on");
            } else {
                speech_say_text("Split mode is now off");
            }
        }
        else{
            //hold 3 Exchange VFO A and VFO b
            int exchange = radio_exchange_vfo();
            if (exchange < 0){
                speech_say_text("exchange vfo unavailable");
            }else{
                speech_say_text("VFO A and B exchanged");
            }
        }
    }
    
    // [4] - PreAmp (press) / AGC (hold) / Attenuation (shift+press)
    if (key == '4'&& !in_set_mode) {
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
    // [5] 
    if(key == '5' && !in_set_mode){
        if(is_shifted && !is_hold){
            speech_say_text("shift five");
        }
        else if(is_hold){
            // hold 5 functions implement in future
        }
        else{
            // press 5 functions implement in future
        }
    }

    // [6]
    if (key == '6'&& !in_set_mode) {
        if (is_shifted && !is_hold) {
            // shift + press 6 -> get filter number
            int filter = radio_get_filter_number();
            char msg[32];

            if (filter == -999) {
                speech_say_text("Filter number unavailable");
            } else {
                snprintf(msg, sizeof(msg), "Filter %d", filter);
                speech_say_text(msg);
            }
        }
        else if (!is_shifted && is_hold) {
            // hold 6 -> audio peaking filter
            int status = radio_get_apf_status();

            if (status == -999) {
                speech_say_text("Audio peaking filter unavailable");
            } else if (status) {
                speech_say_text("Audio peaking filter is on");
            } else {
                speech_say_text("Audio peaking filter is off");
            }
        }
        else if (!is_shifted && !is_hold) {
            // press 6 -> read filter width
            int width = radio_get_filter_width();
            char msg[64];

            if (width == -999) {
                speech_say_text("Filter width unavailable");
            }
            else if (width <= 0) {
                speech_say_text("Filter width is normal");
            }
            else if (width >= 1000) {
                snprintf(msg, sizeof(msg), "Filter width is %.1f kilohertz", width / 1000.0);
                speech_say_text(msg);
            }
            else {
                snprintf(msg, sizeof(msg), "Filter width is %d hertz", width);
                speech_say_text(msg);
            }
        }
    }
    // [7] - Noise Blanker query
    if (key == '7' && !in_set_mode){
        if(is_shifted && !is_hold){//shift 7, attenna selection status
            int ant = radio_get_antenna();
            char buffer[64];

            if (ant > 0) {
                snprintf(buffer, sizeof(buffer), "Antenna %d", ant);
            } else {
                snprintf(buffer, sizeof(buffer), "Antenna not available");
            }

            speech_say_text(buffer);
            return true;
        }
        else if(!is_hold){
            char buffer[64];
            bool nb_on = radio_get_nb_enabled();
            int nb_level = radio_get_nb_level();
            snprintf(buffer, sizeof(buffer), "Noise blanker %s, level %d", 
                    nb_on ? "on" : "off", nb_level >= 0 ? nb_level : 0);
            speech_say_text(buffer);
            return true;
        }
        else{// attena tuner status
            int tuner = radio_get_tuner_status();
            char buffer[64];

            if (tuner >= 0) {
                snprintf(buffer, sizeof(buffer), "Tuner %s", tuner ? "on" : "off");
            } else {
                snprintf(buffer, sizeof(buffer), "Tuner status not available");
            }
            speech_say_text(buffer);
            return true;
        }
    }

    
    // [8] - Noise Reduction (press) / Mic Gain (hold)
    if (key == '8'&& !in_set_mode) {
        char buffer[64];
        if (is_shifted && !is_hold){
            int speed = radio_get_keyer_speed();
            char buffer[64];

            if (speed > 0) {
                snprintf(buffer, sizeof(buffer),
                        "Keyer speed %d words per minute", speed);
            } else {
                snprintf(buffer, sizeof(buffer),
                        "Keyer speed not available");
            }

            speech_say_text(buffer);
            return true;
        }
        else if (is_hold) {
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
    if (key == '9' && !in_set_mode) {
        char buffer[64];
        if (is_shifted && !is_hold /*&& not in set mode idle*/) {
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
    
    // [C] - Toggle verbosity (press) / Config mode entry (hold)
    if (key == 'C') {
        if (is_hold) {
            config_mode_enter();
            return true;
        } else {
            g_verbosity_enabled = !g_verbosity_enabled;
            if (g_verbosity_enabled) {
                speech_say_text("Announcements on");
            } else {
                speech_say_text("Announcements off");
            }
            return true;
        }
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
