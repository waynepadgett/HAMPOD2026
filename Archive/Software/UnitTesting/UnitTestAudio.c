bool testCreatingAudioHash(){
    setupAudioHashMap();
    return true;
}

bool testGetAudio(){
    sendSpeakerOutput("1");
    sendSpeakerOutput("one");
    sendSpeakerOutput("DTMF0");
    sendSpeakerOutput("this is a sentince");
    return true;
}