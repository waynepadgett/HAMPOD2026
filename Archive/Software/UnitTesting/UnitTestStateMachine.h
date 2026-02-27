#ifndef HAMPOD_TESTING_STATE
#define HAMPOD_TESTING_STATE
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include "../GeneralFunctions.h"
#include "../StateMachine.h"
#include "../Radio.h"
bool TestStateMachingBadInputSetFrequ();
bool TestStateMachingGoodInputSetFrequ();

#include "UnitTestStateMachine.c"
#endif