/**
 * @file config.c
 * @brief Configuration management implementation
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "config.h"

// ============================================================================
// Internal State
// ============================================================================

// Undo history buffer
typedef struct {
    HampodConfig snapshots[CONFIG_UNDO_DEPTH];
    int head;   // Next write position
    int count;  // Number of valid snapshots (0 to CONFIG_UNDO_DEPTH)
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
static void history_push(const HampodConfig* config);
static int history_pop(HampodConfig* config);
static int config_parse_file(const char* path);
static int config_write_file(const char* path);

// ============================================================================
// Core Functions
// ============================================================================

int config_init(const char* config_path) {
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
    if (!g_initialized) return -1;
    
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
    if (!g_initialized) return -1;
    
    pthread_mutex_lock(&g_config_mutex);
    
    HampodConfig prev;
    int result = history_pop(&prev);
    if (result == 0) {
        g_config = prev;
        config_write_file(g_config_path);  // Auto-save after undo
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
// Radio Getters
// ============================================================================

int config_get_radio_model(void) {
    pthread_mutex_lock(&g_config_mutex);
    int val = g_config.radio_model;
    pthread_mutex_unlock(&g_config_mutex);
    return val;
}

const char* config_get_radio_device(void) {
    // Note: Returns pointer to internal buffer - caller should copy if needed
    return g_config.radio_device;
}

int config_get_radio_baud(void) {
    pthread_mutex_lock(&g_config_mutex);
    int val = g_config.radio_baud;
    pthread_mutex_unlock(&g_config_mutex);
    return val;
}

// ============================================================================
// Audio Getters
// ============================================================================

int config_get_volume(void) {
    pthread_mutex_lock(&g_config_mutex);
    int val = g_config.output_volume;
    pthread_mutex_unlock(&g_config_mutex);
    return val;
}

float config_get_speech_speed(void) {
    pthread_mutex_lock(&g_config_mutex);
    float val = g_config.speech_speed;
    pthread_mutex_unlock(&g_config_mutex);
    return val;
}

bool config_get_key_beep_enabled(void) {
    pthread_mutex_lock(&g_config_mutex);
    bool val = g_config.key_beep_enabled;
    pthread_mutex_unlock(&g_config_mutex);
    return val;
}

// ============================================================================
// Radio Setters (with auto-save and undo)
// ============================================================================

void config_set_radio_model(int model) {
    if (!g_initialized) return;
    
    pthread_mutex_lock(&g_config_mutex);
    history_push(&g_config);
    g_config.radio_model = model;
    config_write_file(g_config_path);
    pthread_mutex_unlock(&g_config_mutex);
}

void config_set_radio_device(const char* device) {
    if (!g_initialized || !device) return;
    
    pthread_mutex_lock(&g_config_mutex);
    history_push(&g_config);
    strncpy(g_config.radio_device, device, sizeof(g_config.radio_device) - 1);
    g_config.radio_device[sizeof(g_config.radio_device) - 1] = '\0';
    config_write_file(g_config_path);
    pthread_mutex_unlock(&g_config_mutex);
}

void config_set_radio_baud(int baud) {
    if (!g_initialized) return;
    
    pthread_mutex_lock(&g_config_mutex);
    history_push(&g_config);
    g_config.radio_baud = baud;
    config_write_file(g_config_path);
    pthread_mutex_unlock(&g_config_mutex);
}

// ============================================================================
// Audio Setters (with auto-save and undo)
// ============================================================================

void config_set_volume(int volume) {
    if (!g_initialized) return;
    
    // Clamp to valid range
    if (volume < 0) volume = 0;
    if (volume > 100) volume = 100;
    
    pthread_mutex_lock(&g_config_mutex);
    history_push(&g_config);
    g_config.output_volume = volume;
    config_write_file(g_config_path);
    pthread_mutex_unlock(&g_config_mutex);
}

void config_set_speech_speed(float speed) {
    if (!g_initialized) return;
    
    // Clamp to valid range
    if (speed < 0.5f) speed = 0.5f;
    if (speed > 2.0f) speed = 2.0f;
    
    pthread_mutex_lock(&g_config_mutex);
    history_push(&g_config);
    g_config.speech_speed = speed;
    config_write_file(g_config_path);
    pthread_mutex_unlock(&g_config_mutex);
}

void config_set_key_beep_enabled(bool enabled) {
    if (!g_initialized) return;
    
    pthread_mutex_lock(&g_config_mutex);
    history_push(&g_config);
    g_config.key_beep_enabled = enabled;
    config_write_file(g_config_path);
    pthread_mutex_unlock(&g_config_mutex);
}

// ============================================================================
// Internal Functions
// ============================================================================

static void config_set_defaults(void) {
    g_config.radio_model = CONFIG_DEFAULT_RADIO_MODEL;
    strncpy(g_config.radio_device, CONFIG_DEFAULT_RADIO_DEVICE, 
            sizeof(g_config.radio_device) - 1);
    g_config.radio_device[sizeof(g_config.radio_device) - 1] = '\0';
    g_config.radio_baud = CONFIG_DEFAULT_RADIO_BAUD;
    g_config.output_volume = CONFIG_DEFAULT_VOLUME;
    g_config.speech_speed = CONFIG_DEFAULT_SPEECH_SPEED;
    g_config.key_beep_enabled = CONFIG_DEFAULT_KEY_BEEP;
}

static void history_push(const HampodConfig* config) {
    // Circular buffer push
    g_history.snapshots[g_history.head] = *config;
    g_history.head = (g_history.head + 1) % CONFIG_UNDO_DEPTH;
    if (g_history.count < CONFIG_UNDO_DEPTH) {
        g_history.count++;
    }
}

static int history_pop(HampodConfig* config) {
    if (g_history.count == 0) {
        return -1;  // No undo available
    }
    
    // Move head back
    g_history.head = (g_history.head - 1 + CONFIG_UNDO_DEPTH) % CONFIG_UNDO_DEPTH;
    g_history.count--;
    
    *config = g_history.snapshots[g_history.head];
    return 0;
}

// ============================================================================
// File I/O (INI format)
// ============================================================================

static int config_parse_file(const char* path) {
    FILE* fp = fopen(path, "r");
    if (!fp) {
        // File doesn't exist - use defaults (not an error)
        return 0;
    }
    
    char line[256];
    char section[64] = "";
    
    while (fgets(line, sizeof(line), fp)) {
        // Skip comments and empty lines
        if (line[0] == '#' || line[0] == ';' || line[0] == '\n') {
            continue;
        }
        
        // Section header [section]
        if (line[0] == '[') {
            char* end = strchr(line, ']');
            if (end) {
                *end = '\0';
                strncpy(section, line + 1, sizeof(section) - 1);
                section[sizeof(section) - 1] = '\0';
            }
            continue;
        }
        
        // Key = Value
        char* eq = strchr(line, '=');
        if (!eq) continue;
        
        *eq = '\0';
        char* key = line;
        char* value = eq + 1;
        
        // Trim whitespace from key
        while (*key == ' ' || *key == '\t') key++;
        char* key_end = key + strlen(key) - 1;
        while (key_end > key && (*key_end == ' ' || *key_end == '\t')) {
            *key_end-- = '\0';
        }
        
        // Trim whitespace from value
        while (*value == ' ' || *value == '\t') value++;
        char* value_end = value + strlen(value) - 1;
        while (value_end > value && (*value_end == ' ' || *value_end == '\t' || 
               *value_end == '\n' || *value_end == '\r')) {
            *value_end-- = '\0';
        }
        
        // Parse based on section and key
        if (strcmp(section, "radio") == 0) {
            if (strcmp(key, "model") == 0) {
                g_config.radio_model = atoi(value);
            } else if (strcmp(key, "device") == 0) {
                strncpy(g_config.radio_device, value, sizeof(g_config.radio_device) - 1);
                g_config.radio_device[sizeof(g_config.radio_device) - 1] = '\0';
            } else if (strcmp(key, "baud") == 0) {
                g_config.radio_baud = atoi(value);
            }
        } else if (strcmp(section, "audio") == 0) {
            if (strcmp(key, "volume") == 0) {
                g_config.output_volume = atoi(value);
            } else if (strcmp(key, "speech_speed") == 0) {
                g_config.speech_speed = (float)atof(value);
            } else if (strcmp(key, "key_beep") == 0) {
                g_config.key_beep_enabled = atoi(value) != 0;
            }
        }
    }
    
    fclose(fp);
    return 0;
}

static int config_write_file(const char* path) {
    FILE* fp = fopen(path, "w");
    if (!fp) {
        return -1;
    }
    
    fprintf(fp, "# HAMPOD Configuration\n");
    fprintf(fp, "# Auto-generated - edit with care\n\n");
    
    fprintf(fp, "[radio]\n");
    fprintf(fp, "model = %d\n", g_config.radio_model);
    fprintf(fp, "device = %s\n", g_config.radio_device);
    fprintf(fp, "baud = %d\n\n", g_config.radio_baud);
    
    fprintf(fp, "[audio]\n");
    fprintf(fp, "volume = %d\n", g_config.output_volume);
    fprintf(fp, "speech_speed = %.1f\n", g_config.speech_speed);
    fprintf(fp, "key_beep = %d\n", g_config.key_beep_enabled ? 1 : 0);
    
    fclose(fp);
    return 0;
}
