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

#ifndef HAMPOD_SOFT_RADIO
#define HAMPOD_SOFT_RADIO

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
#include <hamlib/rig.h>
#include <hamlib/riglist.h>
#include "Modes/Mode.h"
#include "GeneralFunctions.h"

//toy with the idea of the radio struct holding function pointers so they are more like objects
typedef struct Radio{
    char* make;
    int model;
    int port;
    Mode* currentMode;
    rig_model_t myrig_model; 
    RIG *my_rig; 
} Radio;

void startRadios();
void loadUpRadioUsingData(char* make, int model, int port, Mode* defaultMode, rig_model_t myrig_model);
void freeRadios();
Mode* getCurrentMode();
ModeData* getModeDetails();
void setRadioMode(Mode* modeToSetTo);
void* runRadioCommand(KeyPress* keyInput);

void setCurrentRadio(int radioID);
int getCurrentRadioID();
int getRadioAmount();
Radio* getCurrentRadio();
Radio** getAllRadios();
#ifndef SHAREDLIB
#include "Radio.c"
#endif
#endif