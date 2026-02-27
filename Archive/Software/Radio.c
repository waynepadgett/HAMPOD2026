#include "Radio.h"

#define SERIAL_PORT_S0 "/dev/ttyS0"
#define SERIAL_PORT_S1 "/dev/ttyS1"
#define SERIAL_PORT_USB0 "/dev/ttyUSB0"
#define SERIAL_PORT_USB1 "/dev/ttyUSB1"

Radio** radios;
int maxRadios = 2;
int currentRadio = -1;
int radioCount = 0;

void startRadios(){
     radios = calloc(2,sizeof(Radio*));
}

void loadUpRadioUsingData(char* make, int model, int port, Mode* defaultMode, rig_model_t myrig_model){
    if(SIMULATEOUTPUT == 0){
        rig_load_all_backends(); 
    }
    Radio* newRadio = malloc(sizeof(Radio));
    newRadio->make = make;
    newRadio->model = model;
    newRadio->port = port;
    newRadio->currentMode = defaultMode; //THIS NEEDS TO BE SET TO SOMETHING
    newRadio->myrig_model = myrig_model; 
    newRadio->my_rig = rig_init(myrig_model);

    switch (port) {
        case 1:
            rig_set_conf(newRadio->my_rig, rig_token_lookup(newRadio->my_rig, "rig_pathname"), SERIAL_PORT_S0);
            break;
        case 2:
            rig_set_conf(newRadio->my_rig, rig_token_lookup(newRadio->my_rig, "rig_pathname"), SERIAL_PORT_S1);
            break;
        case 3:
            rig_set_conf(newRadio->my_rig, rig_token_lookup(newRadio->my_rig, "rig_pathname"), SERIAL_PORT_USB0);
            break;
        case 4:
            rig_set_conf(newRadio->my_rig, rig_token_lookup(newRadio->my_rig, "rig_pathname"), SERIAL_PORT_USB1);
            break;
        default:
            // Handle invalid port value, e.g., set a default or log an error
            break;
    }

    rig_open(newRadio->my_rig);
    currentRadio = radioCount;
    radioCount++;
    radios[currentRadio] = newRadio;
}

void freeRadios(){
    for(int i = 0; i<radioCount;i++){
        if(SIMULATEOUTPUT == 0){
            rig_close(radios[i]->my_rig);
            rig_cleanup(radios[i]->my_rig);
        }
        free(radios[i]);
    }
}

Mode* getCurrentMode(){
    return radios[currentRadio]->currentMode;
}

ModeData* getModeDetails(){
    return radios[currentRadio]->currentMode->modeDetails;
    }

//TODO make sure each mode has this, even if it is set to null
void setRadioMode( Mode* modeToSetTo){
    if(radios[currentRadio]->currentMode != NULL && radios[currentRadio]->currentMode->exitMode != NULL){
        radios[currentRadio]->currentMode->exitMode();
    }
    radios[currentRadio]->currentMode = modeToSetTo;
    if(modeToSetTo->enterMode != NULL){
        modeToSetTo->enterMode();
    }
}

void* runRadioCommand(KeyPress* keyInput){
    PRINTFLEVEL1("running mode of %s\n",radios[currentRadio]->currentMode->modeDetails->modeName);
    void* results;
    printf("About to obtain results\n");
    if (radios[currentRadio] == NULL) {
        printf("radios[%d] is NULL!\n", currentRadio);
    }
    if (radios[currentRadio]->currentMode == NULL) {
        printf("currentMode is NULL after setRadioMode!\n");
    }
    if (radios[currentRadio]->my_rig == NULL) {
        printf("my_rig is NULL!\n");
    }
    results = radios[currentRadio]->currentMode->modeInput(keyInput,radios[currentRadio]->my_rig);
    printf("Obtained results\n");
    return results;
}

void setCurrentRadio(int radioID){
    currentRadio = radioID;
}
int getRadioAmount(){
    return radioCount;
}
int getCurrentRadioID(){
    return currentRadio;
}
Radio* getCurrentRadio(){
    return radios[currentRadio];
}
Radio** getAllRadios(){
    return radios;
}
