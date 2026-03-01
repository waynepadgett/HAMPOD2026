/**
 * @file config.c
 * @brief Configuration management implementation
 */

#include "config.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ============================================================================
// Internal State
// ============================================================================

// Undo history buffer
typedef struct {
  HampodConfig snapshots[CONFIG_UNDO_DEPTH];
  int head;  // Next write position
  int count; // Number of valid snapshots (0 to CONFIG_UNDO_DEPTH)
} ConfigHistory;

// Global state
static HampodConfig g_config;
static ConfigHistory g_history;
static char g_config_path[256];
static pthread_mutex_t g_config_mutex = PTHREAD_MUTEX_INITIALIZER;
static bool g_initialized = false;

// ============================================================================
// Internal Functions (forward declarations)
// ============================================================================

static void config_set_defaults(void);
static void history_push(const HampodConfig *config);
static int history_pop(HampodConfig *config);
static int config_parse_file(const char *path);
static int config_write_file(const char *path);

// ============================================================================
// Core Functions
// ============================================================================

int config_init(const char *config_path) {
  pthread_mutex_lock(&g_config_mutex);

  // Set defaults first
  config_set_defaults();

  // Clear history
  g_history.head = 0;
  g_history.count = 0;

  // Store config path
  if (config_path) {
    strncpy(g_config_path, config_path, sizeof(g_config_path) - 1);
    g_config_path[sizeof(g_config_path) - 1] = '\0';
  } else {
    strncpy(g_config_path, CONFIG_DEFAULT_PATH, sizeof(g_config_path) - 1);
    g_config_path[sizeof(g_config_path) - 1] = '\0';
  }

  // Try to load from file (overrides defaults if file exists)
  config_parse_file(g_config_path);

  g_initialized = true;
  pthread_mutex_unlock(&g_config_mutex);

  return 0;
}

int config_save(void) {
  if (!g_initialized)
    return -1;

  pthread_mutex_lock(&g_config_mutex);
  int result = config_write_file(g_config_path);
  pthread_mutex_unlock(&g_config_mutex);

  return result;
}

void config_cleanup(void) {
  pthread_mutex_lock(&g_config_mutex);
  g_initialized = false;
  g_history.count = 0;
  pthread_mutex_unlock(&g_config_mutex);
}

// ============================================================================
// Undo Functions
// ============================================================================

int config_undo(void) {
  if (!g_initialized)
    return -1;

  pthread_mutex_lock(&g_config_mutex);

  HampodConfig prev;
  int result = history_pop(&prev);
  if (result == 0) {
    g_config = prev;
    config_write_file(g_config_path); // Auto-save after undo
  }

  pthread_mutex_unlock(&g_config_mutex);
  return result;
}

int config_get_undo_count(void) {
  pthread_mutex_lock(&g_config_mutex);
  int count = g_history.count;
  pthread_mutex_unlock(&g_config_mutex);
  return count;
}

// ============================================================================
// Radio Management
// ============================================================================

int config_get_active_radio_index(void) {
  pthread_mutex_lock(&g_config_mutex);
  int active = -1;
  for (int i = 0; i < MAX_RADIOS; i++) {
    if (g_config.radios[i].enabled) {
      active = i;
      break;
    }
  }
  pthread_mutex_unlock(&g_config_mutex);
  return active;
}

const RadioSettings *config_get_radio(int index) {
  if (index < 0 || index >= MAX_RADIOS)
    return NULL;
  return &g_config.radios[index];
}

void config_set_radio_enabled(int index, bool enabled) {
  if (!g_initialized || index < 0 || index >= MAX_RADIOS)
    return;

  pthread_mutex_lock(&g_config_mutex);
  history_push(&g_config);

  // If enabling, disable all others
  if (enabled) {
    for (int i = 0; i < MAX_RADIOS; i++) {
      g_config.radios[i].enabled = false;
    }
  }
  g_config.radios[index].enabled = enabled;

  config_write_file(g_config_path);
  pthread_mutex_unlock(&g_config_mutex);
}

// Helper to get pointer to active radio (caller MUST hold mutex)
static RadioSettings *get_active_radio_internal(void) {
  for (int i = 0; i < MAX_RADIOS; i++) {
    if (g_config.radios[i].enabled)
      return &g_config.radios[i];
  }
  return &g_config.radios[0]; // Fallback to first radio
}

// ============================================================================
// Radio Getters (Act on the currently active radio)
// ============================================================================

int config_get_radio_model(void) {
  pthread_mutex_lock(&g_config_mutex);
  int val = get_active_radio_internal()->model;
  pthread_mutex_unlock(&g_config_mutex);
  return val;
}

const char *config_get_radio_device(void) {
  // Note: Returns pointer to internal buffer
  return get_active_radio_internal()->device;
}

int config_get_radio_baud(void) {
  pthread_mutex_lock(&g_config_mutex);
  int val = get_active_radio_internal()->baud;
  pthread_mutex_unlock(&g_config_mutex);
  return val;
}

const char *config_get_radio_name(void) {
  return get_active_radio_internal()->name;
}

const char *config_get_radio_port(void) {
  return get_active_radio_internal()->port;
}

int config_get_radio_detected_model(void) {
  pthread_mutex_lock(&g_config_mutex);
  int val = get_active_radio_internal()->detected_model;
  pthread_mutex_unlock(&g_config_mutex);
  return val;
}

// ============================================================================
// Audio Getters
// ============================================================================

int config_get_volume(void) {
  pthread_mutex_lock(&g_config_mutex);
  int val = g_config.audio.volume;
  pthread_mutex_unlock(&g_config_mutex);
  return val;
}

float config_get_speech_speed(void) {
  pthread_mutex_lock(&g_config_mutex);
  float val = g_config.audio.speech_speed;
  pthread_mutex_unlock(&g_config_mutex);
  return val;
}

bool config_get_key_beep_enabled(void) {
  pthread_mutex_lock(&g_config_mutex);
  bool val = g_config.audio.key_beep_enabled;
  pthread_mutex_unlock(&g_config_mutex);
  return val;
}

const char *config_get_audio_preferred_device(void) {
  return g_config.audio.preferred_device;
}

const char *config_get_audio_device_name(void) {
  return g_config.audio.device_name;
}

const char *config_get_audio_port(void) { return g_config.audio.port; }

int config_get_audio_card_number(void) {
  pthread_mutex_lock(&g_config_mutex);
  int val = g_config.audio.card_number;
  pthread_mutex_unlock(&g_config_mutex);
  return val;
}

// ============================================================================
// Keypad Getters
// ============================================================================

const char *config_get_keypad_port(void) { return g_config.keypad.port; }

const char *config_get_keypad_device_name(void) {
  return g_config.keypad.device_name;
}

const char *config_get_keypad_layout(void) {
  if (g_config.keypad.layout[0] == '\0')
    return "calculator";
  return g_config.keypad.layout;
}

// ============================================================================
// Radio Setters (Act on the currently active radio, auto-save after each)
// ============================================================================

void config_set_radio_model(int model) {
  if (!g_initialized)
    return;
  pthread_mutex_lock(&g_config_mutex);
  history_push(&g_config);
  get_active_radio_internal()->model = model;
  config_write_file(g_config_path);
  pthread_mutex_unlock(&g_config_mutex);
}

void config_set_radio_device(const char *device) {
  if (!g_initialized || !device)
    return;
  pthread_mutex_lock(&g_config_mutex);
  history_push(&g_config);
  strncpy(get_active_radio_internal()->device, device, 63);
  get_active_radio_internal()->device[63] = '\0';
  config_write_file(g_config_path);
  pthread_mutex_unlock(&g_config_mutex);
}

void config_set_radio_baud(int baud) {
  if (!g_initialized)
    return;
  pthread_mutex_lock(&g_config_mutex);
  history_push(&g_config);
  get_active_radio_internal()->baud = baud;
  config_write_file(g_config_path);
  pthread_mutex_unlock(&g_config_mutex);
}

void config_set_radio_name(const char *name) {
  if (!g_initialized || !name)
    return;
  pthread_mutex_lock(&g_config_mutex);
  history_push(&g_config);
  strncpy(get_active_radio_internal()->name, name, 63);
  get_active_radio_internal()->name[63] = '\0';
  config_write_file(g_config_path);
  pthread_mutex_unlock(&g_config_mutex);
}

void config_set_radio_port(const char *port) {
  if (!g_initialized || !port)
    return;
  pthread_mutex_lock(&g_config_mutex);
  history_push(&g_config);
  strncpy(get_active_radio_internal()->port, port, 127);
  get_active_radio_internal()->port[127] = '\0';
  config_write_file(g_config_path);
  pthread_mutex_unlock(&g_config_mutex);
}

void config_set_radio_detected_model(int model) {
  if (!g_initialized)
    return;
  pthread_mutex_lock(&g_config_mutex);
  history_push(&g_config);
  get_active_radio_internal()->detected_model = model;
  config_write_file(g_config_path);
  pthread_mutex_unlock(&g_config_mutex);
}

// ============================================================================
// Audio Setters (auto-save after each)
// ============================================================================

void config_set_volume(int volume) {
  if (!g_initialized)
    return;
  if (volume < 0)
    volume = 0;
  if (volume > 100)
    volume = 100;
  pthread_mutex_lock(&g_config_mutex);
  history_push(&g_config);
  g_config.audio.volume = volume;
  config_write_file(g_config_path);
  pthread_mutex_unlock(&g_config_mutex);
}

void config_set_speech_speed(float speed) {
  if (!g_initialized)
    return;
  if (speed < 0.5f)
    speed = 0.5f;
  if (speed > 2.0f)
    speed = 2.0f;
  pthread_mutex_lock(&g_config_mutex);
  history_push(&g_config);
  g_config.audio.speech_speed = speed;
  config_write_file(g_config_path);
  pthread_mutex_unlock(&g_config_mutex);
}

void config_set_key_beep_enabled(bool enabled) {
  if (!g_initialized)
    return;
  pthread_mutex_lock(&g_config_mutex);
  history_push(&g_config);
  g_config.audio.key_beep_enabled = enabled;
  config_write_file(g_config_path);
  pthread_mutex_unlock(&g_config_mutex);
}

void config_set_audio_device_name(const char *name) {
  if (!g_initialized || !name)
    return;
  pthread_mutex_lock(&g_config_mutex);
  history_push(&g_config);
  strncpy(g_config.audio.device_name, name, 127);
  g_config.audio.device_name[127] = '\0';
  config_write_file(g_config_path);
  pthread_mutex_unlock(&g_config_mutex);
}

void config_set_audio_port(const char *port) {
  if (!g_initialized || !port)
    return;
  pthread_mutex_lock(&g_config_mutex);
  history_push(&g_config);
  strncpy(g_config.audio.port, port, 127);
  g_config.audio.port[127] = '\0';
  config_write_file(g_config_path);
  pthread_mutex_unlock(&g_config_mutex);
}

void config_set_audio_card_number(int card) {
  if (!g_initialized)
    return;
  pthread_mutex_lock(&g_config_mutex);
  history_push(&g_config);
  g_config.audio.card_number = card;
  config_write_file(g_config_path);
  pthread_mutex_unlock(&g_config_mutex);
}

// ============================================================================
// Keypad Setters (auto-save after each)
// ============================================================================

void config_set_keypad_port(const char *port) {
  if (!g_initialized || !port)
    return;
  pthread_mutex_lock(&g_config_mutex);
  history_push(&g_config);
  strncpy(g_config.keypad.port, port, 127);
  g_config.keypad.port[127] = '\0';
  config_write_file(g_config_path);
  pthread_mutex_unlock(&g_config_mutex);
}

void config_set_keypad_device_name(const char *name) {
  if (!g_initialized || !name)
    return;
  pthread_mutex_lock(&g_config_mutex);
  history_push(&g_config);
  strncpy(g_config.keypad.device_name, name, 127);
  g_config.keypad.device_name[127] = '\0';
  config_write_file(g_config_path);
  pthread_mutex_unlock(&g_config_mutex);
}

void config_set_keypad_layout(const char *layout) {
  if (!g_initialized || !layout)
    return;
  pthread_mutex_lock(&g_config_mutex);
  history_push(&g_config);
  strncpy(g_config.keypad.layout, layout, 15);
  g_config.keypad.layout[15] = '\0';
  config_write_file(g_config_path);
  pthread_mutex_unlock(&g_config_mutex);
}

// ============================================================================
// Internal Functions
// ============================================================================

static void config_set_defaults(void) {
  memset(&g_config, 0, sizeof(HampodConfig));

  // Default radio 1
  g_config.radios[0].enabled = true;
  strcpy(g_config.radios[0].name, "Primary Radio");
  g_config.radios[0].model = CONFIG_DEFAULT_RADIO_MODEL;
  strcpy(g_config.radios[0].device, CONFIG_DEFAULT_RADIO_DEVICE);
  g_config.radios[0].baud = CONFIG_DEFAULT_RADIO_BAUD;

  // Audio defaults
  strcpy(g_config.audio.preferred_device, "USB2.0 Device");
  g_config.audio.volume = CONFIG_DEFAULT_VOLUME;
  g_config.audio.speech_speed = CONFIG_DEFAULT_SPEECH_SPEED;
  g_config.audio.key_beep_enabled = CONFIG_DEFAULT_KEY_BEEP;
  g_config.audio.card_number = -1;

  // Keypad defaults
  strcpy(g_config.keypad.layout, "calculator");
}

static void history_push(const HampodConfig *config) {
  g_history.snapshots[g_history.head] = *config;
  g_history.head = (g_history.head + 1) % CONFIG_UNDO_DEPTH;
  if (g_history.count < CONFIG_UNDO_DEPTH) {
    g_history.count++;
  }
}

static int history_pop(HampodConfig *config) {
  if (g_history.count == 0)
    return -1;
  g_history.head = (g_history.head - 1 + CONFIG_UNDO_DEPTH) % CONFIG_UNDO_DEPTH;
  g_history.count--;
  *config = g_history.snapshots[g_history.head];
  return 0;
}

// ============================================================================
// File I/O (INI format)
// ============================================================================

static int config_parse_file(const char *path) {
  FILE *fp = fopen(path, "r");
  if (!fp)
    return 0;

  char line[512];
  char section[64] = "";

  while (fgets(line, sizeof(line), fp)) {
    if (line[0] == '#' || line[0] == ';' || line[0] == '\n')
      continue;

    if (line[0] == '[') {
      char *end = strchr(line, ']');
      if (end) {
        *end = '\0';
        strncpy(section, line + 1, sizeof(section) - 1);
      }
      continue;
    }

    char *eq = strchr(line, '=');
    if (!eq)
      continue;

    *eq = '\0';
    char *key = line;
    char *value = eq + 1;

    // Trim
    while (*key == ' ' || *key == '\t')
      key++;
    char *k_end = key + strlen(key) - 1;
    while (k_end > key && (*k_end == ' ' || *k_end == '\t'))
      *k_end-- = '\0';

    while (*value == ' ' || *value == '\t')
      value++;
    char *v_end = value + strlen(value) - 1;
    while (v_end > value && (*v_end == ' ' || *v_end == '\t' ||
                             *v_end == '\n' || *v_end == '\r'))
      *v_end-- = '\0';

    // Handle [radio.X]
    if (strncmp(section, "radio.", 6) == 0) {
      int idx = atoi(section + 6) - 1;
      if (idx >= 0 && idx < MAX_RADIOS) {
        if (strcmp(key, "enabled") == 0)
          g_config.radios[idx].enabled =
              (strcmp(value, "true") == 0 || atoi(value) != 0);
        else if (strcmp(key, "name") == 0)
          strncpy(g_config.radios[idx].name, value, 63);
        else if (strcmp(key, "model") == 0)
          g_config.radios[idx].model = atoi(value);
        else if (strcmp(key, "device") == 0)
          strncpy(g_config.radios[idx].device, value, 63);
        else if (strcmp(key, "baud") == 0)
          g_config.radios[idx].baud = atoi(value);
        else if (strcmp(key, "port") == 0)
          strncpy(g_config.radios[idx].port, value, 127);
        else if (strcmp(key, "detected_model") == 0)
          g_config.radios[idx].detected_model = atoi(value);
      }
    }
    // Backward compatibility for old [radio] section
    else if (strcmp(section, "radio") == 0) {
      if (strcmp(key, "model") == 0)
        g_config.radios[0].model = atoi(value);
      else if (strcmp(key, "device") == 0)
        strncpy(g_config.radios[0].device, value, 63);
      else if (strcmp(key, "baud") == 0)
        g_config.radios[0].baud = atoi(value);
    } else if (strcmp(section, "audio") == 0) {
      if (strcmp(key, "preferred_device") == 0)
        strncpy(g_config.audio.preferred_device, value, 63);
      else if (strcmp(key, "device_name") == 0)
        strncpy(g_config.audio.device_name, value, 127);
      else if (strcmp(key, "port") == 0)
        strncpy(g_config.audio.port, value, 127);
      else if (strcmp(key, "volume") == 0)
        g_config.audio.volume = atoi(value);
      else if (strcmp(key, "speech_speed") == 0)
        g_config.audio.speech_speed = (float)atof(value);
      else if (strcmp(key, "key_beep") == 0)
        g_config.audio.key_beep_enabled = (atoi(value) != 0);
      else if (strcmp(key, "card_number") == 0)
        g_config.audio.card_number = atoi(value);
    } else if (strcmp(section, "keypad") == 0) {
      if (strcmp(key, "port") == 0)
        strncpy(g_config.keypad.port, value, 127);
      else if (strcmp(key, "device_name") == 0)
        strncpy(g_config.keypad.device_name, value, 127);
      else if (strcmp(key, "layout") == 0)
        strncpy(g_config.keypad.layout, value, 15);
    }
  }

  fclose(fp);
  return 0;
}

static int config_write_file(const char *path) {
  FILE *fp = fopen(path, "w");
  if (!fp)
    return -1;

  fprintf(fp, "# HAMPOD Configuration\n# Auto-generated - edit with care\n\n");

  for (int i = 0; i < MAX_RADIOS; i++) {
    // Only write radio sections that are either enabled or have a name/model
    // set
    if (g_config.radios[i].enabled || g_config.radios[i].model != 0) {
      fprintf(fp, "[radio.%d]\n", i + 1);
      fprintf(fp, "enabled = %d\n", g_config.radios[i].enabled ? 1 : 0);
      fprintf(fp, "name = %s\n", g_config.radios[i].name);
      fprintf(fp, "model = %d\n", g_config.radios[i].model);
      fprintf(fp, "device = %s\n", g_config.radios[i].device);
      fprintf(fp, "baud = %d\n", g_config.radios[i].baud);
      fprintf(fp, "port = %s\n", g_config.radios[i].port);
      fprintf(fp, "detected_model = %d\n\n", g_config.radios[i].detected_model);
    }
  }

  fprintf(fp, "[audio]\n");
  fprintf(fp, "preferred_device = %s\n", g_config.audio.preferred_device);
  fprintf(fp, "device_name = %s\n", g_config.audio.device_name);
  fprintf(fp, "port = %s\n", g_config.audio.port);
  fprintf(fp, "card_number = %d\n", g_config.audio.card_number);
  fprintf(fp, "volume = %d\n", g_config.audio.volume);
  fprintf(fp, "speech_speed = %.2f\n", g_config.audio.speech_speed);
  fprintf(fp, "key_beep = %d\n\n", g_config.audio.key_beep_enabled ? 1 : 0);

  fprintf(fp, "[keypad]\n");
  fprintf(fp, "layout = %s\n", g_config.keypad.layout);
  fprintf(fp, "port = %s\n", g_config.keypad.port);
  fprintf(fp, "device_name = %s\n", g_config.keypad.device_name);

  fclose(fp);
  return 0;
}
