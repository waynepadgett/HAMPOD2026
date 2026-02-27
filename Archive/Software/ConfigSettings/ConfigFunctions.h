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

#ifndef HAMPOD_SOFT_CONFIGFUNCTIONS
#define HAMPOD_SOFT_CONFIGFUNCTIONS

#ifndef SIMULATEOUTPUT
#ifdef TESTING
#define SIMULATEOUTPUT 1
#else
#define SIMULATEOUTPUT 0
#endif
#endif


#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include "../GeneralFunctions.h"
#include "../APIObjects/HashMap.h"
#include "ConfigParams.h"
#include "../ModeRouting.h"
#include "../StateMachine.h"
#include "../FirmwareCommunication.h"
#include "../Modes/Mode.h"
#include "../Radio.h"

void populateConfigFunctions();

void saveToFile(int fileNumber);
int SaveData(KeyPress* keyData);

int setHotkeys(KeyPress* keyData);

#ifndef SHAREDLIB
#include "ConfigFunctions.c"
#endif
#endif
