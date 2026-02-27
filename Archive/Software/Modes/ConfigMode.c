#include "ConfigMode.h"
//Config mode is broken up into multiple parts, the idea is to have one section dedicated to navigation and simple selection of configs, the other is ment for more common but complicated stuff, like having the user input a number
char** configNames;
int currentConfig = 0;
double* oldValues;
bool selectingConfig = true;

void configNavigation(KeyPress* keyInput){
    char output[100];
    char* output2;
    KeyPress* clearing;
    switch (keyInput->keyPressed)
    {
    case '4':
        currentConfig --;
        if(currentConfig < 0){
            currentConfig = getLengthOfConfigs()-1;
        }
        PRINTFLEVEL1("SOFTWARE: switched configs to %s\n", configNames[currentConfig]);
        sprintf(output, "%s",configNames[currentConfig]);
        sendSpeakerOutput(output);
        //say the name
        break;
    
    case '6':
        currentConfig ++;
        if(currentConfig >= getLengthOfConfigs()){
            currentConfig = 0;
        }
        PRINTFLEVEL1("SOFTWARE: switched configs to %s\n", configNames[currentConfig]);
        sprintf(output, "%s",configNames[currentConfig]);
        sendSpeakerOutput(output);
        //say the name
        break;
    case '8':
        switch (getConfigByName(configNames[currentConfig])->configType)
        {
        case ONOFF:
        case NUMERIC:
            if(strcmp(configNames[currentConfig], "Frequency")==0) {
                printf("I made it to the frequencyConfig\n");
                getConfigByName(configNames[currentConfig])->defaultValue -= 100;
                char freqString[10];
                sprintf(freqString, "%d\n", getConfigByName(configNames[currentConfig])->defaultValue);
            }

        case ONOFFNUMERIC:
            output2 = incrementConfig(configNames[currentConfig], false);
            PRINTFLEVEL1("SOFTWARE: Set config %s to %s\n", configNames[currentConfig], output2);
            sendSpeakerOutput(output2);
            break;
        
        default:
            break;
        }
        break;
    case '2':
        switch (getConfigByName(configNames[currentConfig])->configType)
        {
        case ONOFF:
        case NUMERIC:
        case ONOFFNUMERIC:
            output2 = incrementConfig(configNames[currentConfig], true);
            PRINTFLEVEL1("SOFTWARE: Set config %s to %s\n", configNames[currentConfig], output2);
            sendSpeakerOutput(output2);
            break;
        default:
            break;
        }
        break;

    case '5':
        switch(getConfigByName(configNames[currentConfig])->configType){
            case NUMPAD:
                clearing = malloc(sizeof(KeyPress));
                clearing->keyPressed = '#';
                keypadInput(clearing);
                free(clearing);
                selectingConfig = false;
                sendSpeakerOutput("Input number to set value to");
                break;
            case OTHER:
                PRINTFLEVEL1("SOFTWARE: Running function related to config%s\n", configNames[currentConfig]);
                sendSpeakerOutput(getConfigByName(configNames[currentConfig])->startingDescription);
                selectingConfig = false;
                // keyInput->keyPressed = '-';
                // configNavigation(keyInput);
                break;
            default:
                break;
        }
        break;
    case '9':
        PRINTFLEVEL1("SOFTWARE: Saving changes to configs\n");
        if (writeConfigParamsToFile("ConfigSettings/Configs.txt") == 0) {
            sendSpeakerOutput("Configureations saved successfully");
        } else {
            sendSpeakerOutput("Error saving configurations");
        }

        free(oldValues);
        oldValues = getListOfCurrentValues();

        break;

    case '7':
        PRINTFLEVEL1("SOFTWARE: canceling changes to configs\n");
        setListOfcurrentValues(oldValues);
        break;

    default:
        break;
    }
}

void configOTHERFlow(KeyPress* keyInput){
    int returnnedValue = getConfigByName(configNames[currentConfig])->configFuntion(keyInput);
    PRINTFLEVEL1("SOFTWARE: Got value %i form config function\n",returnnedValue);
    if(returnnedValue == 1){
         selectingConfig = true;
    }
}

void configNUMPADFlow(KeyPress* keyInput){
    double value = keypadInput(keyInput);
    char buffer[30];
    if(value <= 0){
        sprintf(buffer, "%c", keyInput->keyPressed);
        sendSpeakerOutput(buffer);
        //output the number
    }else{
        sprintf(buffer, "%f", value);
        sendSpeakerOutput(buffer);
        updateConfigs(configNames[currentConfig], value);
        selectingConfig = true;
        //output the number
    }
}
/**
 * 4,6 go forward or backwards in a config 
 * 2,8 go up and down for the configs
 * 7, save changes
 * 9, cancel changes
 * 5 run the config spesific command 
*/
void* configCommandRelay(KeyPress* keyInput, RIG* radioDetails){
    if(selectingConfig){
        configNavigation(keyInput);
    }else{
        switch(getConfigByName(configNames[currentConfig])->configType){
            case NUMPAD:
                configNUMPADFlow(keyInput);
                break;
            case OTHER:
                configOTHERFlow(keyInput);
                break;
            default:
                break;
        }
    }
    
    return NULL;
}

void freeConfigMode(Mode* modeToFree){
    free(modeToFree->modeDetails->modeName);
    free(modeToFree->modeDetails);
    free(modeToFree);
}

//Grab the current values when loading in
void enterConfigMode(){
    configNames = getListOfConfigNames();
    oldValues = getListOfCurrentValues();
}

//exiting without saving will just cancel all changes 
void exitConfigMode(){
    setListOfcurrentValues(oldValues);
    free(configNames);
    free(oldValues);
    configNames = getListOfConfigNames();
    oldValues = getListOfCurrentValues();
}

Mode* ConfigLoad(){
    Mode* newMode = (Mode*) malloc(sizeof(Mode));

    if(newMode == NULL){
        return NULL;
    }
    newMode->modeInput = configCommandRelay;
    newMode->freeMode = freeConfigMode;
    newMode->enterMode = enterConfigMode;
    newMode->exitMode = enterConfigMode;

    ModeData* newData = (ModeData*)malloc(sizeof(ModeData));

    if(newData == NULL){
        free(newMode);
        return NULL;
    }
    newData->modeName = strdup("Config");
    newData->radioModel = 42;

    newMode->modeDetails = newData;

    return newMode;
}
