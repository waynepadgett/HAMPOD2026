#ifndef HAMPOD_SOFT_DUMMYDTMF
#define HAMPOD_SOFT_DUMMYDTMF
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <hamlib/rig.h>
#include <hamlib/riglist.h>
#include "Mode.h"
#include "../GeneralFunctions.h"

#ifndef SIMULATEOUTPUT
    #ifdef TESTING
        #define SIMULATEOUTPUT 1
    #else
        #define SIMULATEOUTPUT 0
    #endif
#endif


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

Mode* DummyDTMFLoad();
#endif