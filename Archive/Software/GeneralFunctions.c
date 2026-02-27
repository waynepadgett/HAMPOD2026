#include "GeneralFunctions.h"

int convertCharToKeyValue(KeyPress* keyPressed){
    int returnValue = keyPressed->keyPressed - '0';
    return returnValue;
}

char** textFileToArray(char* filePath){
    //read through the file once to find out how many items are in it
    int currentSize = 0;
    FILE *fp;
    char temp[100];
    printf("Software: Creating a char** object for text file %s\n",filePath);
    fp = fopen(filePath , "r");
    if(fp == NULL) {
      perror("Error opening file");
      return (char**) -1;
   }
    while(fgets(temp, 100, fp)!=NULL){
        currentSize ++;
    }
    rewind(fp);
    int readSize = 100;
    char** lines = malloc(sizeof(char*) * (currentSize+1));
    for(int i = 0; i<currentSize;i++){
        lines[i] = malloc(sizeof(char)*readSize);
        fgets(lines[i],readSize,fp);
        if(lines[i][strlen(lines[i])-1] == '\n'){
            lines[i][strlen(lines[i])-1] = '\0';
        }
    }
    fclose(fp);
    lines[currentSize] = "END OF ARRAY";
    return lines;
}

void freeFileArray(char** list){
    int i = 0;
    while(strcmp(list[i],"END OF ARRAY") != 0){
        free(list[i]);
        i++;
    }
    free(list);
}

/**
 * Returns the substring of the given string, starting from the offset given to the length
 * @param original: the original string
 * @param start: the starting offset, 0 if starting from the start, must be less then the length of the string
 * @param length: how many characters of the string to the end of the string. if -1 then goes to the end of the string
 * if start + length is greater  then the length of the string then returns the string 
*/
char* customSubString(char* original, int start, int length){
    int len = 0;// the actual length to be used
    if(length == -1){
        len = strlen(original) - start;
    }else{
        len = length;
    }
    char* returnString = malloc(sizeof(char)*(strlen(original)+1));
    strncpy(returnString,original + start, len);
    returnString[len] = '\0';
    return returnString;
}


char* replaceSubstring(char* original, char* old, char* new){
     char* newStr = malloc(sizeof(char)* (strlen(original)+strlen(old)+strlen(new)+1));
    strcpy(newStr,original);
    
    char *ret = strstr(newStr, old);
    if(ret == NULL){
        return newStr;
    }
    memcpy(ret+strlen(new),ret+strlen(old),strlen(ret)-strlen(new));
    memcpy(ret,new,strlen(new));
    return newStr;
}


int keyPadNumeber = 0;
int decimalPlaces = 0;
bool decimalPointPlaced = false;
/**
 * This is ment to let the user input get a long number from the key pad, like 43.25
 * The number returned will be positive
 * Negitive number outputs mean user has not finished inputting their number yet
 * \# is the enter key
 * * is the decimal then reset key
*/
double keypadInput(KeyPress* keyInput){
    double tempkeyPadNumeber;
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
            if(decimalPointPlaced){
                decimalPlaces ++;
            }
            keyPadNumeber = keyPadNumeber*10 + convertCharToKeyValue(keyInput);
            break;
        case '#':
            tempkeyPadNumeber = keyPadNumeber + 0.0;
            for(int i = 0; i<decimalPlaces; i++){
                tempkeyPadNumeber = tempkeyPadNumeber / 10;
            }
            //set up dummy to be returned
            keyPadNumeber = 0;
            decimalPlaces = 0;
            decimalPointPlaced = false;
            return tempkeyPadNumeber;
        case '*':
            if(decimalPointPlaced){
                keyPadNumeber = 0;
                decimalPlaces = 0;
                decimalPointPlaced = false;
            }else{
                decimalPlaces = 0;
                decimalPointPlaced = true;
            }
            break;
        default:
            break;
    }
    tempkeyPadNumeber = keyPadNumeber + 0.0;
    for(int i = 0; i<decimalPlaces; i++){
        tempkeyPadNumeber = tempkeyPadNumeber / 10;
    }
    return tempkeyPadNumeber* -1.0 ;
}

bool ABState = true;
bool CDState = true;
bool getABState(){
    return ABState;
}
bool getCDState(){
    return CDState;
}
void setABstate(bool state){
    ABState = state;
}
void setCDstate(bool state){
    CDState = state;
}

