#ifndef HAMPOD_TESTING_MODEROUTING
#define HAMPOD_TESTING_MODEROUTING
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include "../ModeRouting.h"
#include "../Modes/Mode.h"
//TODO make the connected c file

bool testCreateModeRoutingFunction();
bool testGettingOneMode();
bool testGettingManyModes();
bool testGettingSameMode();
bool testFreeModes();
bool testGrabEachMode();

#include "UnitTestModeRouting.c"
#endif