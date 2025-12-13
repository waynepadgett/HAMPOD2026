#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>

#include "hampod_firm_packet.h"

#define INPUT_PIPE "Firmware_i"
#define OUTPUT_PIPE "Firmware_o"

const char* keypad_names[] = {
    "1", "2", "3", "A", "4", "5", "6", "B", "7", "8", "9", "C", "POINT", "0", "POUND", "D"
};
const char* DTMF_names[] = {
    "DTMF1", "DTMF2", "DTMF3", "DTMFA", "DTMF4", "DTMF5", "DTMF6", "DTMFB", "DTMF7", "DTMF8", 
    "DTMF9", "DTMFC", "DTMFASTERISK", "DTMF0", "DTMFPOUND", "DTMFD"
};

int index_getter(char keypad_input){
    int index = 0;
    if(keypad_input >= '0' && keypad_input <= '9') {
        return keypad_input - '0'; //UnASCII the input
    }
    if(keypad_input >= 'A' && keypad_input <= 'D') {
        return keypad_input - '1'; //Now THIS is unmaintainable code
    }
    if(keypad_input == '#') {
        return 14;
    }
    if(keypad_input == '*') {
        return 15;
    }
    return -1;
}
int input_pipe;
int output_pipe;
void send_packet(Inst_packet* packet){
    printf("Message = %s\n", packet->data);
    
    // Serialize header explicitly to match Firmware reader
    write(output_pipe, &packet->type, sizeof(Packet_type));
    write(output_pipe, &packet->data_len, sizeof(unsigned short));
    write(output_pipe, &packet->tag, sizeof(unsigned short));

    unsigned char buffer[256];
    memcpy(buffer, packet->data, packet->data_len);
    printf("Message = %s\n", (char*)buffer);
    write(output_pipe, buffer, packet->data_len);
}

Inst_packet* read_from_pipe(){
    unsigned char buffer[256];
    Packet_type type;
    unsigned short size;
    unsigned short tag;
    read(input_pipe, &type, sizeof(Packet_type));
    read(input_pipe, &size, sizeof(unsigned short));
    read(input_pipe, &tag, sizeof(unsigned short));
    read(input_pipe, buffer, size);
    Inst_packet* temp = create_inst_packet(type, size, buffer, tag);
    return temp;
}

int main(){
    printf("Hampod Software Emulation Tool For interacting with the hardware\n");
    printf("Size of type = %d\n", sizeof(Packet_type));
    printf("Connecting to Firmware_o\n");
    input_pipe = open(OUTPUT_PIPE, O_RDONLY);
    if(input_pipe == -1){
        perror("open");
        exit(-1);
    }
    printf("Attempting to connect to Firmware_i\n");
    output_pipe;
    for(int i = 0; i < 1000; i++){
        output_pipe = open(INPUT_PIPE, O_WRONLY);
        printf("Attempt %d/1000\r", i);
        if(output_pipe != -1){
            break;
        }
        usleep(10000); // Wait 10ms between attempts
    }
    if(output_pipe == -1){
        printf("\nUnsuccessful Connection\n");
        perror("open");
        exit(1);
    }
    printf("\nSuccessful connection to Firmware_i\n");
    while(1) {
        printf("Packet attempt\n");
        int mode = 0; //keypad = 0: DTMF = 1
        char new_data = AUDIO;
        Packet_type keypad = KEYPAD;
        char keypad_msg = 'r';
        char len = 1;
        char msg[] = "sThis is a keypad and audio integration test for the firmware. Press * to toggle DTMF mode.";
        Inst_packet* temp = create_inst_packet(new_data, strlen(msg) + 1, msg, 0);
        send_packet(temp);
        Inst_packet* audio_packet = read_from_pipe();
        destroy_inst_packet(&audio_packet);
        while(1) { //sue me
            temp = create_inst_packet(keypad, len, &keypad_msg, 0);
            send_packet(temp);
            destroy_inst_packet(&temp);
            Inst_packet* keypad_packet = read_from_pipe();
            if(keypad_packet->type != KEYPAD) {
                printf("bruh\n");
                destroy_inst_packet(&keypad_packet);
                continue;
            }
            char keypad_moment = keypad_packet->data[0];
            printf("keypad says %x (%c)\n", keypad_moment, keypad_moment);
            
            if(keypad_packet->data[0] == 12){
                mode ^= 1;
                destroy_inst_packet(&keypad_packet);
                continue;
            }
            if(keypad_moment == 0xFF || keypad_moment == '-'){
                destroy_inst_packet(&keypad_packet);
                continue;
            }
            
            char buffer[256];
            if(mode == 0) {
                printf("Not DTMF\n");
                strcpy(buffer, "ppregen_audio/");
                strcat(buffer, keypad_names[keypad_moment]);
                temp = create_inst_packet(new_data, strlen(buffer) + 1, buffer, 0);
                send_packet(temp);
                destroy_inst_packet(&temp);
                audio_packet = read_from_pipe();
                destroy_inst_packet(&audio_packet);
            } else {
                printf("DTMF\n");
                strcpy(buffer, "ppregen_audio/");
                strcat(buffer, DTMF_names[keypad_moment]);
                temp = create_inst_packet(new_data, strlen(buffer) + 1, buffer, 0);
                send_packet(temp);
                destroy_inst_packet(&temp);
                audio_packet = read_from_pipe();
                destroy_inst_packet(&audio_packet);
            }
            usleep(16670);
        }
        
    }
}
