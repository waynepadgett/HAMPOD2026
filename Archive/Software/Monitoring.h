#ifndef HAMPOD_SOFT_MONITORING
#define HAMPOD_SOFT_MONITORING


#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <stdbool.h>
#include <string.h>
#include <dirent.h> 
#include <sys/ioctl.h>
#include <asm/termbits.h>
#include <stdarg.h>

#include "GeneralFunctions.h"
#include "APIObjects/LinkedList.h"
#include "FirmwareCommunication.h"


#define INPUT_PIPE "Firmware_i"
#define OUTPUT_PIPE "Firmware_o"

#ifdef TESTING
    #define SIMULATEOUTPUT 1
#else
    #define SIMULATEOUTPUT 0
#endif

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


typedef struct MonitoringLink{
    char* (*getData)(void*);
    void* callData;
    char* oldData;
    bool flaggedForDeletion;
} MonitoringLink;


void startMonitoringLoop();
void* monitoringLoop(void* d);
void endMonitoringLoop();

void addMonitoringLink(char* (*getData)(void*), void* callData);
void* removeMonitoringLink(char* (*getData)(void*));
#ifndef SHAREDLIB
#include "Monitoring.c"
#endif

#endif