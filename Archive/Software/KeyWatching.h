#ifndef HAMPOD_ERROROUTPUT
#define HAMPOD_ERROROUTPUT

    #ifdef OUTPUTLEVEL1
    #define PRINTFLEVEL1(...) \
        do { \
            if(OUTPUTLEVEL1) { \
                printf(__VA_ARGS__); \
            } \
        } while(0)
    #else

    #define PRINTFLEVEL1(...) \
    do{}while(0)

    #endif

    #ifdef OUTPUTLEVEL2
    #define PRINTFLEVEL2(...) \
        do { \
            if(OUTPUTLEVEL1) { \
                printf(__VA_ARGS__); \
            } \
        } while(0)
    #else

    #define PRINTFLEVEL2(...) \
    do{}while(0)

    #endif
#endif

#ifndef HAMPOD_SOFT_KeyWAtch
#define HAMPOD_SOFT_KeyWAtch
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <stdbool.h>
#include <string.h>
#include "../Firmware/hampod_firm_packet.h"
#include "FirmwareCommunication.h"
#include "StateMachine.h"
#include "../Firmware/keypad_firmware.h"


void* keyWatcher(void* args);

KeyPress* interperateKeyPresses(char keyPress);
void resetKeyInputVars();
#ifndef SHAREDLIB
#include "KeyWatching.c"
#endif
#endif