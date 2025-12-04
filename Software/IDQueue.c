#include <stdlib.h>
#include "IDQueue.h"


ID_queue* create_IDqueue(){
    ID_queue* new = malloc(sizeof(ID_queue));
        if(!new) {
            perror("Queue memory allocation failed");
            exit(1);
        }
        new->head = NULL;
        new->tail = NULL;
        return new;
}
void IDenqueue(ID_queue* queue, int value){
    IDNode* new_node = malloc(sizeof(IDNode));
    if(!new_node) {
        perror("Node memory allocation failed");
        exit(1);
    }
    new_node->id = value;
    new_node->next = NULL;

    if(queue->tail == NULL) {
        queue->head = new_node;
        queue->tail = new_node;
    } else {
        queue->tail->next = new_node;
        queue->tail = new_node;
    }
}
/**
 * -1 means that nothing was in the queue
*/
int IDpeek(ID_queue* queue){
    if(queue->head == NULL){
        return -1;
    }
    return queue->head->id;
}
int IDdequeue(ID_queue* queue){
    if(queue->head == NULL) {
        return -1;
    }

    IDNode* removed_node = queue->head;
    int id = removed_node->id;
    queue->head = removed_node->next;

    if(queue->head == NULL) {
        queue->tail = NULL;
    }
    free(removed_node);
    return id;
}
void destroy_IDqueue(ID_queue* queue){
     while(queue->tail != NULL) {
        IDdequeue(queue);
    }
    free(queue);
}

bool IDcontains(ID_queue* queue, int value){
    IDNode* here = queue->head;
    while(here != NULL){
        if(here->id == value){
            return true;
        }else{
            here = here->next;
        }
    }
    return false;
}