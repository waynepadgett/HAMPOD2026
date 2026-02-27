#include "FrequensyMode.h"
double currentFrequency = 0;
int decimalPlace = 0;
bool hasDecimal = false;
// * does not go back to normal mode 
void* frequencyCommandRelay(KeyPress* keyInput, RIG* my_rig){
    switch(keyInput->keyPressed){
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            printf("I made it here in FrequencyCommandRelay\n");
            if(hasDecimal){
                decimalPlace ++;
            }
            currentFrequency = currentFrequency*10 + convertCharToKeyValue(keyInput);
            //read out the number
            char putoutmessage[2] = "a";
            putoutmessage[0] = keyInput->keyPressed;          
            sendSpeakerOutput(putoutmessage);
            break;
        case '#':
            currentFrequency = currentFrequency * 1000000;
            for(int i = 0; i<decimalPlace; i++){
                currentFrequency = currentFrequency / 10;
            }
        PRINTFLEVEL1("Software: would be setting the radio frequency %f",currentFrequency);
            //currentFrequency = currentFrequency / pow(10,decimalPlace);
            
            //TODO add the hamlib code to change the frequency here 
            //enter frequency into the radio
            int retcode = rig_set_freq(my_rig, RIG_VFO_CURR, currentFrequency);
            if (retcode != RIG_OK)
            {
                printf("rig_set_freq: error = %s \n", rigerror(retcode));
            }
            decimalPlace = 0;
            hasDecimal = false;
            currentFrequency = 0;
            break;
        case '*':
            if(hasDecimal){
                currentFrequency = 0;
                decimalPlace = 0;
                hasDecimal = false;
            }else{
                decimalPlace = 0;
                hasDecimal = true;
            }
            break;
        default:
        if(SIMULATEOUTPUT == 1){
            PRINTFLEVEL1("got key of %c and nothing happens with that input\n",keyInput->keyPressed);
        }
        break;
    }
    return NULL;
}


void freeFrequencyMode(Mode* modeToFree){
    free(modeToFree->modeDetails->modeName);
    free(modeToFree->modeDetails);
    free(modeToFree);
}

Mode* FrequensyLoad(){
    Mode* newMode = (Mode*) malloc(sizeof(Mode));

    if(newMode == NULL){
        return NULL;
    }
    newMode->modeInput = frequencyCommandRelay;
    newMode->freeMode = freeFrequencyMode;
    newMode->enterMode = NULL;
    newMode->exitMode = NULL;

    ModeData* newData = (ModeData*)malloc(sizeof(ModeData));

    if(newData == NULL){
        free(newMode);
        return NULL;
    }
    newData->modeName = strdup("frequency mode");
    newData->radioModel = 42; //not sure for why this is here and if I need it 

    newMode->modeDetails = newData;

    return newMode;
}
