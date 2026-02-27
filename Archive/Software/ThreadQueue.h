#ifndef HAMPOD_Software_ThreadQUEUE
#define HAMPOD_Software_ThreadQUEUE
#include <pthread.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

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

typedef struct ThreadNode {
    pthread_t thread; 
    struct ThreadNode* next;
} ThreadNode;

typedef struct Thread_queue {
    ThreadNode* head;
    ThreadNode* tail;
} Thread_queue;

void Threadenqueue(Thread_queue* queue, pthread_t thread);
Thread_queue* createThreadQueue();
pthread_t ThreadDequeue(Thread_queue* queue);
void destroyThreadQueue(Thread_queue* queue);
bool ThreadQueueIsEmpty(Thread_queue* queue);

#ifndef SHAREDLIB
#include "ThreadQueue.c"
#endif
#endif
