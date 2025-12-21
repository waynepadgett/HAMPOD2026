/**
 * @file test_config.c
 * @brief Unit tests for configuration module
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include "config.h"

#define TEST_CONFIG_PATH "/tmp/test_hampod.conf"

static int tests_passed = 0;
static int tests_failed = 0;

#define TEST(name) \
    printf("Testing: %s... ", name);

#define PASS() \
    do { printf("PASS\n"); tests_passed++; } while(0)

#define FAIL(msg) \
    do { printf("FAIL: %s\n", msg); tests_failed++; } while(0)

// ============================================================================
// Test Functions
// ============================================================================

void test_init_defaults(void) {
    TEST("init with no file uses defaults");
    
    // Remove test file if exists
    unlink(TEST_CONFIG_PATH);
    
    config_init(TEST_CONFIG_PATH);
    
    if (config_get_radio_model() != CONFIG_DEFAULT_RADIO_MODEL) {
        FAIL("radio_model not default");
        return;
    }
    if (config_get_volume() != CONFIG_DEFAULT_VOLUME) {
        FAIL("volume not default");
        return;
    }
    if (config_get_speech_speed() != CONFIG_DEFAULT_SPEECH_SPEED) {
        FAIL("speech_speed not default");
        return;
    }
    
    config_cleanup();
    PASS();
}

void test_setters_update_values(void) {
    TEST("setters update values");
    
    unlink(TEST_CONFIG_PATH);
    config_init(TEST_CONFIG_PATH);
    
    config_set_volume(50);
    if (config_get_volume() != 50) {
        FAIL("volume not updated");
        config_cleanup();
        return;
    }
    
    config_set_speech_speed(1.5f);
    if (config_get_speech_speed() < 1.49f || config_get_speech_speed() > 1.51f) {
        FAIL("speech_speed not updated");
        config_cleanup();
        return;
    }
    
    config_set_radio_model(9999);
    if (config_get_radio_model() != 9999) {
        FAIL("radio_model not updated");
        config_cleanup();
        return;
    }
    
    config_cleanup();
    PASS();
}

void test_auto_save(void) {
    TEST("setters auto-save to file");
    
    unlink(TEST_CONFIG_PATH);
    config_init(TEST_CONFIG_PATH);
    
    // Set a value (should auto-save)
    config_set_volume(42);
    config_cleanup();
    
    // Reinit and check value persisted
    config_init(TEST_CONFIG_PATH);
    if (config_get_volume() != 42) {
        FAIL("volume not persisted");
        config_cleanup();
        return;
    }
    
    config_cleanup();
    unlink(TEST_CONFIG_PATH);
    PASS();
}

void test_undo_single(void) {
    TEST("undo restores previous value");
    
    unlink(TEST_CONFIG_PATH);
    config_init(TEST_CONFIG_PATH);
    
    // Initial value
    int original = config_get_volume();
    
    // Change it
    config_set_volume(25);
    if (config_get_volume() != 25) {
        FAIL("volume not set");
        config_cleanup();
        return;
    }
    
    // Undo
    if (config_undo() != 0) {
        FAIL("undo failed");
        config_cleanup();
        return;
    }
    
    if (config_get_volume() != original) {
        FAIL("undo did not restore original value");
        config_cleanup();
        return;
    }
    
    config_cleanup();
    unlink(TEST_CONFIG_PATH);
    PASS();
}

void test_undo_count(void) {
    TEST("undo count tracks history");
    
    unlink(TEST_CONFIG_PATH);
    config_init(TEST_CONFIG_PATH);
    
    if (config_get_undo_count() != 0) {
        FAIL("initial undo count not 0");
        config_cleanup();
        return;
    }
    
    config_set_volume(10);
    if (config_get_undo_count() != 1) {
        FAIL("undo count not 1 after first set");
        config_cleanup();
        return;
    }
    
    config_set_volume(20);
    config_set_volume(30);
    if (config_get_undo_count() != 3) {
        FAIL("undo count not 3 after three sets");
        config_cleanup();
        return;
    }
    
    config_cleanup();
    unlink(TEST_CONFIG_PATH);
    PASS();
}

void test_undo_max_depth(void) {
    TEST("undo respects max depth (10)");
    
    unlink(TEST_CONFIG_PATH);
    config_init(TEST_CONFIG_PATH);
    
    // Make 15 changes
    for (int i = 0; i < 15; i++) {
        config_set_volume(i);
    }
    
    // Should only have CONFIG_UNDO_DEPTH (10) undos
    if (config_get_undo_count() != CONFIG_UNDO_DEPTH) {
        char msg[64];
        snprintf(msg, sizeof(msg), "undo count is %d, expected %d", 
                 config_get_undo_count(), CONFIG_UNDO_DEPTH);
        FAIL(msg);
        config_cleanup();
        return;
    }
    
    // Undo all 10
    for (int i = 0; i < CONFIG_UNDO_DEPTH; i++) {
        if (config_undo() != 0) {
            FAIL("undo failed before reaching limit");
            config_cleanup();
            return;
        }
    }
    
    // 11th undo should fail
    if (config_undo() == 0) {
        FAIL("undo succeeded past limit");
        config_cleanup();
        return;
    }
    
    config_cleanup();
    unlink(TEST_CONFIG_PATH);
    PASS();
}

void test_value_clamping(void) {
    TEST("setters clamp values to valid range");
    
    unlink(TEST_CONFIG_PATH);
    config_init(TEST_CONFIG_PATH);
    
    // Volume clamping
    config_set_volume(-10);
    if (config_get_volume() != 0) {
        FAIL("volume not clamped to 0");
        config_cleanup();
        return;
    }
    
    config_set_volume(150);
    if (config_get_volume() != 100) {
        FAIL("volume not clamped to 100");
        config_cleanup();
        return;
    }
    
    // Speech speed clamping
    config_set_speech_speed(0.1f);
    if (config_get_speech_speed() < 0.49f || config_get_speech_speed() > 0.51f) {
        FAIL("speech_speed not clamped to 0.5");
        config_cleanup();
        return;
    }
    
    config_set_speech_speed(5.0f);
    if (config_get_speech_speed() < 1.99f || config_get_speech_speed() > 2.01f) {
        FAIL("speech_speed not clamped to 2.0");
        config_cleanup();
        return;
    }
    
    config_cleanup();
    unlink(TEST_CONFIG_PATH);
    PASS();
}

void test_file_parsing(void) {
    TEST("INI file parsing");
    
    // Create a test config file
    FILE* fp = fopen(TEST_CONFIG_PATH, "w");
    if (!fp) {
        FAIL("could not create test file");
        return;
    }
    fprintf(fp, "[radio]\n");
    fprintf(fp, "model = 1234\n");
    fprintf(fp, "device = /dev/ttyACM0\n");
    fprintf(fp, "baud = 38400\n\n");
    fprintf(fp, "[audio]\n");
    fprintf(fp, "volume = 65\n");
    fprintf(fp, "speech_speed = 1.2\n");
    fprintf(fp, "key_beep = 0\n");
    fclose(fp);
    
    config_init(TEST_CONFIG_PATH);
    
    if (config_get_radio_model() != 1234) {
        FAIL("radio_model not parsed");
        config_cleanup();
        unlink(TEST_CONFIG_PATH);
        return;
    }
    if (strcmp(config_get_radio_device(), "/dev/ttyACM0") != 0) {
        FAIL("radio_device not parsed");
        config_cleanup();
        unlink(TEST_CONFIG_PATH);
        return;
    }
    if (config_get_radio_baud() != 38400) {
        FAIL("radio_baud not parsed");
        config_cleanup();
        unlink(TEST_CONFIG_PATH);
        return;
    }
    if (config_get_volume() != 65) {
        FAIL("volume not parsed");
        config_cleanup();
        unlink(TEST_CONFIG_PATH);
        return;
    }
    if (config_get_key_beep_enabled() != false) {
        FAIL("key_beep not parsed");
        config_cleanup();
        unlink(TEST_CONFIG_PATH);
        return;
    }
    
    config_cleanup();
    unlink(TEST_CONFIG_PATH);
    PASS();
}

// ============================================================================
// Main
// ============================================================================

int main(void) {
    printf("\n=== Config Module Unit Tests ===\n\n");
    
    test_init_defaults();
    test_setters_update_values();
    test_auto_save();
    test_undo_single();
    test_undo_count();
    test_undo_max_depth();
    test_value_clamping();
    test_file_parsing();
    
    printf("\n=== Results ===\n");
    printf("Passed: %d\n", tests_passed);
    printf("Failed: %d\n", tests_failed);
    
    return tests_failed > 0 ? 1 : 0;
}
