#include "MonitoringMode.h"

int* stateOfMonitoring;
vfo_t general_vfo_monitoring = RIG_VFO_CURR;

void* monitoringModeRelay(KeyPress* keyInput, RIG* rig){
    void** inputData;
    switch (keyInput->keyPressed){
        case '1'://Frequensy
            if(!stateOfMonitoring[0]){//turn it on
                sendSpeakerOutput("Starting monitoring of Frequensy");
                stateOfMonitoring[0] = 1;
                inputData = malloc(sizeof(void*)*2);
                inputData[0] = rig;
                inputData[1] = &general_vfo_monitoring;
                addMonitoringLink(get_current_frequency,(void*) inputData);
            }else{//turn it off
                sendSpeakerOutput("Stopping monitoring of Frequensy");
                stateOfMonitoring[0] = 0;
                free(removeMonitoringLink(get_current_frequency));
            }
        break;
        case '2'://vfo
            if(!stateOfMonitoring[1]){//turn it on
                sendSpeakerOutput("Starting monitoring of VFO");
                stateOfMonitoring[1] = 1;
                inputData = malloc(sizeof(void*)*1);
                inputData[0] = rig;
                addMonitoringLink(get_current_vfo, (void*) inputData);
            }else{//turn it off
                sendSpeakerOutput("Stopping monitoring of VFO");
                stateOfMonitoring[1] = 0;
                free(removeMonitoringLink(get_current_vfo));
            }
        break;
        case '3'://get_current_transceive_mode
            if(!stateOfMonitoring[2]){//turn it on
                sendSpeakerOutput("Starting monitoring of Transceive mode");
                stateOfMonitoring[2] = 1;
                inputData = malloc(sizeof(void*)*1);
                inputData[0] = rig;
                addMonitoringLink(get_current_transceive_mode, (void*) inputData);
            }else{//turn it off
                sendSpeakerOutput("Stopping monitoring of Transceive mode");
                stateOfMonitoring[2] = 0;
                free(removeMonitoringLink(get_current_transceive_mode));
            }
        break;
        case '4'://get_current_mode
            if(!stateOfMonitoring[3]){//turn it on
                sendSpeakerOutput("Starting monitoring of Radio mode");
                stateOfMonitoring[3] = 1;
                inputData = malloc(sizeof(void*)*2);
                inputData[0] = rig;
                inputData[1] =  &general_vfo_monitoring;
                addMonitoringLink(get_current_mode,(void*) inputData);
            }else{//turn it off
                sendSpeakerOutput("Stopping monitoring of Radio mode");
                stateOfMonitoring[3] = 0;
                free(removeMonitoringLink(get_current_mode));
            }
        break;
        case '5'://get_current_rit_offset
            if(!stateOfMonitoring[4]){//turn it on
                sendSpeakerOutput("Starting monitoring of rit offset");
                stateOfMonitoring[4] = 1;
                inputData = malloc(sizeof(void*)*2);
                inputData[0] = rig;
                inputData[1] =  &general_vfo_monitoring;
                addMonitoringLink(get_current_rit_offset,(void*) inputData);
            }else{//turn it off
                sendSpeakerOutput("Stopping monitoring of rit offset");
                stateOfMonitoring[4] = 0;
                free(removeMonitoringLink(get_current_rit_offset));
            }
        break;
        case '6'://get_current_xit_offset
            if(!stateOfMonitoring[5]){//turn it on
                sendSpeakerOutput("Starting monitoring of xit offset");
                stateOfMonitoring[5] = 1;
                inputData = malloc(sizeof(void*)*2);
                inputData[0] = rig;
                inputData[1] =  &general_vfo_monitoring;
                addMonitoringLink(get_current_xit_offset,(void*) inputData);
            }else{//turn it off
                sendSpeakerOutput("Stopping monitoring of xit offset");
                stateOfMonitoring[5] = 0;
                free(removeMonitoringLink(get_current_xit_offset));
            }
        break;
        case '7'://get_current_tuning_step
            if(!stateOfMonitoring[6]){//turn it on
                sendSpeakerOutput("Starting monitoring of tuning step");
                stateOfMonitoring[6] = 1;
                inputData = malloc(sizeof(void*)*2);
                inputData[0] = rig;
                inputData[1] =  &general_vfo_monitoring;
                addMonitoringLink(get_current_tuning_step,(void*) inputData);
            }else{//turn it off
                sendSpeakerOutput("Stopping monitoring of tuning step");
                stateOfMonitoring[6] = 0;
                free(removeMonitoringLink(get_current_tuning_step));
            }
        break;
        case '8'://get_level
            if(!stateOfMonitoring[7]){//turn it on
                sendSpeakerOutput("Starting monitoring of level R F");
                stateOfMonitoring[7] = 1;
                inputData = malloc(sizeof(void*)*3);
                inputData[0] = rig;
                inputData[1] =  &general_vfo_monitoring;
                inputData[2] = (void*)(uintptr_t)RIG_LEVEL_RF;
                addMonitoringLink(get_level,(void*) inputData);
            }else{//turn it off
                sendSpeakerOutput("Stopping monitoring of level R F");
                stateOfMonitoring[7] = 0;
                free(removeMonitoringLink(get_level));
            }
        break;
        case '9'://get_func
            if(!stateOfMonitoring[8]){//turn it on
                sendSpeakerOutput("Starting monitoring of Func Squelch");
                stateOfMonitoring[8] = 1;
                inputData = malloc(sizeof(void*)*3);
                inputData[0] = rig;
                inputData[1] =  &general_vfo_monitoring;
                inputData[2] = (void*)(uintptr_t)RIG_FUNC_SQL;
                addMonitoringLink(get_func,(void*) inputData);
            }else{//turn it off
                sendSpeakerOutput("Stopping monitoring of Func Squelch");
                stateOfMonitoring[8] = 0;
                free(removeMonitoringLink(get_func));
            }
        break;
        case '*': //clears out the 
            sendSpeakerOutput("Stopping all monitorings");
            if(stateOfMonitoring[0]){
                free(removeMonitoringLink(get_current_frequency));
            }
            if(stateOfMonitoring[1]){
                free(removeMonitoringLink(get_current_vfo));
            }
            if(stateOfMonitoring[2]){
                free(removeMonitoringLink(get_current_transceive_mode));
            }
            if(stateOfMonitoring[3]){
                free(removeMonitoringLink(get_current_mode));
            }
            if(stateOfMonitoring[4]){
                free(removeMonitoringLink(get_current_rit_offset));
            }
            if(stateOfMonitoring[5]){
                free(removeMonitoringLink(get_current_xit_offset));
            }
            if(stateOfMonitoring[6]){
                free(removeMonitoringLink(get_current_tuning_step));
            }
            if(stateOfMonitoring[7]){
                free(removeMonitoringLink(get_level));
            }
            if(stateOfMonitoring[8]){
                free(removeMonitoringLink(get_func));
            }
            for(int i = 0; i<9;i++){
                stateOfMonitoring[i] = 0;
            }
        break;
        case '#'://says some output
            sendSpeakerOutput("Press number key to toggle. 1 frequency, 2 V F O, 3 transceive mode, 4 Radio mode, 5 rit offset, 6 xit offset, 7 tuning step, 8 level R F, 9  Func Squelch, * turn off all monitorings");
        break;
        default:
        break;
    }
}

void freeModnitoringMode(Mode* self){
    free(self->modeDetails->modeName);
    free(self->modeDetails);
    free(self);
    free(stateOfMonitoring);
}

Mode* MonitoringLoad(){
    Mode* newMode = (Mode*) malloc(sizeof(Mode));

    if(newMode == NULL){
        return NULL;
    }
    newMode->modeInput = monitoringModeRelay;
    newMode->freeMode = freeModnitoringMode;
    newMode->enterMode = NULL;
    newMode->exitMode = NULL;

    ModeData* newData = (ModeData*)malloc(sizeof(ModeData));

    if(newData == NULL){
        free(newMode);
        return NULL;
    }
    newData->modeName = strdup("Monitoring");
    newData->radioModel = 42;

    newMode->modeDetails = newData;

    stateOfMonitoring = calloc(9,sizeof(int));

    return newMode;
}