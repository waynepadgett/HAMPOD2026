pthread_t speakerThread;
bool running = true;
char* audioFolderPath = "../Firmware/pregen_audio/";
// creating the locks

pthread_mutex_t thread_lock;

HashMap* audioHashMap;
HashMap* stringDictinary;
int firmware_input_fd = -1;
int firmware_output_fd = -1;

//this starts up the communication of the firmware
void firmwareCommunicationStartup(){
    if(pthread_mutex_init(&thread_lock, NULL) != 0) {
        perror("thread_lock");
        exit(1);
    }
    setupAudioHashMap();
    setupDictinaryHashMap();

    printf("Software: Connecting to Firmware pipes\n");
    firmware_input_fd = open(INPUT_PIPE, O_WRONLY);
    if(firmware_input_fd == -1) {
        perror("Failed to open Firmware_i");
    }
    firmware_output_fd = open(OUTPUT_PIPE, O_RDONLY);
    if(firmware_output_fd == -1) {
        perror("Failed to open Firmware_o");
    }
    printf("Software: Connected to Firmware pipes\n");
}

int readNumPad() {
    if(firmware_input_fd == -1 || firmware_output_fd == -1) {
        return '-';
    }

    pthread_mutex_lock(&thread_lock);
    
    unsigned char cmd = 'r';
    Inst_packet* packet = create_inst_packet(KEYPAD, 1, &cmd, 0);
    write(firmware_input_fd, packet, 8);
    write(firmware_input_fd, packet->data, 1);
    destroy_inst_packet(&packet);

    // Read response
    Packet_type type;
    unsigned short size;
    unsigned short tag;
    char key;
    
    read(firmware_output_fd, &type, sizeof(Packet_type));
    read(firmware_output_fd, &size, sizeof(unsigned short));
    read(firmware_output_fd, &tag, sizeof(unsigned short));
    read(firmware_output_fd, &key, sizeof(char));
    
    pthread_mutex_unlock(&thread_lock);
    
    return (int)key;
}

// Wrapper for firmwarePlayAudio to match pthread_create signature
void* firmwarePlayAudioWrapper(void* text) {
    firmwarePlayAudio(text);
    return NULL;
}

/**
 * Creates the speeker output and puts it onto the qeueu asycronusly 
 * Return a string
 * //TODO make it check if the file exists first
 * //TODO set up the text based upon that for sending out
*/
char* sendSpeakerOutput(char* textIn){
    char* text = applyDictionary(textIn);
    if (!text) {
        printf("applyDictionary returned NULL!\n");
        return textIn;
    }
    printf("DEBUG: text after applyDictionary is '%s'\n", text);
    size_t testLen = strlen(text);
    printf("DEBUG: text length is %zu\n", testLen);

    if(SIMULATEOUTPUT){
        PRINTFLEVEL1("TESTING SPEAKER OUTPUT: %s\n", text);
        bool hasAudioFile = getHashMap(audioHashMap, text) != NULL;
         if(hasAudioFile){
            PRINTFLEVEL1("SOFTWARE: Audio file was found\n");
         }else if(shouldCreateAudioFile(text)){
            PRINTFLEVEL1("SOFTWARE:No audio file found but saving new file\n");
         }else{
            PRINTFLEVEL1("SOFTWARE:No audio file found and NOT creating a new file\n");
         }
        return text;
    }
    //TODO add the stuff for checking if it exits
    bool hasAudioFile = getHashMap(audioHashMap, text) != NULL;
    PRINTFLEVEL2("SOFTWARE: Gotted %i from the audioHashmap\n",hasAudioFile);
    char* audioPath = NULL;
    char* outputText = NULL;
    printf("DEBUG: I made it here after defining audioPath and ouputText\n");
    if(hasAudioFile) {
        audioPath = getHashMap(audioHashMap, text);
        size_t pathLen = (audioPath) ? strlen(audioPath) : 0;
        printf("DEBUG: This is pathlen:%d and this is audioPath: %s\n",pathLen,audioPath);
        outputText = malloc((pathLen + 2)*sizeof(char));
        printf("DEBUG: This is right after mallocin outputText\n");
    } else {
         outputText = malloc((strlen(text)+100+1)*sizeof(char));
    }
    PRINTFLEVEL2("SOFTWARE: Malloced a new array\n");
    if(hasAudioFile){
        printf("DEBUG: about to strcopy for type p message\n");
        strcpy(outputText,"p");
        printf("DEBUG: made it past strcpy(outputText, p)\n");
        strcat(outputText,audioPath);
        printf("DEBUG: Made it to end of this if\n");
    }else if(shouldCreateAudioFile(text)){
         PRINTFLEVEL2("SOFTWARE:Creating new audio hashmap entrie for this\n");
        strcpy(outputText,"s");
        strcat(outputText,text);
        //TODO add it to the hashmap
        char* nameAndPath = malloc(sizeof(char)*(strlen(text)+strlen(audioFolderPath)+1));
        char* nameOnly = malloc(sizeof(char)*(strlen(text)+10+1));
        strcpy(nameAndPath,audioFolderPath);
        strcpy(nameOnly,text);
        strcat(nameAndPath,nameOnly);
        //TODO insert into the hash with (path/name, name)
        PRINTFLEVEL2("SOFTWARE: adding the data %s with the key of %s\n",nameAndPath,nameOnly);
        insertHashMap(audioHashMap,nameAndPath,nameOnly);
    }else{
        strcpy(outputText,"d");
        strcat(outputText,text);
    }

    PRINTFLEVEL1("SOFTWARE: Sending text [%s] to be outputed by speakers\n",outputText);
    int result;
    PRINTFLEVEL2("SOFTWARE Locking up speakout output to send out %s\n", outputText);
    pthread_mutex_lock(&thread_lock);
    PRINTFLEVEL2("SOFTWARE Creating the thread\n");
    result = pthread_create(&speakerThread, NULL, firmwarePlayAudioWrapper, (void*) outputText);
    PRINTFLEVEL2("SOFTWARE sing if the result was good\n");
    if (result) {
        fprintf(stderr, "Error creating thread: %d\n", result);
        exit(0);
    }
    PRINTFLEVEL2("SOFTWARE unlockiing the speaker lock\n");
    pthread_mutex_unlock(&thread_lock);
    // return firmwareCommandQueue(speakerPacket);
    PRINTFLEVEL2("SOFTWARE Returning the speaker output value\n");
   return text;
}

/**
 * filterBypass: if true will bypass the save generted file
 * verbosityBypass: is true will play even if verbosity is turned off
 * linearCall: if true will not do threading and will lock up until the audio is played
*/
char* sendSpeakerOutputWithConditions(char* textIn, bool filterBypass, bool verbosityBypass, bool linearCall){
    char* text = applyDictionary(textIn);
    if(SIMULATEOUTPUT){
        PRINTFLEVEL1("TESTING SPEAKER OUTPUT: %s\n", text);
         bool hasAudioFile = getHashMap(audioHashMap, text) != NULL;
         if(hasAudioFile){
            PRINTFLEVEL1("SOFTWARE: Audio file was found\n");
         }else if(shouldCreateAudioFile(text)){
            PRINTFLEVEL1("SOFTWARE:No audio file found but saving new file\n");
         }else{
            PRINTFLEVEL1("SOFTWARE:No audio file found and NOT creating a new file\n");
         }
        return text;
    }
    //TODO add the stuff for checking if it exits
    bool hasAudioFile = getHashMap(audioHashMap, text) != NULL;
    PRINTFLEVEL2("SOFTWARE: Gotted %i from the audioHashmap\n",hasAudioFile);
    char* audioPath = NULL;
    char* outputText = NULL;
    if (hasAudioFile) {
        audioPath = getHashMap(audioHashMap, text);
        printf("DEBUG: audioPath: %s\n", audioPath);
        size_t pathLen = (audioPath) ? strlen(audioPath) : 0;
        size_t totalNeeded = pathLen + 2;
        outputText = malloc((totalNeeded+50)*sizeof(char));
    } else {
        outputText = malloc((strlen(text)+100+1)*sizeof(char));
    }
    PRINTFLEVEL2("SOFTWARE: Malloced a new array\n");
    if(hasAudioFile){
        strcpy(outputText,"p");
        strcat(outputText,audioPath);
    }else if(shouldCreateAudioFile(text) || filterBypass){
         PRINTFLEVEL2("SOFTWARE:Creating new audio hashmap entrie for this\n");
        strcpy(outputText,"s");
        strcat(outputText,text);
        //TODO add it to the hashmap
        char* nameAndPath = malloc(sizeof(char)*(strlen(text)+strlen(audioFolderPath)+1));
        char* nameOnly = malloc(sizeof(char)*(strlen(text)+10+1));
        strcpy(nameAndPath,audioFolderPath);
        strcpy(nameOnly,text);
        strcat(nameAndPath,nameOnly);
        //TODO insert into the hash with (path/name, name)
        PRINTFLEVEL2("SOFTWARE: adding the data %s with the key of %s\n",nameAndPath,nameOnly);
        insertHashMap(audioHashMap,nameAndPath,nameOnly);
    }else{
        strcpy(outputText,"d");
        strcat(outputText,text);
    }

    PRINTFLEVEL1("SOFTWARE: Sending text [%s] to be outputed by speakers\n",outputText);
    int result;
    PRINTFLEVEL2("SOFTWARE Locking up speakout output to send out %s\n", outputText);
    pthread_mutex_lock(&thread_lock);
    PRINTFLEVEL2("SOFTWARE Creating the thread\n");
    if(linearCall){
        firmwarePlayAudio((void*)outputText);
        result = 0;
    }else{
        result = pthread_create(&speakerThread, NULL, firmwarePlayAudioWrapper, (void*) outputText);
    }
    PRINTFLEVEL2("SOFTWARE sing if the result was good\n");
    if (result) {
        fprintf(stderr, "Error creating thread: %d\n", result);
        exit(0);
    }
    PRINTFLEVEL2("SOFTWARE unlockiing the speaker lock\n");
    pthread_mutex_unlock(&thread_lock);
    // return firmwareCommandQueue(speakerPacket);
    PRINTFLEVEL2("SOFTWARE Returning the speaker output value\n");
   return text;
}


void setupAudioHashMap(){
    char* softwarePath = audioFolderPath;
    PRINTFLEVEL2("SOFTWARE:Creating the hashmap\n");
    audioHashMap = createHashMap(audioHash,audioCompare);
    struct dirent *de; 
    DIR *dr = opendir(softwarePath);//TODO set this to the correct location
    if (dr == NULL)  // opendir returns NULL if couldn't open directory 
    { 
        printf("Could not open current directory" ); 
        return; 
    }
    while ((de = readdir(dr)) != NULL){
        printf("%s\n", de->d_name);
        //TODO see if this will grab also the .wav part and if it grabs the path.
        if(strcmp(de->d_name, ".") != 0 && strcmp(de->d_name, "..") != 0){   
            char* nameAndPath = malloc(sizeof(char)*(strlen(de->d_name)+strlen(softwarePath)+1));
            char* nameOnly = malloc(sizeof(char)*(strlen(de->d_name)+10+1));
            strcpy(nameAndPath, softwarePath);
            strcpy(nameOnly,de->d_name);
            nameOnly[strlen(de->d_name)-4] = '\0'; //add back in the null
            strcat(nameAndPath,nameOnly);
            //TODO insert into the hash with (path/name, name)
            PRINTFLEVEL2("SOFTWARE: adding the data %s with the key of %s\n",nameAndPath,nameOnly);
            insertHashMap(audioHashMap,nameAndPath,nameOnly);
        }
    }
    PRINTFLEVEL2("SOFTWARE:Finished adding stuff to Hashmap\n");
    closedir(dr);     
}
int audioHash(void* key){
    char* st = (char*) key;
    int hash = 0;
    PRINTFLEVEL2("Creating a hash for the string %s\n", st);
    for(int i = 0; i<strlen(st); i++){
        hash += st[i];
    }
    return hash;
}
bool audioCompare(void* key1, void* key2){
    if(strcmp((char*) key1, (char*) key2) == 0){
        return true;
    }else{
        return false;
    }
}
void audioFree(void* data){
    free(data);
}

/**
 * Returns true if the audio is appropreate for creating a new text output
 * Currently filtering out any text with numbers in it
*/
bool shouldCreateAudioFile(char* text){
    for(int i = 0; i<strlen(text); i++){
        switch (text[i]){
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
                return false;
                break;
            default:
                break;
        }
    }
    return true;
}



void setupDictinaryHashMap(){
    stringDictinary = createHashMap(StringHash,StringHashCompare);
    char** dictinary = textFileToArray("ConfigSettings/dictionary.txt");
    char* remain;
    char* start;
    PRINTFLEVEL1("Got the dictionay, starting to load up the dictionary\n");
    for(int i = 0; strcmp(dictinary[i], "END OF ARRAY") != 0;i++){
        size_t needed = strlen(dictinary[i]) + 1;
        start = malloc(needed);
        strcpy(start,dictinary[i]);
        remain = start;
        remain = strchr(remain, ' ');
        remain[0] = '\0';
        remain = remain+1;
        char* remainCopy = strdup(remain);
        insertHashMap(stringDictinary,(void*) remainCopy, (void*) start);
    }
    freeFileArray(dictinary);
    // free(remain); removing this temporarily
}

char** split(const char *str, char delimiter, int *numTokens) {
    // Count the number of tokens
    int count = 1; // At least one token (empty string or non-existent string)
    const char *ptr = str;
    while (*ptr != '\0') {
        if (*ptr == delimiter) {
            count++;
        }
        ptr++;
    }

    // Allocate memory for the array of strings
    char **tokens = (char **)malloc(count * sizeof(char *));
    if (tokens == NULL) {
        fprintf(stderr, "Memory allocation failed.\n");
        return NULL;
    }

    // Copy tokens into the array
    int tokenIndex = 0;
    int startIndex = 0;
    for (int i = 0; str[i] != '\0'; i++) {
        if (str[i] == delimiter) {
            int tokenLength = i - startIndex;
            tokens[tokenIndex] = (char *)malloc((tokenLength + 1) * sizeof(char));
            if (tokens[tokenIndex] == NULL) {
                fprintf(stderr, "Memory allocation failed.\n");
                // Free memory allocated so far
                for (int j = 0; j < tokenIndex; j++) {
                    free(tokens[j]);
                }
                free(tokens);
                return NULL;
            }
            strncpy(tokens[tokenIndex], str + startIndex, tokenLength);
            tokens[tokenIndex][tokenLength] = '\0'; // Null-terminate the string
            startIndex = i + 1;
            tokenIndex++;
        }
    }

    // Copy the last token
    int lastTokenLength = strlen(str) - startIndex;
    tokens[tokenIndex] = (char *)malloc((lastTokenLength + 1) * sizeof(char));
    if (tokens[tokenIndex] == NULL) {
        fprintf(stderr, "Memory allocation failed.\n");
        // Free memory allocated so far
        for (int j = 0; j <= tokenIndex; j++) {
            free(tokens[j]);
        }
        free(tokens);
        return NULL;
    }
    strncpy(tokens[tokenIndex], str + startIndex, lastTokenLength);
    tokens[tokenIndex][lastTokenLength] = '\0'; // Null-terminate the string

    *numTokens = count;
    return tokens;
}

void freeTokens(char **tokens, int numTokens) {
    if (tokens == NULL) return;
    for (int i = 0; i < numTokens; i++) {
        free(tokens[i]);
    }
    free(tokens);
}

int is_digit(char c) {
    return c >= '0' && c <= '9';
}

void insertSpaces(char *str) {
    int len = strlen(str);
    for(int i = 0; i<len;i++){
        if(is_digit(str[i])){
            for(int j = len+3;j>i+2;j--){
                str[j] = str[j-2];
            }
            str[i+1] = str[i];
            str[i] = ' ';
            str[i+2] =' ';
            i += 2;
            len += 2;
        }
    }
//    int i = 0;
  //  while (str[i] != '\0') {
    //    if (is_digit(str[i])) {
      //      int len = strlen(str);
        //    for (int j = len; j>=i; j--) {
          //          str[j+2] = str[j];
            // }
            //str[i] = ' ';
            //str[i+2] = ' ';
            //i += 3;
        //} else {
          //  i++;
        //}
   // }
}

char* applyDictionary(char* s){
    //apply the dictonary to this
    PRINTFLEVEL1("Applying dictionay changes to %s\n",s); 
    char* stringBuild = malloc(sizeof(char)*(strlen(s)*3 + 1));
    if (stringBuild == NULL) {
        fprintf(stderr, "Memory allocation failed.\n");
        return NULL; // Return NULL to indicate failure
    }
    stringBuild[0] = '\0';
    int count;
    char** splitStuff = split(s,' ',&count);
    PRINTFLEVEL2("All setup before creating the token\n");
    for(int i = 0;i<count;i++){
        PRINTFLEVEL2("testing if: %s: is in the hash\n",splitStuff[i]);
        if(containsHashMap(stringDictinary,(void*) splitStuff[i])){
            PRINTFLEVEL2("It was in it\n");
            strcat(stringBuild,(char*)getHashMap(stringDictinary,(void*) splitStuff[i]));
        }else{
            PRINTFLEVEL2("It was NOT in it\n");
            strcat(stringBuild,splitStuff[i]);
        }
        strcat(stringBuild," ");
    }
    freeTokens(splitStuff,count);
    PRINTFLEVEL1("Applying number spacing to to %s\n",stringBuild);
    //apply the numeric updates to this
    insertSpaces(stringBuild);
    PRINTFLEVEL1("finished creation and got %s\n",stringBuild);
    size_t finalLen = strlen(stringBuild);
    if (finalLen > 0) {
        stringBuild[strlen(stringBuild)-1] = '\0';
    }
    return stringBuild;
}


void freeFirmwareComunication(){
    running = false;
    printf("Software:destroying thread uqueue mutexes\n");
    pthread_mutex_destroy(&thread_lock);
    destroyHashMap(audioHashMap,audioFree,audioFree);
    destroyHashMap(stringDictinary,StringHashFree,StringHashFree);
}
