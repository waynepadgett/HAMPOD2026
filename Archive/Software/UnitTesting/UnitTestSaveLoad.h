#ifndef HAMPOD_TESTING_SAVELOAD
#define HAMPOD_TESTING_SAVELOAD

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include "../GeneralFunctions.h"
#include "../ConfigSettings/ConfigParams.h"
#include "../ConfigSettings/ConfigFunctions.h"
#include "../ModeRouting.h"
#include "../Radio.h"
#include "../StateMachine.h"
#include "../ConfigSettings/ConfigLoad.h"
void saveLoadSetupStuffToSave();
bool testSaveFile();
void setUpForLoadTesting();
bool testLoadFile();

#include "UnitTestSaveLoad.c"
#endif