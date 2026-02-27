/* TODO list
* 2) add a way to toggle if the letter keys are being used for standard input
* 3) redue how loading up a mode works to properly use the toggles
* 4) add a new mode metadata to say if it uses the letter keys and which ones, This sould be in pairs A,B and C,D
*/

#include "StateMachine.h"

ModeStates modeState = bootUp;
char** modeNames;
bool programableKeysOn = true;

ModeStates modeFlow(KeyPress* keyInput){
    PRINTFLEVEL2("Mode flow step 1\n");
    //the inital switch is for the programable keys, thisis so hat things can be avoided and passed over
    //specialty keys will be handled in each of the individual flows 
    switch (modeState)
    {
        case bootUp:
            BootupFlow(keyInput);
            return bootUp;
            break;
        case standard:
            StandardModeFlow(keyInput);
            return standard;
            break;
        case modeSelect:
            ModeSelectFlow(keyInput);
            return modeSelect;
            break;
        default:
            //make a screem of unknown
            //it should never get here
            break;
    }
        return -1;
}

//no idea how to store these yet
//TODO have there be another file dedicated to figureing this out since it will probably be a large function
char** companiesList;
char** modelList;
char** hamlibIDList;
char* company;
int modelIndex;
BootUpStates bootUpState = selectNewOrSave;
BootUpStates BootupFlow(KeyPress* keyInput){
    
    switch (bootUpState)
    {
        case selectNewOrSave:
            if(keyInput->keyPressed == '0')/*Save*/{
                bootUpState = selectSave;
                sendSpeakerOutput("Select save file to load up");
                break;
            }else if(keyInput->keyPressed == '1')/*Load new*/{
                sendSpeakerOutput("Select company of the radio to load up");
                bootUpState = chooseCompany;
                break;
            }else if(keyInput->keyPressed == '#'){
                sendSpeakerOutput("zero Select Save. One select company. Hash to read out extra controls");
                break;
            }
            break;
        case chooseCompany:
            if (keyInput->keyPressed == '0') {
                sendSpeakerOutput("zero Select Save. One select company. Hash to read out extra controls");
                bootUpState = selectNewOrSave;
                break;
            }else if(keyInput->keyPressed == '#'){
                sendSpeakerOutput("One through nine to select company. zero to go back. star then number to read out company of said number. Star hold to read out all companys. C and D to go to next and prior page");
                break;
            }
            int index = selectEntryInList(keyInput,companiesList);
            PRINTFLEVEL1("Software: gotback company index of %i\n", index);
            if(index != -1){
                company = companiesList[index];
                char fileNames[100];
                sprintf(fileNames,"StartupFiles/%s_Model.txt",company);
                printf("I'm here before making a model list\n");
                modelList = textFileToArray(fileNames);
                for(int i=0; i<5; i++) {
                    printf("About to print %i",i);
                    printf(modelList[i]);
                    printf("\n");
                }
                sprintf(fileNames,"StartupFiles/%s_Hamlib.txt",company);
                hamlibIDList = textFileToArray(fileNames);
                bootUpState = chooseModel;
                sendSpeakerOutput("Select Model of radio");
            }
            break;
        case chooseModel:
            if (keyInput->keyPressed == '0') {
                bootUpState = chooseCompany;
                sendSpeakerOutput("Select company of the radio to load up");
                break;
            }else if(keyInput->keyPressed == '#'){
                sendSpeakerOutput("One through nine to select model. zero to go back. star then number to read out model of said number. Star hold to read out all models. C and D to go to next and prior page");
                break;
            }
            modelIndex = selectEntryInList(keyInput,modelList);
            if(modelIndex != -1){
                sendSpeakerOutput("Select port radio is connected to. Choose number one to four");
                bootUpState = selectLink;
            }
            break;
        case selectLink:
            if(keyInput->keyPressed == '0')/*Back*/{
                    bootUpState = chooseModel;
                    break;
                }else{
                    char outputText[100];
                    //Add in 1234 protection
                    switch(keyInput->keyPressed){
                        case '1':
                        case '2':
                        case '3':
                        case '4':
                            
                            sprintf(outputText, "Linking radio make %s of model %s to port %i", company, modelList[modelIndex], convertCharToKeyValue(keyInput));
                            sendSpeakerOutput(outputText);
                            loadUpRadioUsingData(company,atoi(modelList[modelIndex]), convertCharToKeyValue(keyInput), getModeByName("frequency mode"), atoi(hamlibIDList[modelIndex]));
                            if(getCurrentRadioID() == 1){
                                modeState = standard;
                                sendSpeakerOutput("Starting normal operations");
                            }else{
                                bootUpState = linkMore;
                                sendSpeakerOutput("Do you have more radios to link up?");
                            }
                            break;
                        default:
                            sendSpeakerOutput("Choose number one to four");
                            break;
                    }
                    break;
                }
                break;
        case linkMore:
            if(keyInput->keyPressed == '1')/*Yes*/{
                bootUpState = chooseCompany;
                sendSpeakerOutput("Select company of the radio to load up");
                break;
            }else if(keyInput->keyPressed == '0')/*No*/{
                modeState = standard;
                sendSpeakerOutput("Starting normal operations");
                break; 
            }
            break;
        case selectSave:
            if(keyInput->keyPressed == '0')/*Back*/{
                sendSpeakerOutput("zero Select Save. One select company. Hash to read out extra controls");
                bootUpState = selectNewOrSave;
                break;
            }else{
                bool returnedFromLoad = false;
                char outputText[100];
                switch (keyInput->keyPressed){
                    case '1':
                    case '2':
                    case '3':
                    case '4':
                    case '5':
                    case '6':
                    case '7':
                    case '8':
                    case '9':
                        sprintf(outputText,"Loading up save file %i\n",convertCharToKeyValue(keyInput));
                        // sendSpeakerOutput(outputText);

                        returnedFromLoad = loadUpFromSave(convertCharToKeyValue(keyInput));
                        PRINTFLEVEL1("got back value of %i from the loading function\n",returnedFromLoad);
                        if(returnedFromLoad){
                            sendSpeakerOutput("Sarting normal operations");
                            modeState = standard;
                        }else{
                            sendSpeakerOutput("Something when wrong loading up the save file");
                        }
                        break;
                    default:
                        PRINTFLEVEL1("Software: Invalid save file chosen\n");
                        sendSpeakerOutput("Invalid key was pressed");
                        break;
                }
            }
            break;
        default:
        //should not get here
        break;
    }
    return bootUpState;
}


int modeSelectPage = 0; //the page number that we are on for mode select
int isReadingOut = 0; //TODO make this actualy cause a read out
/**
 * this handles the select mode state
*/
int ModeSelectFlow(KeyPress* keyInput){
    if(keyInput->keyPressed == '#'){
        sendSpeakerOutput("One through nine to select mode. zero to go back. star then number to read out mode of said number. Star hold to read out all modes on page. C and D to go to next and prior page");
        return 1;
    }
    if(isReadingOut){
         switch(keyInput->keyPressed){
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                if(modeSelectPage*9 + convertCharToKeyValue(keyInput)-1 < getModeCount()){
                    sendSpeakerOutput(modeNames[modeSelectPage*9 + convertCharToKeyValue(keyInput)-1]);
                }else{
                    sendSpeakerOutput("out of bounce");
                }
                break;
            default:
                sendSpeakerOutput("No number key selected. returning to selection mode");
                break;
         }
        isReadingOut = 0;
    }else{
        char shortName[40];
        switch (keyInput->keyPressed){
            case 'C':
                if(modeSelectPage*9 < getModeCount()){
                    modeSelectPage = modeSelectPage + 1;
                }
                sprintf(shortName, "Switching to page %d",modeSelectPage);
                sendSpeakerOutputWithConditions(shortName,true,false,false);
                break;
            case 'D':
                if(modeSelectPage > 0){
                    modeSelectPage = modeSelectPage - 1;
                }
                sprintf(shortName, "Switching to page %d",modeSelectPage);
                sendSpeakerOutputWithConditions(shortName,true,false,false);
                break;
            case '*':
                if(keyInput->isHold){
                    char LongOutput[200];
                    strcpy(LongOutput, "");
                    for(int i = modeSelectPage*9; i < (modeSelectPage+ 1) * 9 ; i++){
                        if(i >= getModeCount()){
                            break;
                        }else{
                            char shortNameLocal[30];
                            sprintf(shortNameLocal, " %i %s ", i-modeSelectPage*9 + 1 ,modeNames[i]);
                            strcat(LongOutput,shortNameLocal);
                        }
                    }   
                    sendSpeakerOutput(LongOutput);
                }else{
                    isReadingOut = 1;
                    sendSpeakerOutput("Press number to read out its option");
                }
                break;
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                switchToRadioMode(modeNames[(modeSelectPage*9) + convertCharToKeyValue(keyInput) - 1]);
                modeSelectPage = 0;
                isReadingOut = 0;   
                break;
        
            default:
                //Error should never get here
                break;
        }
    }
        return -1;
}

//TODO fill this out 
int StandardModeFlow(KeyPress* keyInput){ //TODO be able to toggle the letter keys
    PRINTFLEVEL1("Standard flow for modes with key input of %c, shift of %i, and hold of %i\n",keyInput->keyPressed,keyInput->shiftAmount,keyInput->isHold);
    switch (keyInput->keyPressed)
    {
    case 'A': // A
        switch (keyInput->shiftAmount)
        {
        case 0:
            if(keyInput->isHold){
                //Volume up
            }else{
                
            }
            break;
        case 1:
            if(keyInput->isHold){
                //Volume up
            }else{
                
            }
            break;
        case 2:
            if(keyInput->isHold){
                //Volume up
            }else{
                
            }
            break;
        default:
            break;
        }
        break;
    case 'B': //B
     switch (keyInput->shiftAmount)
        {
        case 0:
            if(keyInput->isHold){
                //Volume down
                modeState = modeSelect;
                sendSpeakerOutput("Mode Select. press # to learn key presses");
            }else{
                runRadioCommand(keyInput);
            }
            break;
        case 1:
            if(keyInput->isHold){

            }else{
                // switchToRadioMode("Normal");
            }
            break;
        case 2:
             if(keyInput->isHold){

            }else{
               
            }
            break;
        default:
            break;
        }
        break;
    case 'C': // C
        if(getCDState()){
            switchToRadioMode(getModeViaProgramableKey(keyInput)->modeDetails->modeName);
        }else{
            runRadioCommand(keyInput);
        }
        break;
    case 'D': // D
        if(getCDState()){
            switchToRadioMode(getModeViaProgramableKey(keyInput)->modeDetails->modeName);
        }else{
            runRadioCommand(keyInput);
        }
        break;
    default:
        PRINTFLEVEL1("No letter key was pressed so going to the mode\n");
        runRadioCommand(keyInput);
        break;
    }
        return -1;
}


/**
 * Reads out the name of the asked for mode
 * TODO make this use firmware output
 * OUTDATED
*/
int readOutModeName(char* modeName){
    //DEBUG
    printf("%s", modeName);
    sendSpeakerOutput(modeName);
    return 1;
}

/**
 * This is passed the modeID
 * if the modeID is negitive that represents that it is a non-standard mode
 * TODO see if this can just be replaced with the new modeRouting file
 * Idea, have this be what takes in the modeID and then using it would also switch the modeState as needed
*/
int switchToRadioMode(char* modeName){ //TODO redue this to be better sueted, all this needs to do is to set what the letter toggles are when swiching to said mode
    modeState = standard;
    PRINTFLEVEL1("Switching to radio mode%s\n",modeName);
    char modeSwitchText[50];
    sprintf(modeSwitchText, "Switching to radio mode%s", modeName);
    sendSpeakerOutput(modeSwitchText);
    setRadioMode(getModeByName(modeName));
    return -1;
}



void stateMachineStart(){
    startRadios();
    modeRoutingStart();
    modeNames = getAllModeNames();
    PRINTFLEVEL1("SOFTWARE: mode names retreaved\n");
    companiesList = textFileToArray("StartupFiles/Company_List.txt");
}

void setModeState(ModeStates state){
    modeState = state;
}
void setBootUpState(BootUpStates state){
    bootUpState = state;
}

void toggleCDHotkeys(bool state){
    programableKeysOn = state;
}
void freeStateMachine(){
    freeRadios();
    PRINTFLEVEL2("finished freeing radios\n");
    freeModes();
    PRINTFLEVEL2("freed the modes\n");
        free(companiesList);
}



int charIndex = 0;
int charSelectPage = 0;
bool listReadOut = false;
/**
 * When # is pressed it read out next key press
 * When # is held down 
 * returns the index of where in the list the selected entry is
 * will return -1 when nothing is selected yet
 * Assumes you have used the textFileToArray function
 * pass in a keypress of key '-' to clear it
*/
int selectEntryInList(KeyPress* keyInput, char** list){
    if(keyInput->keyPressed == '-'){
        charIndex = 0;
        charSelectPage = 0;
        listReadOut = false;
    }
    if(listReadOut){
        switch(keyInput->keyPressed){
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                for(int i = charSelectPage*9; i < (charSelectPage) * 9 + convertCharToKeyValue(keyInput) - 1 ; i++){
                    if(strcmp(list[i], "END OF ARRAY") == 0){
                        sendSpeakerOutput("Out of range");
                        return -1;
                        break;
                    }
                }
                sendSpeakerOutput(list[charSelectPage*9 + convertCharToKeyValue(keyInput) - 1]);
                listReadOut = false;
                break;
            default:
                listReadOut = false;
                sendSpeakerOutput("No number key selected. returning to selection mode");
                break;
            }
            return -1;
    }else{
        bool flag;
        char* shortName = malloc(sizeof(char)*30);
        switch (keyInput->keyPressed){
            case 'C':
                flag = true;
                for(int i = charSelectPage*9; i < (charSelectPage+ 1) * 9 ; i++){
                    if(strcmp(list[i], "END OF ARRAY") == 0){
                        flag = false;
                        break;
                    }
                }
                if(flag){
                    charSelectPage = charSelectPage + 1;
                }
                sprintf(shortName, "Switching to page %d",charSelectPage);
                printf("DEBUG: This is the value of shortname: %s\n", shortName);
                sendSpeakerOutputWithConditions(shortName,true,false,false);
                break;
            case 'D':
                charSelectPage = charSelectPage - 1;
                if(charSelectPage < 0){ 
                    charSelectPage = 0;
                }
                sprintf(shortName, "Switching to page %d",charSelectPage);
                sendSpeakerOutputWithConditions(shortName,true,false,false);
                break;
            case '*':
                if(keyInput->isHold){
                    char* LongOutput = malloc(sizeof(char)*200);
                    strcpy(LongOutput, "");
                    for(int i = charSelectPage*9; i < (charSelectPage+ 1) * 9 ; i++){
                        if(strcmp(list[i], "END OF ARRAY") == 0){
                            break;
                        }else{
                            char* shortNameLocal = malloc(sizeof(char)*30);
                            sprintf(shortNameLocal, " %i %s ", i-charSelectPage*9 + 1 ,list[i] );
                            strcat(LongOutput,shortNameLocal);
                            free(shortNameLocal);
                        }
                    }
                    sendSpeakerOutput(LongOutput);
                    free(LongOutput);
                }else{
                    listReadOut = true;
                    sendSpeakerOutput("Press number to read out its option");
                }
                break;
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                printf("I am here before finding page: page: %i\n",charSelectPage);
                for(int i = charSelectPage*9; i < (charSelectPage) * 9 + convertCharToKeyValue(keyInput) - 1 ; i++){
                    if(strcmp(list[i], "END OF ARRAY") == 0){
                        sendSpeakerOutput("Out of range");
                        return -1;
                        break;
                    }
                }
                printf("past for loop\n");
                sendSpeakerOutput(list[charSelectPage*9 + convertCharToKeyValue(keyInput) - 1]);
                int returnValue = charSelectPage*9 + convertCharToKeyValue(keyInput) - 1;
                printf("Return value: %i\n",returnValue);
                charIndex = 0;
                charSelectPage = 0;
                listReadOut = false;
                return returnValue;
                break;
        
            default:
                //Error should never get here
                break;
        }
        free(shortName);
    }
    return -1;
}


bool loadUpFromSave(int saveFileNumber){
    //get the file to load
    char fileName[100];
    sprintf(fileName, "ConfigSettings/SaveFiles/saveNumber%i.txt",saveFileNumber);
    PRINTFLEVEL1("SOFTWARE: loading up save file %s\n",fileName);
    char** saveFile = textFileToArray(fileName);

    if( (int) saveFile == -1){
        PRINTFLEVEL1("SOFTWARE: Failed to load up save file %s\n",fileName);
        return false;
    }
    PRINTFLEVEL1("SOFTWARE: loaded up the file %s\n", fileName);
    int i = 0;
    printf("%s\n",saveFile[i]);
    i++;
    while(strcmp(saveFile[i], "Start of Hotkeys") != 0){
        /*
        1) substring to get the components
        2) convert the 2nd part to a double
        3) grab the config object
        4) set the value to the found double
        */
        // 1
        char* doubleString = strchr(saveFile[i], ':');
        int index = (int)(doubleString - saveFile[i]);
        char* Name = customSubString(saveFile[i],0, index);
        // 2
        double value = atof(doubleString+1);
        //3 4 

        PRINTFLEVEL1("SOFTWARE: loading value %f for config %s\n",value,Name);
        updateConfigs(Name, value);
        i++;
    }
    printf("%s\n",saveFile[i]);
    i++;
    while(strcmp(saveFile[i], "Start of radios") != 0){
        /*
        1) Convert substring into index and Name
        2) See if the name not null
        3) set the hotkey via getting the mode by name
        */
       //TODO set this up
        char* nameString = strchr(saveFile[i], ':') + 1;
        int seporatorIndex = (int)(nameString - saveFile[i]);
        int index = atoi(customSubString(saveFile[i],0, seporatorIndex));
        //2
        if(strcmp("NULL", nameString) != 0){
            PRINTFLEVEL1("Loading up mode %s into index %i\n",nameString,index);
            setProgramibleKeysByIndex(index,nameString);
        }
        i++;
    }
    printf("%s\n",saveFile[i]);
    i++;
    int j = 0;
    while(strcmp(saveFile[i], "END OF ARRAY") != 0){
        /*
        1) substring to get the components
        2) get the hampod ID and the port name
        3) link them together
        4) save it to the thing
        */
        char* point0 = strchr(saveFile[i], ':');
        char* point1 = strchr(point0+1, ':');
        char* point2 = strchr(point1+1, ':');
        PRINTFLEVEL2("SOFTWARE: Points found are %s,%s,%s\n",point0,point1,point2);
        int dis0 = (int) (point0 - saveFile[i]);
        int dis1 = (int) (point1 - point0);
        int dis2 = (int) (point2 - point1);
        int dis3 =  strlen(point2);
        PRINTFLEVEL2("SOFTWAR: the lengths are %i,%i,%i,%i\n",dis0,dis1,dis2,dis3);
        char* sMake = customSubString(saveFile[i],0,dis0);
        char* sModel = customSubString(saveFile[i],point0 - saveFile[i]+1,dis1);
        char* sPort = customSubString(saveFile[i],point1 - saveFile[i]+1,dis2);
        char* sRigModel =customSubString(saveFile[i],point2 - saveFile[i]+1,dis3);
        PRINTFLEVEL2("SOFTWARE: Created filtered strings %s, %s, %s, %s to load in\n",sMake,sModel,sPort,sRigModel);
        int iModel = atoi(sModel);
        int iPort = atoi(sPort);
        int iRigModel = atoi(sRigModel);
        PRINTFLEVEL1("Loading up radio id %i\n",iRigModel);
        loadUpRadioUsingData(sMake,iModel,iPort,getModeByName("Normal"),iRigModel);
        j++;
        i++;
    }
    free(saveFile);
    return true;
}
