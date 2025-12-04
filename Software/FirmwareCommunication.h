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

#ifndef HAMPOD_SOFT_FIRMCOMMUN
#define HAMPOD_SOFT_FIRMCOMMUN


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
#include "APIObjects/HashMap.h"
#include "../Firmware/audio_firmware.h"
#include "../Firmware/hampod_firm_packet.h"


#define INPUT_PIPE "../Firmware/Firmware_i"
#define OUTPUT_PIPE "../Firmware/Firmware_o"

#ifdef TESTING
    #define SIMULATEOUTPUT 1
#else
    #define SIMULATEOUTPUT 0
#endif



char* sendSpeakerOutput(char* text);
char* sendSpeakerOutputWithConditions(char* textIn, bool filterBypass, bool verbosityBypass, bool linearCall);
void setupAudioHashMap();
int audioHash(void* key);
bool audioCompare(void* key1, void* key2);
void audioFree(void* data);
bool shouldCreateAudioFile(char* textIn);

void firmwareCommunicationStartup();

void freeFirmwareComunication();

void setupDictinaryHashMap();
char* applyDictionary(char* s);
#ifndef SHAREDLIB
#include "FirmwareCommunication.c"
#endif
#endif

