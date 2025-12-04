#include <stdlib.h>
#include "ThreadQueue.h"


Thread_queue* createThreadQueue(){
        Thread_queue* new = malloc(sizeof(Thread_queue));
        if(!new) {
            perror("Queue memory allocation failed");
            exit(1);
        }
        new->head =  NULL;
        new->tail = NULL;
        return new;
}
void Threadenqueue(Thread_queue* queue, pthread_t thread){
    ThreadNode* new_node = malloc(sizeof(ThreadNode));
    if(!new_node) {
        perror("Node memory allocation failed");
        exit(1);
    }
    //this is to copy the value so it does not get lost
    // memcpy((void*) new_node->thread, (void*) thread, sizeof(pthread_t));
    new_node->thread = thread;
    new_node->next = NULL;

    if(queue->tail == NULL) {
        queue->head = new_node;
        queue->tail = new_node;
    } else {
        queue->tail->next = new_node;
        queue->tail = new_node;
    }
}
pthread_t ThreadDequeue(Thread_queue* queue){
      if(queue->head == NULL) {
        return (pthread_t) NULL;
    }

    ThreadNode* removed_node = queue->head;
    pthread_t ithread = removed_node->thread;
    queue->head = removed_node->next;

    if(queue->head == NULL) {
        queue->tail = NULL;
    }
    free(removed_node);
    return ithread;
}
void destroyThreadQueue(Thread_queue* queue){
     while(queue->tail != NULL) {
        ThreadDequeue(queue);
    }
    free(queue);
}

bool ThreadQueueIsEmpty(Thread_queue* queue){
    if(queue->head == NULL){
        return true;
    }else{
        return false;
    }
}