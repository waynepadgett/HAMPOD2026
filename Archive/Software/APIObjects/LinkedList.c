#include "LinkedList.h"
#include "LinkedList.h"
LinkedList* createLinkedList(){
    LinkedList* lList = malloc(sizeof(LinkedList));
    lList->head = NULL;
    lList->tail = NULL;
    return lList;
}
void destroyLinkedList(LinkedList* lList, void (*destroyFunction)(void*)){
    while(lList->head != NULL){
        PRINTFLEVEL2("removing item from list while destroying: String value is %s\n",lList->head->data);
        destroyFunction(llRemoveHead(lList));
    }
    free(lList);
}
void llAddHead(LinkedList* lList,void* ldata){
    if(llIsEmpty(lList)){
    //where it is empty
        lList->head = createListNode(ldata);
        lList->tail = lList->head;
    }else{
    //where it is not empty
        ListNode* oldHead = lList->head;
        ListNode* newHead = createListNode(ldata);
        oldHead->prior = newHead;
        newHead->next = oldHead;
        lList->head = newHead;
    }
}
void llAddTail(LinkedList* lList,void* ldata){
    if(llIsEmpty(lList)){
    //where it is empty
        PRINTFLEVEL1("the list is empty so adding new stuff to it\n");
        lList->head = createListNode(ldata);
        // PRINTFLEVEL2("head populated\n");
        lList->tail = lList->head;
        // PRINTFLEVEL2("tail populated\n");
    }else{
    //where it is not empty
        ListNode* oldTail = lList->tail;
        ListNode* newTail = createListNode(ldata);
        oldTail->next = newTail;
        newTail->prior = oldTail;
        lList->tail = newTail;
    }
}

//assumes that a linked list is already created
void llAddAfterNode(LinkedList* lList, ListNode* node, void* ldata){
    ListNode* newNode = createListNode(ldata);
    newNode->next = node->next;
    newNode->prior = node;
    node->next = newNode;
    if(newNode->next == NULL){
        lList->tail = newNode;
    }else{
        newNode->next->prior = newNode;
    }
}

void* llRemoveHead(LinkedList* lList){
    if(llIsEmpty(lList)){
        return (ListNode*) NULL;
    }
    ListNode* oldNode = lList->head;
    lList->head = lList->head->next;
    if(lList->head == NULL){
        lList->tail = NULL;
    }else{
        lList->head->prior = NULL;
    }
    return destroyListNode(oldNode);
}
void* llRemoveTail(LinkedList* lList){
    if(llIsEmpty(lList)){
        return (ListNode*) NULL;
    }
    ListNode* oldNode = lList->tail;
    lList->tail = oldNode->prior;
    if(lList->tail == NULL){
        lList->head = NULL;
    }else{
        lList->tail->next = NULL;
        oldNode->prior->next = NULL;
    }
    PRINTFLEVEL2("removed stufs memory location%s\n",oldNode);
    return destroyListNode(oldNode);
}
//assumes that this is part of a linked list
void* llRemoveNode(LinkedList* lList, ListNode* node){
    ListNode* priorNode = node->prior;
    ListNode* nextNode = node->next;
    //this is head
    
    if(priorNode == NULL && nextNode == NULL){
        lList->head = NULL;
        lList->tail = NULL;
       
    }else if(priorNode == NULL){
        lList->head = nextNode;
        nextNode->prior = NULL;
    }else if(nextNode == NULL){
        priorNode->next = NULL;
        lList->tail = priorNode;
    }else{
        priorNode->next = nextNode;
        nextNode->prior = priorNode;
    }
    return destroyListNode(node);
}
bool llIsEmpty(LinkedList* lList){
    if(lList->head == NULL && lList->tail == NULL){
        return true;
    }else if(lList->head == NULL || lList->tail == NULL){
        PRINTFLEVEL1("PRoblem with linklist, head and tail, one is null and the other is not\n");
        return true;
    }else{
        return false;
    }
}

ListNode* createListNode(void* ldata){
    ListNode* node = malloc(sizeof(ListNode));
    // PRINTFLEVEL2("populating new listNode with data\n");
    node->data = ldata;
    node->next = NULL;
    node->prior = NULL;
    // PRINTFLEVEL2("retirning new listNode\n");
    return node;
}
void* destroyListNode(ListNode* node){
    void* nData = node->data;
    free(node);
    return nData;
}
