#ifndef HAMPOD_TESTING_LINKEDLIST
#define HAMPOD_TESTING_LINKEDLIST

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
#include "../APIObjects/LinkedList.h"

bool testCreateDestroyLList();
bool testAddToLList();
bool testRemoveFromLList();
bool testDestroyFilledList();
void destroyDataLList(void* data);

#include "UnitTestLinkedList.c"
#endif