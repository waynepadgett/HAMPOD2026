#ifndef HAMPOD_TESTING_MONITORING
#define HAMPOD_TESTING_MONITORING

#ifdef OUTPUTLEVEL1
#define PRINTFLEVEL1(...) \
    do { \
        if(OUTPUTLEVEL1) { \
            printf(__VA_ARGS__); \
        } \
    } while(0)
#else

#define PRINTFLEVEL1(...) \
    while(0)

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
    while(0)

#endif

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include "../Monitoring.h"

char* dummyGetWraper1(void* input);
char* dummyGetWraper2(void* input);
bool testCreateDestroyMonitoring();
bool testAddingMonitoringComponent();
//remove test
bool testRemovingMonitoringComponent();
//see if it will monitor changes and output them
bool testMonitoringAfterChanges();

void resetMonitoringWrappers();


#include "UnitTestMonitoring.c"
#endif