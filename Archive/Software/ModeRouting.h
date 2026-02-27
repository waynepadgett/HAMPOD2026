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

#ifndef HAMPOD_SOFT_MODE_ROUTING
#define HAMPOD_SOFT_MODE_ROUTING
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <dlfcn.h>
#include <dirent.h>
#include "Modes/Mode.h"
#include "GeneralFunctions.h"
#include "APIObjects/HashMap.h"


// Mode* getModeById(int modeID); //goal is to make this outdated Done
void modeRoutingStart(); //change to this return type Done
Mode* dynamicalyLoadInModeByName(char* name);
char** getNamesOfLoadFunctions();
void freeModesLambda(void* data);
void freeModes();

Mode* getModeByName(char* name);//new
Mode** getAllModes();//new
char** getAllModeNames();
int getModeCount();

void setProgramibleKeys(KeyPress* key, char* name); //make this use the name and not the ID
Mode* getModeViaProgramableKey(KeyPress* key);
void setProgramibleKeysByIndex(int index, char* name);
Mode** getHotKeyList();

#ifndef SHAREDLIB
#include "ModeRouting.c"
#endif
#endif
