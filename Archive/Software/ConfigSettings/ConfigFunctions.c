//This is the file that is edited inorder to add new complex configs to the software
#include "ConfigFunctions.h"
void populateConfigFunctions(){
    ConfigParam* OtherFlagged;
    OtherFlagged = getConfigByName("save");
    OtherFlagged->configFuntion = SaveData;
    char* SaveDescription = "Press any number one through nine to save to that save file";
    OtherFlagged->startingDescription = malloc(strlen(SaveDescription)+1);
    strcpy(OtherFlagged->startingDescription,SaveDescription);
    OtherFlagged = getConfigByName("HotKey");
    OtherFlagged->configFuntion = setHotkeys;
    char* HotKeyDescription = "Press any number one through nine to select mode to be linked";
    OtherFlagged->startingDescription = malloc(strlen(HotKeyDescription)+1);
    strcpy(OtherFlagged->startingDescription,HotKeyDescription);
}

void saveToFile(int fileNumber){
    char cwd[1024];
if (getcwd(cwd, sizeof(cwd)) != NULL) {
    printf("Current working directory: %s\n", cwd);
} else {
    perror("getcwd() error");
    return;
}
    char fileName[100];
    sprintf(fileName, "ConfigSettings/SaveFiles/saveNumber%i.txt",fileNumber);
    FILE *fp = fopen(fileName, "w+");
    printf("File Path: %s\n", fileName);
    if (fp == NULL)
    {
        printf("Error opening the file %s\n", fileName);
        return;
    }

    //fprintf(fp, "This is the line #%d\n", i + 1);

    //Save the configs
    fprintf(fp, "Start of Configs\n");
    char** configsToSave = getListOfConfigNames();
    int num = getLengthOfConfigs();
    for(int i = 0 ; i<num;i++){
        fprintf(fp, "%s : %f\n",configsToSave[i], getConfigByName(configsToSave[i])->currentValue);
    }
    //save the hotkeys
    fprintf(fp,"Start of Hotkeys\n");
    Mode** modesToSave = getHotKeyList();
    PRINTFLEVEL1("got the modesToSave with address of %i\n",(int)modesToSave);
    for(int i = 0; i<12;i++){
    PRINTFLEVEL1("Mode at index %i has the address of %i\n",i, (int) modesToSave[i]);
        if(modesToSave[i] == 0){ //TODO make it so that you can get the mode id's
            fprintf(fp,"%i:%s\n",i,"NULL");
            PRINTFLEVEL1("NO Mode in save%i\n", i);
        }else{
            fprintf(fp,"%i:%s\n",i,modesToSave[i]->modeDetails->modeName);
            PRINTFLEVEL1("Mode %s found at index %i\n",modesToSave[i]->modeDetails->modeName, i);
        }
    }
    //save the hamlib
    fprintf(fp,"Start of radios\n");
    Radio** radiosToSave = getAllRadios();
    for(int j = 0; j<2;j++){
        if(radiosToSave[j] != 0){
            PRINTFLEVEL1("Saving radio with port %i and model %i\n", radiosToSave[j]->port, radiosToSave[j]->model);
            fprintf(fp,"%s:%i:%i:%i\n", radiosToSave[j]->make, radiosToSave[j]->model, radiosToSave[j]->port, (int) radiosToSave[j]->myrig_model); 
        }
    }
    // close the file
    fclose(fp);
}

int SaveData(KeyPress* keyData){
    if(keyData->keyPressed == '-'){
        sendSpeakerOutput("Select save slot 0 to 9 to save to");
        return -1;
    }
    switch (keyData->keyPressed)
        {
            case '#':
                return 1;
                break;
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
                saveToFile(convertCharToKeyValue(keyData));
                PRINTFLEVEL1("SOFTWARE: Succsessfully save to a file\n");
                return 1;
                break;
            default:
            break;
        }
        return -1;
}

int chosenModeId = 0;
int pageNumber = 0;
bool selectingMode = true;
int setHotkeys(KeyPress* keyData){
    //1 turn of C,D supression
    setCDstate(false);
    //2 get the mode that they want to use
    Mode** modes;
    char buffer[200];
    if(selectingMode){
        switch (keyData->keyPressed)
        {
            case '*':
                modes = getAllModes();
                strcpy(buffer,"");
                for(int i = 0;i<9;i++){
                    if(getModeCount() < pageNumber*9 +1){
                        break;
                    }
                    char shortBuffer[30];
                    sprintf(shortBuffer," %i %s ", i, modes[pageNumber*9 + i]->modeDetails->modeName);
                    strcat(buffer,shortBuffer);

                }
                sendSpeakerOutput(buffer);
                break;
            case 'C':
                pageNumber ++;
                if(pageNumber*9 > getModeCount()){
                    pageNumber = 0;
                }else{
                    sendSpeakerOutput("Next page");
                }
                break;
            case 'D':
                pageNumber --;
                if(pageNumber < 0){
                    pageNumber = getModeCount()/9;
                }else{
                    sendSpeakerOutput("Prior page");
                }
                break;
            case '0':
                pageNumber = 0;
                selectingMode = true;
                setCDstate(true);
                return 1;
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                chosenModeId = pageNumber*9 + convertCharToKeyValue(keyData)-1;
                if(chosenModeId > getModeCount()){
                    break;
                }else{
                    sendSpeakerOutput(getAllModeNames()[chosenModeId]);
                    sendSpeakerOutput("Select state of C or D to link selected mode to");
                    selectingMode = false;
                }
                break;
            default:
            break;
        }
    }else{
        if(keyData->keyPressed == 'C' || keyData->keyPressed == 'D'){
            //3 get the hot key they want to set
            char* name = getAllModeNames()[chosenModeId];
            sprintf(buffer, "Linking mode %s to key %c with shift value %i and hold value %i",name,keyData->keyPressed,keyData->shiftAmount,keyData->isHold);
            sendSpeakerOutput(buffer);
            setProgramibleKeys(keyData,name);
            pageNumber = 0;
            selectingMode = true;
            setCDstate(true);
            return 1;
        }
    }
    return 0;
}
