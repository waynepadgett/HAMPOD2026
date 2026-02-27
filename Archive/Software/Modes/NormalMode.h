#ifndef HAMPOD_SOFT_NORMAL
#define HAMPOD_SOFT_NORMAL
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <hamlib/rig.h>
#include <hamlib/riglist.h>
#include "Mode.h"
#include "../GeneralFunctions.h"
#include "../FirmwareCommunication.h"

#include "../HamlibWrappedFunctions/HamlibGetFunctions.h"
#include "../HamlibWrappedFunctions/HamlibSetFunctions.h"

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

Mode* NormalLoad();


#endif