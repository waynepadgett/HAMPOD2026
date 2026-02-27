#include "Monitoring.h"
LinkedList* monitoringList;
pthread_mutex_t linkedListLock;
pthread_t monitoringThread;
bool monitoringRunning = true;
bool monitoringFinished = false;
void startMonitoringLoop(){
    monitoringList = createLinkedList();
    if(pthread_mutex_init(&linkedListLock, NULL) != 0) {
        perror("linkedListLock");
        exit(1);
    }
    monitoringRunning = true;
    monitoringFinished = false;
    //start up a thread
    int result = pthread_create(&monitoringThread, NULL, monitoringLoop, NULL);
}
void* monitoringLoop(void* d){
    pthread_mutex_lock(&linkedListLock);
    ListNode* current = monitoringList->head;
    pthread_mutex_unlock(&linkedListLock);
    while(monitoringRunning){
        if(current == NULL){
            pthread_mutex_lock(&linkedListLock);
            current = monitoringList->head;
            pthread_mutex_unlock(&linkedListLock);
            usleep(10000);
            continue;
        }
        // PRINTFLEVEL2("there is something to me monitored\n");
        MonitoringLink* linkData = (MonitoringLink*) current->data;
        // PRINTFLEVEL2("The old data is:%s\n", linkData->oldData);
        if(linkData->flaggedForDeletion){
            //delete it
            pthread_mutex_lock(&linkedListLock);
            ListNode* nextThing = current->next;
            llRemoveNode(monitoringList,current);
            current = nextThing;
            pthread_mutex_unlock(&linkedListLock);
        }else{
            char* newData = linkData->getData(linkData->callData);
              PRINTFLEVEL2("Old:%s\nNew:%s\ncomparison value:%d\n",linkData->oldData,newData,strcmp(newData,linkData->oldData));
            if(strcmp(newData,linkData->oldData) != 0){
                sendSpeakerOutputWithConditions(newData, false, true, true);
                free(linkData->oldData);
                linkData->oldData = newData;
                PRINTFLEVEL1("Updating the old data to be new data\n");
            }else{
                PRINTFLEVEL2("Nothing changed to freeing new data\n");
                free(newData);
            }
            pthread_mutex_lock(&linkedListLock);
            current = current->next;
            pthread_mutex_unlock(&linkedListLock);
        }
    }
    monitoringFinished = true;
    return NULL;
}

void destroyMonitoringNodes(void* dataIn){
    MonitoringLink* linkData = (MonitoringLink*) dataIn;
    free(linkData->oldData);
    free(linkData->callData);
    free(linkData);
}
//WARNING THIS IS LOSSY IN ITS CURRENT FORM
void endMonitoringLoop(){
    monitoringRunning = false;
    while(!monitoringFinished){
        usleep(10000);
    }
    pthread_mutex_destroy(&linkedListLock);
    destroyLinkedList(monitoringList,destroyMonitoringNodes);
}

/**
 * Monitorying wraper function need to return a malloced char* pointer
*/
void addMonitoringLink(char* (*getData)(void*), void* callData){
    MonitoringLink* newLinkData = malloc(sizeof(MonitoringLink));
    // PRINTFLEVEL1("Populating the new link data with input paremiters\n");
    newLinkData->getData = getData;
    newLinkData->callData = callData;
    newLinkData->oldData = malloc(sizeof(char)*5);
    sprintf(newLinkData->oldData,"");
    newLinkData->flaggedForDeletion = false;
    // PRINTFLEVEL2("adding the new link to the monitoring chain\n");
    pthread_mutex_lock(&linkedListLock);
    llAddTail(monitoringList,(void*)newLinkData);
    pthread_mutex_unlock(&linkedListLock);
    // PRINTFLEVEL2("New has been added\n");

}
void* removeMonitoringLink(char* (*getData)(void*)){
    pthread_mutex_lock(&linkedListLock);
    ListNode* current = monitoringList->head;
    while(current != NULL){
        MonitoringLink* linkData = (MonitoringLink*) current->data;
        if(linkData->getData == getData){
            linkData->flaggedForDeletion = true;
            pthread_mutex_unlock(&linkedListLock);
            return linkData->callData;
        }
        current = current->next;
    }
    pthread_mutex_unlock(&linkedListLock);
    return NULL;
}
