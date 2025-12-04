#include <stdlib.h>
#include <string.h>
#include "hampod_firm_packet.h"

Inst_packet* create_inst_packet(Packet_type new_type, unsigned short new_len, unsigned char *new_data, unsigned short tag) {
    Inst_packet *new = malloc(sizeof(Inst_packet));
    new->type = new_type;
    new->data_len = new_len;
    new->tag = tag;
    new->data = malloc(new_len);
    memcpy(new->data, new_data, new_len);
    return new;
}

void destroy_inst_packet(Inst_packet** packet){
    Inst_packet* temp = *packet;
    free(temp->data);
    free(*packet);
    *packet = 0;
}
