#ifndef HAMPOD_TESTING_CONFIG
#define HAMPOD_TESTING_CONFIG

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include "../GeneralFunctions.h"
#include "../ConfigSettings/ConfigParams.h"
#include "../APIObjects/HashMap.h"

//I also will need to make a simple hashing function
int TestHash(void* input);
bool TestCompare(void* a, void* b);
void TestFreeHash(void* a);
bool TestCreateDestroyHash();
bool TestCreateHash();
bool TestInsertHash();
bool TestGetHash();
bool TestRemoveHash();
bool TestGrowHash();

bool TestLoadUnloadConfig();
bool TestLoad();//this needs to be ran before the rest
bool TestGetConfig();
bool TestGetConfigLength();
bool TestGetCurrentValues();
bool TestSetCurrentValues();
bool TestInrementValue();
bool TestUpdateConfig();
#include "UnitTestConfigs.c"
#endif