#ifndef HAMPOD_SOFT_LLIST
#define HAMPOD_SOFT_LLIST

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
#include "../GeneralFunctions.h"

typedef struct ListNode{
    void* data;
    struct ListNode* next;
    struct ListNode* prior;
}ListNode;

typedef struct LinkedList {
    ListNode* head;
    ListNode* tail;
} LinkedList;

LinkedList* createLinkedList();
void destroyLinkedList(LinkedList* lList,void (*destroyFunction)(void*));
void llAddHead(LinkedList* lList,void* ldata);
void llAddTail(LinkedList* lList,void* ldata);
void llAddAfterNode(LinkedList* lList, ListNode* node, void* ldata);
void* llRemoveHead(LinkedList* lList);
void* llRemoveTail(LinkedList* lList);
void* llRemoveNode(LinkedList* lList, ListNode* node);
bool llIsEmpty(LinkedList* lList);

ListNode* createListNode(void* ldata);
void* destroyListNode(ListNode* node);
#ifndef SHAREDLIB
#include "LinkedList.c"
#endif
#endif