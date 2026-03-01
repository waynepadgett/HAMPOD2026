#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "config_mode.h"

// Stubs for dependencies since we only want to test config_mode logic
int config_init(const char *config_path) { return 0; }
int config_save(void) { return 0; }
int config_undo(void) { return 0; }
int config_get_undo_count(void) { return 0; }

static int g_vol = 50;
int config_get_volume(void) { return g_vol; }
void config_set_volume(int volume) { g_vol = volume; }

static float g_speed = 1.0f;
float config_get_speech_speed(void) { return g_speed; }
void config_set_speech_speed(float speed) { g_speed = speed; }

static char g_layout[32] = "calculator";
const char *config_get_keypad_layout(void) { return g_layout; }
void config_set_keypad_layout(const char *layout) { strcpy(g_layout, layout); }

bool config_get_key_beep_enabled(void) { return false; }
int config_get_audio_card_number(void) { return 2; }

void speech_say_text(const char *text) {
  // printf("SPEECH: %s\n", text);
}
int comm_set_speech_speed(float speed) { return 0; }
int comm_play_beep(int type) { return 0; }
int comm_send_config_packet(unsigned char sc, unsigned char v) { return 0; }

// Test Helpers
void test_entry(void) {
  config_mode_init();
  assert(!config_mode_is_active());
  config_mode_enter();
  assert(config_mode_is_active());
  assert(config_mode_get_state() == CONFIG_MODE_BROWSING);
  assert(config_mode_get_parameter() == CONFIG_PARAM_VOLUME);
  printf("test_entry passed\n");
}

void test_exit_save(void) {
  config_mode_init();
  config_mode_enter();
  config_mode_handle_key('C', true); // Hold C
  assert(!config_mode_is_active());
  printf("test_exit_save passed\n");
}

int main(void) {
  printf("Running config mode tests...\n");
  test_entry();
  test_exit_save();
  printf("All config mode tests passed!\n");
  return 0;
}
