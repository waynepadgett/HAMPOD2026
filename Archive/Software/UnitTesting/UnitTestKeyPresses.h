#ifndef HAMPOD_TESTING_KEY
#define HAMPOD_TESTING_KEY

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include "UnitTestKeyPresses.h"
#include "../GeneralFunctions.h"
#include "../FirmwareCommunication.h"
bool fullTest();
bool compareKeyInputs(KeyPress* correct, KeyPress* testing);
bool testKeyPadEntering();

#include "UnitTestKeyPresses.c"
#endif