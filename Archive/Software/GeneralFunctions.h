#ifndef HAMPOD_SOFT_GEN
#define HAMPOD_SOFT_GEN
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
typedef struct KeyPress{
    char keyPressed;
    int shiftAmount;
    bool isHold;
} KeyPress;

typedef enum {
    ONOFF,
    NUMERIC,
    ONOFFNUMERIC,
    SELECTION,
    OTHER,
    NUMPAD
} ConfigType;

typedef struct ConfigParam{
    char* name; 
    double defaultValue;
    double minValue;
    double maxValue;
    double stepSize;
    char** selectionSet;
    int selectionSize;
    int (*configFuntion)(KeyPress*);
    ConfigType configType;
    double currentValue;
    char* startingDescription;
} ConfigParam;

int convertCharToKeyValue(KeyPress* keyPressed);

char** textFileToArray(char* filePath);

void freeFileArray(char** list);

//TODO make simple testing code for this
char* customSubString(char* original, int start, int length);

char* replaceSubstring(char* original, char* old, char* new);

double keypadInput(KeyPress* keyInput);

bool getABState();
bool getCDState();
void setABstate(bool state);
void setCDstate(bool state);
#ifndef SHAREDLIB
#include "GeneralFunctions.c"
#endif

#endif

//
