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

#ifndef HAMPOD_SOFT_STATE
#define HAMPOD_SOFT_STATE

#ifndef TESTINGOUTPUT
#ifdef TESTING
#define TESTINGOUTPUT 1
#else
#define TESTINGOUTPUT 0
#endif
#endif
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <hamlib/rig.h>
#include <hamlib/riglist.h>
#include "ModeRouting.h"
#include "GeneralFunctions.h"
#include "Radio.h"
#include "Modes/Mode.h"
#include "FirmwareCommunication.h"
#include "APIObjects/HashMap.h"
#include "ConfigSettings/ConfigParams.h"
typedef enum {
    bootUp,
    standard,
    modeSelect
} ModeStates;

typedef enum {
    selectNewOrSave,
    chooseCompany,
    chooseModel,
    selectLink,
    linkMore,
    selectSave
} BootUpStates;


/**
 * Each of the returns is just for unit testing sake,
 * they return an identifyer depending on which path they took
*/
ModeStates modeFlow(KeyPress* keyInput);
BootUpStates BootupFlow(KeyPress* keyInput);

int ModeSelectFlow(KeyPress* keyInput);
int switchToRadioMode(char* modeName);
int StandardModeFlow(KeyPress* keyInput);
int readOutModeName(char* modeName);
void stateMachineStart();

void freeStateMachine();
void toggleCDHotkeys(bool state);

/**
 * These functions are for testing purposes only
*/
void setModeState(ModeStates state);
void setBootUpState(BootUpStates state);
void setCompanyAndModel(char* comp, int mod);

int selectEntryInList(KeyPress* keyInput, char** list);

bool loadUpFromSave(int saveFileNumber);

#ifndef SHAREDLIB
#include "StateMachine.c"
#endif
#endif