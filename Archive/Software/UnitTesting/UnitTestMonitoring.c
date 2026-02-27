int wapper1Counter = 0;
int wapper2Counter = 0;

char* dummyGetWraper1(void* input){
    char* buffer = malloc(sizeof(char)*40);
    sprintf(buffer,"Wraper 1 has value %d",wapper1Counter);
    return buffer;
}
char* dummyGetWraper2(void* input){
    char* buffer = malloc(sizeof(char)*40);
    sprintf(buffer,"Wraper 2 has value %d",wapper2Counter);
    return buffer;
}
bool testCreateDestroyMonitoring(){
    startMonitoringLoop();
    usleep(100000);
    endMonitoringLoop();
    usleep(100000);
    return true;
}
bool testAddingMonitoringComponent(){
    startMonitoringLoop();
    usleep(100000);
    PRINTFLEVEL1("Adding first wraper\n");
    addMonitoringLink(dummyGetWraper1,NULL);
    PRINTFLEVEL2("first wraper has been added\n");
    usleep(100000);
    PRINTFLEVEL1("Adding second wraper\n");
    addMonitoringLink(dummyGetWraper2,NULL);
    usleep(100000);
    endMonitoringLoop();
    usleep(100000);
    return true;
}
//see if it will monitor changes and output them
bool testMonitoringAfterChanges(){
    printf("Should see, (wraper number, output): (1,1):(2,2):(1,3),(2,4)\n");
    startMonitoringLoop();
    usleep(100000);
    addMonitoringLink(dummyGetWraper1,NULL);
    usleep(100000);
    addMonitoringLink(dummyGetWraper2,NULL);
    usleep(100000);
    wapper1Counter = 1;
    usleep(100000);
    wapper2Counter = 2;
    usleep(100000);
    wapper1Counter = 3;
    wapper2Counter = 4;
    usleep(300000);
    endMonitoringLoop();
    usleep(100000);
    resetMonitoringWrappers();
    return true;
}
//remove test
bool testRemovingMonitoringComponent(){
    printf("Should see, (wraper number, output): (1,1):(2,2):(1,3),(2,4):(2,6)\n");
    startMonitoringLoop();
    usleep(100000);
    addMonitoringLink(dummyGetWraper1,NULL);
    usleep(100000);
    addMonitoringLink(dummyGetWraper2,NULL);
    usleep(100000);
    wapper1Counter = 1;
    usleep(100000);
    wapper2Counter = 2;
    usleep(100000);
    wapper1Counter = 3;
    wapper2Counter = 4;
    usleep(300000);
    printf("Removing wraper 1\n");
    removeMonitoringLink(dummyGetWraper1);
    usleep(200000);
    wapper1Counter = 5;
    wapper2Counter = 6;
    usleep(300000);
    printf("Removing wraper 2\n");
    removeMonitoringLink(dummyGetWraper2);
    usleep(200000);
    wapper1Counter = 7;
    wapper2Counter = 8;
    usleep(300000);
    endMonitoringLoop();
    usleep(100000);
    resetMonitoringWrappers();
    return true;
}


void resetMonitoringWrappers(){
    wapper1Counter = 0;
    wapper2Counter = 0;
}