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

#ifndef HAMPOD_SOFT_CONFIGPARAMS
#define HAMPOD_SOFT_CONFIGPARAMS

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include "../GeneralFunctions.h"
#include "../APIObjects/HashMap.h"





//for loading in configs
void loadConfigParams();
ConfigType stringToConfigType(char* str);
char** convertTocharArray(char* str, int size);
//for writing to Configs
int writeConfigParamsToFile(const char *filename);
//for the hashmap
void freeConfigParam(void* param);
int ConfigHashing(void* key);
bool ConfigCompare(void* data, void* key);
void freeCongigFull();
//for accsessing configs
ConfigParam* getConfigByName(char* name);
char** getListOfConfigNames();
int getLengthOfConfigs();
double* getListOfCurrentValues();
void setListOfcurrentValues(double* values);
char* incrementConfig(char* name, bool up);
char* updateConfigs(char* name, double value);

#ifndef SHAREDLIB
#include "ConfigParams.c"
#endif
#endif
