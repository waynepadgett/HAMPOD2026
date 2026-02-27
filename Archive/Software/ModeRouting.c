#include <stdlib.h>
#include <string.h>
#include "ModeRouting.h"
HashMap* ModeHashMap;
Mode** keyBinds;
char* libraryName = "./libModes.so";
char* path = "Modes";
/*
* Creaes both the hashmap of the modes and creates the array of the keybinds
*/
void modeRoutingStart(){
    ModeHashMap = createHashMap(StringHash,StringHashCompare);
    Mode* tempMode;

    //1) get the .c file name
    //2) modify the string names to use the load format
    char** loadFunctionNames = getNamesOfLoadFunctions();
    //3) run the dynamic grab and save them to the hashmap
    int i = 0;
    while(strcmp(loadFunctionNames[i],"ENDOFNAMES") != 0){
        printf("Adding mode with load function %s\n",loadFunctionNames[i]);
        tempMode = dynamicalyLoadInModeByName(loadFunctionNames[i]);
        insertHashMap(ModeHashMap, tempMode, tempMode->modeDetails->modeName);
        free(loadFunctionNames[i]);
        i++;
    }
    free(loadFunctionNames[i]);
    free(loadFunctionNames);
    //This starts up the keybinds
    keyBinds = calloc(12, sizeof(Mode*));
    if(keyBinds == NULL){
        printf("Mallocing the keybings failded\n");
        exit(-1);
    }
    PRINTFLEVEL1("SOFTWARE: keybings object created\n");
}

Mode* getModeByName(char* name){
    printf("About to getHashMap\n");
    Mode *someMode = getHashMap(ModeHashMap,name);
    if (someMode == NULL) {
        printf("getModeByName(\"frequency mode\") returned NULL!\n");
    } else {
        printf("getModeByName(\"frequency mode\" returned Mode pointer %p\n", (void*)someMode);
        if(someMode->modeInput == NULL) {
            printf("modeInput is NULL!\n");
        } else {
            printf("modeInput is at %p\n", (void*)someMode->modeInput);
        }
    }
    return (Mode*) getHashMap(ModeHashMap, name);
}
Mode** getAllModes(){
    return (Mode**) getAllEntriesHashMap(ModeHashMap);
}

char** getAllModeNames(){
    char** names = malloc(sizeof(char*) * ModeHashMap->quantity);
    Mode** modes = getAllModes();
    PRINTFLEVEL2("SOFTWARE:Gotten all mode objects\n");
    for(int i = 0;i<ModeHashMap->quantity;i++){
        Mode* tempMode = modes[i];
        PRINTFLEVEL2("SOFTWARE: Got temp mode\n");
         if(tempMode == NULL){
            printf("Something went wrong the mode is NULL in getAllModeNames\n");
            continue;
        }
        ModeData* tempMetaData = tempMode->modeDetails;
        PRINTFLEVEL2("SOFTWRE: Got the metadata\n");
        if(tempMetaData == NULL){
            printf("Something went wrong in getAllModeNames with getting the metadata\n");
            continue;
        }
        char* modeName = tempMetaData->modeName;
        PRINTFLEVEL2("SOFTWARE; got mode name of %s\n",modeName);
        names[i] = modeName;
        PRINTFLEVEL2("SOFTWARE:Added name %s to the list at index %i\n",modes[i]->modeDetails->modeName,i);
    }
    return names;
}
//Used later on;
//[C, C shift 1, C shift 2, C hold, C hold Shift 1, C hold shift 2, D , ...]



void freeModesLambda(void* data){
    Mode* tempMode = (Mode*) data;
    tempMode->freeMode(tempMode);
    tempMode = 0;
}
/*
* Frees all of the mode structts and the array
*/
void freeModes(){
    destroyHashMap(ModeHashMap, freeModesLambda, NullHashFree);
    ModeHashMap = 0;
    free(keyBinds);
}

int getModeCount(){
    return ModeHashMap->quantity;
}



/**
 * Since this file handles the routing of functions, it will also handle the routing on the programable keys
*/
static int keyPressToBindValue(KeyPress* key){
    int value = key->shiftAmount;
    if(key->isHold){
        value = value + 3;
    }
    switch (key->keyPressed)
    {
    case 'C':
        break;
    case 'D':
        value = value+6;
        break;
    default:
        //it should not get here
        return -1;
        break;
    }
    return value;
}

/**
 * binds the programable keys
*/
void setProgramibleKeys(KeyPress* key,char* name){
    
    int value = keyPressToBindValue(key);
    if(value == -1){
        //should not get here
        return;
    }
    //value has not been translated

    keyBinds[value] = getModeByName(name);
}

/**
 * gets the mode via the programable keys
*/
Mode* getModeViaProgramableKey(KeyPress* key){
    int value = keyPressToBindValue(key);
    if(value == -1){
        //should not get here
        return NULL;
    }
    //value has not been translated

    return keyBinds[value];
}

Mode** getHotKeyList(){
    Mode** tempKeyBinds = malloc(sizeof(Mode*)*12);
    for(int i = 0; i<12;i++){
        tempKeyBinds[i] = keyBinds[i];
    }
    return tempKeyBinds;
}
void setProgramibleKeysByIndex(int index, char* name){
    keyBinds[index] = getModeByName(name);
}

/**
 * Givin the name of the load function, it will call and return that mode object
*/
Mode* dynamicalyLoadInModeByName(char* functionName){
    void *lib_handle;
    CreateModePointer modeCreateFunction;
    const char *error;

    // Open the shared library at runtime
    lib_handle = dlopen(libraryName, RTLD_LAZY | RTLD_GLOBAL);
    if (!lib_handle) {
        fprintf(stderr, "Error: %s\n", dlerror());
        return NULL;
    }

    // Get a pointer to the function we want to call
    modeCreateFunction = (CreateModePointer)dlsym(lib_handle, functionName);
    if ((error = dlerror()) != NULL)  {
        fprintf(stderr, "Error: %s\n", error);
        dlclose(lib_handle);
        return NULL;
    }

    // Call the function dynamically
    Mode* result = modeCreateFunction();

//    dlclose(lib_handle);

    return result;
}

char** getNamesOfLoadFunctions(){
    int fileCount = 0;
    //getting the number of modes to load in
    DIR* directory = opendir(path);
    if (directory == NULL) {
        perror("Unable to open directory");
        return NULL;
    }
    struct dirent *entry;
    while ((entry = readdir(directory)) != NULL) {
        if (entry->d_type == DT_REG) {
            fileCount++;
        }
    }
    closedir(directory);

    //get the .c names of each of the files
    char** loadNames = malloc(sizeof(char*)*(fileCount+1));
    if(loadNames == NULL) {
        perror("malloc failed");
        return NULL;
    }

    directory = opendir(path);
    if (directory == NULL) {
        perror("Unable to open directory");
        return NULL;
    }
    int i = 0;
    while((entry = readdir(directory)) != NULL) {
        if (entry->d_type == DT_REG && strstr(entry->d_name, ".c") != NULL) {
            loadNames[i] = replaceSubstring(entry->d_name,"Mode.c","Load");
            PRINTFLEVEL2("%s : %s\n", entry->d_name,loadNames[i]);
            i++;
        }
    }
    closedir(directory);
    loadNames[i] = malloc(sizeof(char)*100);
    if (loadNames[i] == NULL) {
        perror("malloc failed");

        return loadNames;
    }
    sprintf(loadNames[i],"ENDOFNAMES");
    return loadNames;
}
