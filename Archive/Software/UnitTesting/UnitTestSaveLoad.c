void saveLoadSetupStuffToSave(){
    //load up the configs
    loadConfigParams();
    printf("Configs loaded up\n");
    //create new hotkeys
    stateMachineStart();
    printf("Done staring components\n");
    KeyPress* testingInput = malloc(sizeof(KeyPress));
    //set key input
    testingInput->keyPressed = 'C';
    testingInput->isHold = false;
    testingInput->shiftAmount = 0;
    setProgramibleKeys(testingInput, "Config");
    //set keyinput
    testingInput->keyPressed = 'C';
    testingInput->isHold = false;
    testingInput->shiftAmount = 2;
    setProgramibleKeys(testingInput, "Normal");
    //set keyinput
    testingInput->keyPressed = 'D';
    testingInput->isHold = true;
    testingInput->shiftAmount = 1;
    setProgramibleKeys(testingInput, "DTMF");
    //set keyinput
    testingInput->keyPressed = 'D';
    testingInput->isHold = true;
    testingInput->shiftAmount = 0;
    setProgramibleKeys(testingInput, "frequency mode");
    //create radios
    printf("Done loading up hotkeys\n");
    char* makeTemp = malloc(sizeof(char)*30);
    strcpy(makeTemp,"model 1");
    setRadios(loadUpRadioUsingData(makeTemp,10,12,getModeByName("Normal"),0),0);
    makeTemp = malloc(sizeof(char)*30);
    strcpy(makeTemp,"icom thing I think");
    setRadios(loadUpRadioUsingData(makeTemp,43,62,getModeByName("frequency mode"),1),1);
    printf("Done loading up radios\n");
}

bool testSaveFile(){
    saveToFile(10);
    return true;
}

void setUpForLoadTesting(){
    freeStateMachine();
    freeCongigFull();
    loadConfigParams();
    stateMachineStart();
}
bool testLoadFile(){
    loadUpFromSave(10);
    return true;
}