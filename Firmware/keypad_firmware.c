/* This code is for the keypad firmware for the Hampod Program
* Written by Brendan Perez
* Last modified on 10/23/2023
* Updated for Raspberry Pi USB HAL on 11/28/2025
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>

#include "hampod_queue.h"
#include "hampod_firm_packet.h"
#include "hal/hal_keypad.h"

extern pid_t controller_pid;

unsigned char keypad_running = 1;

pthread_mutex_t keypad_queue_lock;
pthread_mutex_t keypad_queue_available;

void *keypad_io_thread(void* arg);

// Debug print statements from this process are White (\033[0;m)
void keypad_process(){

    KEYPAD_PRINTF("Keypad reader process launched\n");

    // Initialize HAL
    if (hal_keypad_init() != 0) {
        KEYPAD_PRINTF("Failed to initialize keypad HAL\n");
        // We continue anyway, maybe it will work later or user will plug it in
    } else {
        KEYPAD_PRINTF("Keypad HAL initialized: %s\n", hal_keypad_get_impl_name());
    }

    KEYPAD_PRINTF("Connecting to input/output pipes\n");


    int input_pipe_fd = open(KEYPAD_I, O_RDONLY);
    if(input_pipe_fd == -1) {
        perror("open");
        // kill(controller_pid, SIGINT);
        exit(0);
    }

    int output_pipe_fd = open(KEYPAD_O, O_WRONLY);
    if(output_pipe_fd == -1) {
        perror("open");
        // kill(controller_pid, SIGINT);
        exit(0);
    }
    
	KEYPAD_PRINTF("Pipes successfully connected\n");
    KEYPAD_PRINTF("Creating input queue\n");

    Packet_queue* input_queue = create_packet_queue();

    KEYPAD_PRINTF("Creating queue mutex lock\n");

    if(pthread_mutex_init(&keypad_queue_lock, NULL) != 0) {
        perror("pthread_mutex_init");
        // kill(controller_pid, SIGINT);
        exit(1);
    }
    KEYPAD_PRINTF("Creating queue availibility mutex lock\n");

    if(pthread_mutex_init(&keypad_queue_available, NULL) != 0) {
        perror("pthread_mutex_init");
        // kill(controller_pid, SIGINT);
        exit(1);
    }
    
    pthread_t keypad_io_buffer;

    keypad_io_packet thread_input;
    thread_input.pipe_fd = input_pipe_fd;
    thread_input.queue = input_queue;

    KEYPAD_PRINTF("Launching IO thread\n");

    if(pthread_create(&keypad_io_buffer, NULL, keypad_io_thread, (void*)&thread_input) != 0){
        perror("Keypad IO thread failed");
        // kill(controller_pid, SIGINT);
        exit(1);
    }
    usleep(500000); //Half sec sleep to let child thread take control
	while(keypad_running){
		pthread_mutex_lock(&keypad_queue_available);
        pthread_mutex_lock(&keypad_queue_lock);
        if(is_empty(input_queue)) {    
		    pthread_mutex_unlock(&keypad_queue_available);
            pthread_mutex_unlock(&keypad_queue_lock);
            usleep(500);
            continue;
        }
        Inst_packet* received_packet = dequeue(input_queue);
        pthread_mutex_unlock(&keypad_queue_lock);
        pthread_mutex_unlock(&keypad_queue_available);
        
        char read_value = -1;
        if (received_packet->data[0] == 'r') {
            // Use HAL to read keypad
            KeypadEvent event = hal_keypad_read();
            if (event.valid) {
                read_value = event.key;
            } else {
                read_value = '-'; // No key pressed
            }
        }

        Inst_packet* packet_to_send = create_inst_packet(KEYPAD, 1, (unsigned char*)&read_value, received_packet->tag);
        KEYPAD_PRINTF("Sending back value of %x ('%c')\n", read_value, (char) read_value);
        write(output_pipe_fd, packet_to_send, 8);
        write(output_pipe_fd, packet_to_send->data, 1);

        destroy_inst_packet(&packet_to_send);
	}
    pthread_join(keypad_io_buffer, NULL);
    destroy_queue(input_queue);
    close(input_pipe_fd);
    close(output_pipe_fd); //Graceful closing is always nice :)
    
    hal_keypad_cleanup();
    return;
}

//Debug print statements from this thread are Cyan (\033[0;96m)
void *keypad_io_thread(void* arg) {
    
    KEYPAD_IO_PRINTF("Keypad IO thread created\n");

    keypad_io_packet* new_packet =  (keypad_io_packet*)arg;
    int i_pipe = new_packet->pipe_fd;
    Packet_queue* queue = new_packet->queue;
    unsigned char buffer[256];

    KEYPAD_IO_PRINTF("Input pipe = %d, queue ptr = %p\n", i_pipe, queue);

    while(keypad_running) {
        pthread_mutex_lock(&keypad_queue_lock);
        int queue_empty = is_empty(queue);
        pthread_mutex_unlock(&keypad_queue_lock);
        if(queue_empty) {
        KEYPAD_IO_PRINTF("Making queue inaccessible\n");
        pthread_mutex_lock(&keypad_queue_available);
        }

        Packet_type type;
        unsigned short size;
        unsigned short tag;
        read(i_pipe, &type, sizeof(Packet_type));
        read(i_pipe, &size, sizeof(unsigned short));
        read(i_pipe, &tag, sizeof(unsigned short));
        read(i_pipe, buffer, size);

        KEYPAD_IO_PRINTF("Found packet with type %d, size %d\n", type, size);
        KEYPAD_IO_PRINTF("Buffer holds: %s: with size %d\n", buffer, size);

        if(type != KEYPAD) {
            KEYPAD_IO_PRINTF("Packet not supported for Keypad firmware\n");
            continue;
        }

        Inst_packet* new_packet = create_inst_packet(type, size, buffer, tag);

        KEYPAD_IO_PRINTF("Locking queue\n");
        pthread_mutex_lock(&keypad_queue_lock);

        KEYPAD_IO_PRINTF("Queueing packet\n");
        enqueue(queue, new_packet);

        KEYPAD_IO_PRINTF("Releasing queue & making it accessible\n");
/* This code is for the keypad firmware for the Hampod Program
* Written by Brendan Perez
* Last modified on 10/23/2023
* Updated for Raspberry Pi USB HAL on 11/28/2025
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>

#include "hampod_queue.h"
#include "hampod_firm_packet.h"
#include "hal/hal_keypad.h"

extern pid_t controller_pid;

unsigned char keypad_running = 1;

pthread_mutex_t keypad_queue_lock;
pthread_mutex_t keypad_queue_available;

void *keypad_io_thread(void* arg);

// Debug print statements from this process are White (\033[0;m)
void keypad_process(){

    KEYPAD_PRINTF("Keypad reader process launched\n");

    // Initialize HAL
    if (hal_keypad_init() != 0) {
        KEYPAD_PRINTF("Failed to initialize keypad HAL\n");
        // We continue anyway, maybe it will work later or user will plug it in
    } else {
        KEYPAD_PRINTF("Keypad HAL initialized: %s\n", hal_keypad_get_impl_name());
    }

    KEYPAD_PRINTF("Connecting to input/output pipes\n");


    int input_pipe_fd = open(KEYPAD_I, O_RDONLY);
    if(input_pipe_fd == -1) {
        perror("open");
        // kill(controller_pid, SIGINT);
        exit(0);
    }

    int output_pipe_fd = open(KEYPAD_O, O_WRONLY);
    if(output_pipe_fd == -1) {
        perror("open");
        // kill(controller_pid, SIGINT);
        exit(0);
    }
    
	KEYPAD_PRINTF("Pipes successfully connected\n");
    KEYPAD_PRINTF("Creating input queue\n");

    Packet_queue* input_queue = create_packet_queue();

    KEYPAD_PRINTF("Creating queue mutex lock\n");

    if(pthread_mutex_init(&keypad_queue_lock, NULL) != 0) {
        perror("pthread_mutex_init");
        // kill(controller_pid, SIGINT);
        exit(1);
    }
    KEYPAD_PRINTF("Creating queue availibility mutex lock\n");

    if(pthread_mutex_init(&keypad_queue_available, NULL) != 0) {
        perror("pthread_mutex_init");
        // kill(controller_pid, SIGINT);
        exit(1);
    }
    
    pthread_t keypad_io_buffer;

    keypad_io_packet thread_input;
    thread_input.pipe_fd = input_pipe_fd;
    thread_input.queue = input_queue;

    KEYPAD_PRINTF("Launching IO thread\n");

    if(pthread_create(&keypad_io_buffer, NULL, keypad_io_thread, (void*)&thread_input) != 0){
        perror("Keypad IO thread failed");
        // kill(controller_pid, SIGINT);
        exit(1);
    }
    usleep(500000); //Half sec sleep to let child thread take control
	while(keypad_running){
		pthread_mutex_lock(&keypad_queue_available);
        pthread_mutex_lock(&keypad_queue_lock);
        if(is_empty(input_queue)) {    
		    pthread_mutex_unlock(&keypad_queue_available);
            pthread_mutex_unlock(&keypad_queue_lock);
            usleep(500);
            continue;
        }
        Inst_packet* received_packet = dequeue(input_queue);
        pthread_mutex_unlock(&keypad_queue_lock);
        pthread_mutex_unlock(&keypad_queue_available);
        
        char read_value = -1;
        if (received_packet->data[0] == 'r') {
            // Use HAL to read keypad
            KeypadEvent event = hal_keypad_read();
            if (event.valid) {
                read_value = event.key;
            } else {
                read_value = '-'; // No key pressed
            }
        }

        Inst_packet* packet_to_send = create_inst_packet(KEYPAD, 1, (unsigned char*)&read_value, received_packet->tag);
        KEYPAD_PRINTF("Sending back value of %x ('%c')\n", read_value, (char) read_value);
        write(output_pipe_fd, packet_to_send, 8);
        write(output_pipe_fd, packet_to_send->data, 1);

        destroy_inst_packet(&packet_to_send);
	}
    pthread_join(keypad_io_buffer, NULL);
    destroy_queue(input_queue);
    close(input_pipe_fd);
    close(output_pipe_fd); //Graceful closing is always nice :)
    
    hal_keypad_cleanup();
    return;
}

//Debug print statements from this thread are Cyan (\033[0;96m)
void *keypad_io_thread(void* arg) {
    
    KEYPAD_IO_PRINTF("Keypad IO thread created\n");

    keypad_io_packet* new_packet =  (keypad_io_packet*)arg;
    int i_pipe = new_packet->pipe_fd;
    Packet_queue* queue = new_packet->queue;
    unsigned char buffer[256];

    KEYPAD_IO_PRINTF("Input pipe = %d, queue ptr = %p\n", i_pipe, queue);

    while(keypad_running) {
        pthread_mutex_lock(&keypad_queue_lock);
        int queue_empty = is_empty(queue);
        pthread_mutex_unlock(&keypad_queue_lock);
        if(queue_empty) {
        KEYPAD_IO_PRINTF("Making queue inaccessible\n");
        pthread_mutex_lock(&keypad_queue_available);
        }

        Packet_type type;
        unsigned short size;
        unsigned short tag;
        read(i_pipe, &type, sizeof(Packet_type));
        read(i_pipe, &size, sizeof(unsigned short));
        read(i_pipe, &tag, sizeof(unsigned short));
        read(i_pipe, buffer, size);

        KEYPAD_IO_PRINTF("Found packet with type %d, size %d\n", type, size);
        KEYPAD_IO_PRINTF("Buffer holds: %s: with size %d\n", buffer, size);

        if(type != KEYPAD) {
            KEYPAD_IO_PRINTF("Packet not supported for Keypad firmware\n");
            continue;
        }

        Inst_packet* new_packet = create_inst_packet(type, size, buffer, tag);

        KEYPAD_IO_PRINTF("Locking queue\n");
        pthread_mutex_lock(&keypad_queue_lock);

        KEYPAD_IO_PRINTF("Queueing packet\n");
        enqueue(queue, new_packet);

        KEYPAD_IO_PRINTF("Releasing queue & making it accessible\n");
        pthread_mutex_unlock(&keypad_queue_lock);
        if(queue_empty) {
            pthread_mutex_unlock(&keypad_queue_available);
        }
        usleep(100);
    }
    return NULL;
}

// Global fd for reading keypad in Software mode
int software_keypad_pipe_fd = -1;

void keypadTurnon() {
    KEYPAD_PRINTF("Software: Opening keypad pipe for reading\n");
    software_keypad_pipe_fd = open(KEYPAD_O, O_RDONLY | O_NONBLOCK);
    if (software_keypad_pipe_fd == -1) {
        perror("Failed to open keypad pipe in keypadTurnon");
    }
}

int readNumPad() {
    if (software_keypad_pipe_fd == -1) {
        // Try to open if not already open (lazy init or retry)
        software_keypad_pipe_fd = open(KEYPAD_O, O_RDONLY | O_NONBLOCK);
        if (software_keypad_pipe_fd == -1) return '-';
    }

    unsigned char header[8];
    int n = read(software_keypad_pipe_fd, header, 8);
    if (n < 8) {
        return '-';
    }

    // We read the header, now read the data
    // Assuming data length is 1 for keypad packets as per keypad_process
    unsigned char data;
    n = read(software_keypad_pipe_fd, &data, 1);
    if (n < 1) {
        return '-';
    }
    
    return (int)data;
}