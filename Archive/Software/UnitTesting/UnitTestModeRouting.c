bool testCreateModeRoutingFunction(){
    modeRoutingStart();
    freeModes();
    return true;
}
bool testGettingOneMode(){
    modeRoutingStart();
    Mode* testMode = getModeByName("Normal");
    if(testMode==0){
        freeModes();
        return false;
    }
    freeModes();
    return true;
}
bool testGettingManyModes(){
    modeRoutingStart();
    Mode* testMode = getModeByName("Config");
    if(testMode == 0){
        freeModes();
        return false;
    }
    testMode = getModeByName("Normal");
    if(testMode == 0){
        freeModes();
        return false;
    }
    freeModes();
    return true;
}
bool testGettingSameMode(){
    modeRoutingStart();
    Mode* testMode1 = getModeByName("Normal");
    Mode* testMode2 = getModeByName("Normal");

    if(testMode1 == testMode2){
        freeModes();
        return true;
    }
    freeModes();
    return false;
}

bool testFreeModes(){
    return true;
}

bool testGrabEachMode(){
    modeRoutingStart();
     getModeByName("Normal");
     getModeByName("Config");
     getModeByName("DTMF");
     getModeByName("frequency mode");
     freeModes();
     return true;
}