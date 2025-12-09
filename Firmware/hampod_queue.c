#include <stdio.h>
#include <stdlib.h>

#include "hampod_firm_packet.h"
#include "hampod_queue.h"

Packet_queue* create_packet_queue() {
    Packet_queue* new = malloc(sizeof(Packet_queue));
    if(!new) {
        perror("Queue memory allocation failed");
        exit(1);
    }
    new->head = NULL;
    new->tail = NULL;
    return new;
}

void enqueue(Packet_queue* queue, Inst_packet *packet) {
    Node* new_node = malloc(sizeof(Node));
    if(!new_node) {
        perror("Node memory allocation failed");
        exit(1);
    }
    new_node->packet = packet;
    new_node->next = NULL;

    if(queue->tail == NULL) {
        queue->head = new_node;
        queue->tail = new_node;
    } else {
        queue->tail->next = new_node;
        queue->tail = new_node;
    }
}

Inst_packet* dequeue(Packet_queue* queue) {
    if(queue->head == NULL) {
        return NULL;
    }

    Node* removed_node = queue->head;
    Inst_packet* packet = removed_node->packet;
    queue->head = removed_node->next;

    if(queue->head == NULL) {
        queue->tail = NULL;
    }
    free(removed_node);
    return packet;
}

void destroy_queue(Packet_queue* queue) {
    while(queue->tail != NULL) {
        Inst_packet* temp = dequeue(queue);
        destroy_inst_packet(&temp);
    }
    free(queue);
}

int is_empty(Packet_queue* queue){
    return queue->head == NULL;
}
