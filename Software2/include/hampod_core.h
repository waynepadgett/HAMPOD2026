#ifndef HAMPOD_CORE_H
#define HAMPOD_CORE_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

// Debug levels
#define DEBUG_NONE 0
#define DEBUG_ERROR 1
#define DEBUG_INFO  2
#define DEBUG_ALL   3

// Current debug level
#ifndef DEBUG_LEVEL
#define DEBUG_LEVEL DEBUG_INFO
#endif

// Debug macros
#if DEBUG_LEVEL >= DEBUG_ERROR
    #define LOG_ERROR(fmt, ...) fprintf(stderr, "[ERROR] " fmt "\n", ##__VA_ARGS__)
#else
    #define LOG_ERROR(fmt, ...)
#endif

#if DEBUG_LEVEL >= DEBUG_INFO
    #define LOG_INFO(fmt, ...) printf("[INFO] " fmt "\n", ##__VA_ARGS__)
#else
    #define LOG_INFO(fmt, ...)
#endif

// KeyPress Structure (Reused concept from old code)
typedef struct {
    char key;           // The character pressed (e.g., '1', 'A', '#')
    int shiftAmount;    // 0 = normal, 1+ = shifted (if applicable)
    bool isHold;        // true if this is a long press
} KeyPressEvent;

#endif // HAMPOD_CORE_H
